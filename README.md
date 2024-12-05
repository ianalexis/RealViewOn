# RealViewOn

**RealViewOn** tiene como objetivo activar el Real View en las versiones de SolidWorks posteriores a 2010. El sistema puede ejecutarse en la PC final o generar manualmente el archivo necesario para habilitar Real View en otra PC.

## Modos de Operación

- **Smart**: Detecta y genera todas las claves automáticamente.
- **Forced**: Permite generar los archivos solicitando al usuario la información faltante cuando no se pueden obtener todos los datos requeridos.

## Uso

1. El sistema detecta las versiones de SolidWorks instaladas.
2. Busca la GPU utilizada por esa versión de SolidWorks.
3. Genera un archivo `.reg` que puede revisar manualmente antes de su ejecución para habilitar RealView.
4. En caso de no funcionar, modifique los valores `dword` del archivo hasta lograr su activación.

## Compatibilidad

- **SolidWorks**: Versiones posteriores a 2010.
- **GPU**: Nvidia, AMD o Intel.

## Ejecución

1. Descargue el ultimo RealViewOn.exe
2. RUN.

## Disclaimer

Este software está diseñado para mejorar el rendimiento y habilitar funciones específicas en SolidWorks. Úselo bajo su propio criterio y responsabilidad. RealViewOn no está afiliado, respaldado ni aprobado por Dassault Systèmes. SolidWorks y RealView son marcas registradas de Dassault Systèmes.