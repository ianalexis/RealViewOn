# Tests unitarios

Suite de tests de [doctest](https://github.com/doctest/doctest) para la logica
pura de RealViewOn. Corre automaticamente en cada PR a `main` mediante
[`.github/workflows/tests.yml`](../.github/workflows/tests.yml).

## Correr los tests

```powershell
msbuild tests\RealViewOn.Tests.vcxproj /p:Configuration=Release /p:Platform=x64
.\tests\x64\Release\RealViewOn.Tests.exe
```

Opciones utiles del runner:

```powershell
# Listar los casos disponibles
.\tests\x64\Release\RealViewOn.Tests.exe --list-test-cases

# Correr un solo suite o un solo caso
.\tests\x64\Release\RealViewOn.Tests.exe --test-suite="AdvanceMode::enableTab"
.\tests\x64\Release\RealViewOn.Tests.exe --test-case="*idempotente*"

# Reporte JUnit (lo que sube el workflow como artifact)
.\tests\x64\Release\RealViewOn.Tests.exe --reporters=junit --out=test-results.xml
```

Tambien se puede abrir `tests\RealViewOn.Tests.vcxproj` directamente en Visual
Studio; la configuracion `Debug|x64` compila sin optimizaciones para poder
depurar un test que falla.

## Como esta armado

| Archivo | Cubre |
|---|---|
| [`test_solidworks.cpp`](test_solidworks.cpp) | `esCompatible`, invariantes del constructor, `obtenerRegBase` y el cambio de raiz de 2023 |
| [`test_gpu.cpp`](test_gpu.cpp) | Deteccion de marca por vendor/renderer, seleccion de workarounds, armado del contenido del `.reg` |
| [`test_advancemode.cpp`](test_advancemode.cpp) | `enableTab`, rutas versionadas, `getOriginalValue` contra el registro |
| [`test_registro.cpp`](test_registro.cpp) | Cabecera y formato del `.reg` que genera `guardarArchivoReg` |
| [`test_access.h`](test_access.h) | Puentes `friend` para alcanzar las funciones puras privadas |
| [`doctest.h`](doctest.h) | Framework vendorizado (v2.4.11, MIT) |

El proyecto compila `src\AdvanceMode.cpp`, `src\GPU.cpp`, `src\Registro.cpp`,
`src\SolidWorks.cpp` y `src\Teclado.cpp`. Quedan afuera a proposito:

- `src\RealViewOn.cpp`, porque define `main()`.
- `src\Playmidi.cpp`, porque arrastra `rtmidi.lib` y `midifile.lib`.

`RealViewOn.Tests.vcxproj` **no** esta agregado a `RealViewOn.slnx`: el workflow
de release compila la solucion y no debe arrastrar los tests.

## Reglas al agregar tests

**1. Nunca llamar a codigo que pida input por teclado.** `entradaTeclado()` y
`yesOrNo()` usan `_getch()`, que bloquea esperando una tecla y colgaria el runner
en CI. En particular:

- `SolidWorks::setVersion()` pregunta por modo generico cuando la version no esta
  instalada, lo cual en CI es siempre. Los tests fijan `swVersion` con
  `SolidWorksTestAccess::setSwVersion()`.
- El constructor de `GPU` llama a `selecectBrandManual()` si **no** logra deducir
  la marca. Todo `GPU` que se construya en un test tiene que usar un vendor o
  renderer que la tabla reconozca. Para probar cadenas que no matchean usar
  `GPUTestAccess::buscarEnRenderMap()`, que nunca pide input.
- `AdvanceMode::askAdvanceOptions()` pregunta por cada opcion; se testean sus
  piezas internas, no ella.

El paso "Run tests" del workflow tiene `timeout-minutes: 5` como red de
contencion, pero un test que bloquea igual rompe la corrida.

**2. Los tests que tocan el registro usan su propia raiz.** Los de
`getOriginalValue` crean y borran
`HKEY_CURRENT_USER\SOFTWARE\RealViewOnTests` mediante `ClaveDeRegistroTemporal`
(RAII). No tocan `HKEY_CURRENT_USER\SOFTWARE\SolidWorks`, asi que correr la suite
en una maquina con SolidWorks instalado es seguro.

**3. Los tests que escriben archivos usan un directorio temporal.**
`guardarArchivoReg()` escribe siempre en el directorio actual, asi que
`DirectorioTemporal` (RAII, en `test_registro.cpp`) hace `chdir` a una carpeta
propia bajo `%TEMP%` y la borra al salir.

**4. Para alcanzar una funcion privada, ampliar `test_access.h`.** Cada clase
declara `friend struct <Clase>TestAccess;` y ese struct se implementa en
`test_access.h`, que solo compila dentro de este proyecto. Asi los tests llegan a
la logica pura sin ensanchar la API publica de las clases.

## Ruido en la salida

Varias funciones bajo prueba escriben en `cout` (`GPU Brand detected: ...`,
`File RealViewOn2025.reg created successfully at: ...`). Ese texto aparece
intercalado en la salida del runner. Es esperado y no afecta el resultado: el
exit code del binario es lo que decide si la corrida pasa o falla.
