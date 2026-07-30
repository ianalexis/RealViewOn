# tools/

Utilidades del pipeline de release.

| Archivo | Que es |
| --- | --- |
| [`upx.exe`](upx.exe) | [UPX](https://github.com/upx/upx), comprime `RealViewOn.exe`. Build **x64**. |
| [`7zr.exe`](7zr.exe) | [7-Zip](https://github.com/ip7z/7zip) consola standalone, arma `RealViewOn.7z`. Build **x86**. |
| [`tools.lock.json`](tools.lock.json) | Versiones y SHA256 fijados de los dos binarios de arriba. |
| [`Update-VendoredTools.ps1`](Update-VendoredTools.ps1) | Compara los binarios contra su origen y los actualiza. |
| [`ReleaseUPXZIP.bat`](ReleaseUPXZIP.bat) | PostBuildEvent: copia, comprime con UPX y empaqueta con 7-Zip. |
| [`GetWorka.ps1`](GetWorka.ps1) | Script auxiliar para recolectar datos de workarounds. |

## Actualizacion automatica

[`.github/workflows/update-tools.yml`](../.github/workflows/update-tools.yml) corre
los lunes a las 06:00 UTC. Para cada herramienta de `tools.lock.json`:

1. Lista los releases del repo de origen y descarta drafts, prereleases y
   cualquier tag o nombre que parezca `alpha` / `beta` / `rc` / `preview`.
   El flag `prerelease` de GitHub no alcanza por si solo: ni `upx/upx` ni
   `ip7z/7zip` lo usan, asi que un "26.03 beta" llegaria como release normal.
2. Elige el estable mas nuevo **comparando numeros de version**, no fechas: un
   parche de una linea vieja publicado despues no debe ganarle a una minor nueva.
3. Si hay novedad, descarga el asset, extrae el ejecutable y **verifica antes de
   reemplazar nada**:
   - la arquitectura leida del encabezado PE coincide con la fijada en el lock;
   - una prueba funcional real con los mismos flags que usa el release.
4. Reemplaza el binario, actualiza `version` y `sha256` en el lock, y abre un PR.

Si cualquier verificacion falla, el binario del repo **queda intacto** y el job
falla en rojo.

### Las pruebas funcionales

Un binario que descarga bien pero no comprime no sirve, asi que no alcanza con
mirar la version:

- **UPX** — comprime una copia de `tools/7zr.exe` (un PE real y chico que ya esta
  en el repo) con `--ultra-brute`, comprueba que el tamano bajo, valida el
  resultado con `upx -t` y **ejecuta el binario comprimido** para confirmar que
  todavia corre. Es exactamente lo que el pipeline le hace a `RealViewOn.exe`.
- **7zr** — crea un `.7z` con `a -t7z -mx=9 -md=1m -ms=on` (los flags del paso
  "Prepare release assets"), lo valida con `7zr t`, lo extrae y compara el SHA256
  del contenido contra el original.

### Correrlo a mano

```powershell
# Solo informar si hay actualizaciones. No toca el working tree.
pwsh tools/Update-VendoredTools.ps1 -CheckOnly

# Actualizar una herramienta (descarga, verifica y reemplaza el binario + el lock).
pwsh tools/Update-VendoredTools.ps1 -Id upx

# Todas.
pwsh tools/Update-VendoredTools.ps1
```

Sin token la API de GitHub permite 60 requests por hora y por IP. Si se agota:

```powershell
$env:GITHUB_TOKEN = 'ghp_...'   # basta un token sin scopes, solo sube el limite
pwsh tools/Update-VendoredTools.ps1 -CheckOnly
```

### Agregar otra herramienta

Sumar una entrada a `tools.lock.json`; la matriz del workflow se arma leyendo ese
archivo, no hace falta tocar el YAML. Campos:

| Campo | Para que |
| --- | --- |
| `id` | Identificador corto. Es el valor de `-Id` y el nombre del job. |
| `path` | Ruta del binario, relativa a la raiz del repo. |
| `repo` | `owner/name` del repositorio de origen en GitHub. |
| `version` | Version fijada actualmente. La actualiza el bot. |
| `arch` | `x64`, `x86` o `ARM64`. Se valida contra el encabezado PE. |
| `asset` | Nombre del asset del release. `{version}` se reemplaza. |
| `archiveMember` | Ruta dentro del zip, o `null` si el asset ya es el `.exe`. |
| `sha256` | Hash del binario fijado. Lo actualiza el bot. |
| `usedBy` | Texto libre; aparece en el cuerpo del PR. |

Para que el bot tenga prueba funcional, agregar un `case` en
`Invoke-PruebaFuncional`. Sin eso la herramienta se actualiza igual, pero el PR
dice que no hubo prueba.

> El lock se reserializa completo al actualizarse. Si se edita a mano, mantener el
> formato tal como esta (2 espacios de indentacion, mismo orden de claves) para
> que el diff del PR muestre solo `version` y `sha256`.

## Coherencia entre el lock y los binarios

Antes de comparar contra el origen, el script verifica que el SHA256 del binario
en disco coincida con el del lock. Si alguien reemplazo un `.exe` sin actualizar
el lock, falla y lo dice, en lugar de razonar sobre datos que no son ciertos.

## Que NO hace el bot

- **No verifica procedencia criptografica.** Ni `upx/upx` ni `ip7z/7zip` publican
  un archivo de checksums en sus releases de GitHub. El bot comprueba que el
  binario venga del repositorio oficial, que tenga la arquitectura esperada y que
  funcione, y deja el SHA256 en el cuerpo del PR. Comparar ese hash contra el que
  publica el proveedor ([UPX](https://github.com/upx/upx/releases),
  [7-Zip](https://www.7-zip.org/download.html)) queda a cargo de quien revisa.
- **No mergea.** Solo abre el PR.
- **No insiste.** Si ya existe un PR para esa version, abierto o cerrado, no crea
  otro: si se cerro, fue una decision del mantenedor.
- **No dispara los otros workflows.** Los PRs creados con `GITHUB_TOKEN` no
  disparan workflows, por diseno de GitHub. Ademas `tools/**` no esta en los
  `paths` de `tests.yml` ni de `msbuild.yml`, asi que estos PRs no traen CI. Para
  que un cambio de UPX se pruebe compilando y comprimiendo de verdad, agregar
  `tools/**` a los `paths` del `pull_request` de `msbuild.yml` y crear el PR con
  un PAT en lugar de `GITHUB_TOKEN`.
