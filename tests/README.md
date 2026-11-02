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
| --- | --- |
| [`test_solidworks.cpp`](test_solidworks.cpp) | `esCompatible`, `setVersion`, `requiereGenerico`, invariantes del constructor, `obtenerRegBase` y el cambio de raiz de 2023 |
| [`test_gpu.cpp`](test_gpu.cpp) | Deteccion de marca por vendor/renderer, marca elegida a mano, seleccion de workarounds, armado del contenido del `.reg` |
| [`test_advancemode.cpp`](test_advancemode.cpp) | `enableTab`, rutas versionadas, `getOriginalValue` contra el registro, `opciones` y `armarOpciones` |
| [`test_registro.cpp`](test_registro.cpp) | Cabecera y formato del `.reg` que arman `armarArchivoReg` y `guardarArchivoReg` |
| [`test_access.h`](test_access.h) | Puentes `friend` para alcanzar las funciones puras privadas |
| [`registry_fixture.h`](registry_fixture.h) | Helpers RAII para los tests que leen del registro |
| [`doctest.h`](doctest.h) | Framework vendorizado (v2.4.11, MIT) |

El proyecto compila el core: `src\AdvanceMode.cpp`, `src\GPU.cpp`,
`src\Registro.cpp` y `src\SolidWorks.cpp`. Quedan afuera a proposito:

- `src\RealViewOn.cpp`, porque define `main()`.
- `src\Consola.cpp` y `src\Teclado.cpp`, porque piden input por teclado.
- `src\Playmidi.cpp`, porque arrastra `rtmidi.lib` y `midifile.lib`.
- `src\gui\`, la interfaz grafica.

`RealViewOn.Tests.vcxproj` **no** esta agregado a `RealViewOn.slnx`: el workflow
de release compila la solucion y no debe arrastrar los tests.

## Reglas al agregar tests

**1. El core no pide input, y tiene que seguir asi.** Las preguntas viven en
`src\Consola.cpp` (y las de la GUI en su ventana), que este proyecto no compila.
Si algo del core llegara a llamar a `entradaTeclado()` o `yesOrNo()`, el link
falla, porque `Teclado.cpp` tampoco esta en el proyecto. Es a proposito: esas
funciones usan `_getch()`, que bloquearia el runner en CI esperando una tecla.

El paso "Run tests" del workflow tiene `timeout-minutes: 5` como red de
contencion por si algun test igual se colgara.

**2. Los tests que tocan el registro usan su propia raiz.** Los de
`getOriginalValue` y `obtenerCurrent(path)` crean y borran
`HKEY_CURRENT_USER\SOFTWARE\RealViewOnTests` mediante `ClaveDeRegistroTemporal`
(RAII, en [`registry_fixture.h`](registry_fixture.h)). No tocan
`HKEY_CURRENT_USER\SOFTWARE\SolidWorks`, asi que correr la suite en una maquina
con SolidWorks instalado es seguro.

**3. Los tests que escriben archivos usan un directorio temporal.**
`guardarArchivoReg()` escribe siempre en el directorio actual, asi que
`DirectorioTemporal` (RAII, en `test_registro.cpp`) hace `chdir` a una carpeta
propia bajo `%TEMP%` y la borra al salir.

**4. Para alcanzar una funcion privada, ampliar `test_access.h`.** Cada clase
declara `friend struct <Clase>TestAccess;` y ese struct se implementa en
`test_access.h`, que solo compila dentro de este proyecto. Asi los tests llegan a
la logica pura sin ensanchar la API publica de las clases.

## Salida

El core no escribe en `cout`: los mensajes los muestran la consola y la GUI. La
salida del runner es solo la de doctest, y el exit code del binario es lo que
decide si la corrida pasa o falla.
