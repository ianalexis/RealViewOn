# ![logo](RealViewOn.png) ![header](Header.png) ![logo](RealViewOnOff.gif)
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white&style=flat-square)](https://isocpp.org/)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white&style=flat-square)](https://www.microsoft.com/windows/)
[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases)
[![GitHub stars](https://img.shields.io/github/stars/ianalexis/Real-View-On-Releases?style=flat-square&logo=github)](https://github.com/ianalexis/Real-View-On-Releases/stargazers)
[![Download](https://img.shields.io/badge/Download-Stable%20Release-darkgreen.svg?style=flat-square&logo=download)](https://github.com/ianalexis/Real-View-On-Releases/releases/latest/download/RealViewOn.7z)
[![Download](https://img.shields.io/badge/Download-Last%20Build-orangered.svg?style=flat-square&logo=download)](/RealViewOn.7z?raw=true)

**RealViewOn** enables RealView in SolidWorks versions after 2010. The system can run directly on the target PC or manually generate the necessary file to enable RealView on another PC.

## Key Features 🌟

- **Simplifies RealView Activation 🛠️:** Creates a `.reg` file to enable RealView in a straightforward way without needing to browse regedit.
- **FULL USER CONTROL 🖐️:** This tool **DOES NOT apply any changes** directly. It simply generates a `.reg` file for you that you can read and decide to run or not.
- **Supports Old & New Methods 🕰️:** Works with both old and new methods as needed.
- **Fixes Visual & Graphics Issues 🖼️:** Resolves many common graphical errors and visual glitches users experience when enabling enhanced graphics performance.

## Operation Modes ⚙️

- **Smart 🤖:** Automatically detects and generates all keys.
- **Manual 📝:** Allows generating the files by requesting the missing information from the user when all required data cannot be obtained.

## Usage 📋

1. MANUAL - [Download the latest version from Releases (exe or 7z) 📥](https://github.com/ianalexis/Real-View-On-Releases/releases).
2. AUTO - The system detects the installed versions of SolidWorks 🔍.
3. AUTO - It searches for the GPU used by that version of SolidWorks 🖥️.
4. AUTO - Generates a `.reg` file that can be manually reviewed before execution to enable RealView 📝.
5. MANUAL - Review and execute the `.reg` file.
6. MANUAL - Fixes if something doesn't work:If it does not work, modify the `dword` values of the file until activation is achieved 🔧.
   1. RealView not working: Modify the GL2Shaders `dword` values. Examples for your brand bellow it.
   2. Seketchs and visual errors: Modify the Brand `dword` values. Examples for your brand bellow it.

## Compatibility 🖥️

- **SolidWorks:** Versions after 2010.
- **GPU Brand:**
  - 🟢Nvidia ⭐⭐⭐
  - 🔴AMD ⭐⭐
  - 🔵Intel ⭐⭐

- *Reliability*
  - ⭐⭐⭐⭐⭐: [GPU Microarchitectures tested > 3] && [SW versions tested per generation > 2]
  - ⭐⭐⭐⭐: [GPU Microarchitectures tested > 2] && [SW versions tested per generation > 2]
  - ⭐⭐⭐: [GPUs tested > 1] && [SW versions tested > 1]
  - ⭐⭐: [Actual GPU tested]
  - ⭐: [Theoretical testing]

## Contribute 🤝

We welcome any feedback regarding the functionality of the tool, whether it works or not.
If needed, we are happy to assist you , not only to ensure proper usage but also to identify potential areas for improvement.

If you needed to change the values, please share:

- **Values:** `dword` values and changes in the `.reg` file.
- **Renderer:** GPU
- **SolidWorks version:** SW versions you want to enable RealView on.

## Disclaimer ⚠️

This software facilitates enabling features in SolidWorks. Use it at your own discretion and responsibility.
SolidWorks & RealView are registered trademarks of Dassault Systèmes.

___

# ![logo](RealViewOn.png) ![header](Header.png) ![logo](RealViewOnOff.gif)ES
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white&style=flat-square)](https://isocpp.org/)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white&style=flat-square)](https://www.microsoft.com/windows/)
[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases)
[![GitHub stars](https://img.shields.io/github/stars/ianalexis/Real-View-On-Releases?style=flat-square&logo=github)](https://github.com/ianalexis/Real-View-On-Releases/stargazers)
[![Download](https://img.shields.io/badge/Descarga-Ultima%20Estable-darkgreen.svg?style=flat-square&logo=download)](https://github.com/ianalexis/Real-View-On-Releases/releases/latest/download/RealViewOn.7z)
[![Download](https://img.shields.io/badge/Descarga-Ultima%20Compilación-orangered.svg?style=flat-square&logo=download)](/RealViewOn.7z?raw=true)

**RealViewOn** habilita RealView en versiones de SolidWorks posteriores a 2010. El sistema puede ejecutarse directamente en el PC objetivo o generar manualmente el archivo necesario para habilitar RealView en otro PC.

## Características Principales 🌟

- **Simplifica la Activación de RealView 🛠️:** Crea un archivo `.reg` para habilitar RealView de una manera sencilla sin necesidad de navegar por regedit.
- **CONTROL TOTAL DEL USUARIO 🖐️:** Esta herramienta **NO aplica ningún cambio directamente**. Simplemente genera un archivo `.reg` que puedes leer y decidir si ejecutarlo o no.
- **Soporta Métodos Antiguos y Nuevos 🕰️:** Funciona con métodos antiguos y nuevos según sea necesario.
- **Soluciona Problemas Visuales y Gráficos 🖼️:** Resuelve muchos errores gráficos comunes y fallos visuales que los usuarios experimentan al habilitar el rendimiento gráfico mejorado.

## Modos de Operación ⚙️

- **Inteligente 🤖:** Detecta y genera automáticamente todas las claves.
- **Manual 📝:** Permite generar el `.reg` solicitando al usuario la información que no se puede obtener automáticamente.

## Uso 📋

1. MANUAL - [Descarga la última versión desde Releases (exe o 7z) 📥](https://github.com/ianalexis/Real-View-On-Releases/releases).
2. AUTO - El sistema detecta las versiones instaladas de SolidWorks 🔍.
3. AUTO - Busca la GPU utilizada por esa versión de SolidWorks 🖥️.
4. AUTO - Genera un archivo `.reg` que puede ser revisado manualmente antes de la ejecución para habilitar RealView 📝.
5. MANUAL - Revisión y ejecución del archivo `.reg`.
6. MANUAL - Soluciones si algo no funciona: Si no funciona, modifique los valores `dword` del archivo hasta lograr la activación 🔧.
   1. RealView no funciona: Modifique los valores `dword` de GL2Shaders. Ejemplos para su marca a comentados en el archivo.
   2. Sketchs y errores visuales: Modifique los valores `dword` de la marca. mplos para su marca a comentados en el archivo.

## Compatibilidad 🖥️

- **SolidWorks:** Versiones posteriores a 2010.
- **Marca de GPU:**
  - 🟢Nvidia ⭐⭐⭐
  - 🔴AMD ⭐⭐
  - 🔵Intel ⭐⭐
 
- *Confiabilidad*
  - ⭐⭐⭐⭐⭐: [Microarquitecturas de GPU probadas > 3] && [Versiones de SW probadas por generación > 2]
  - ⭐⭐⭐⭐: [Microarquitecturas de GPU probadas > 2] && [Versiones de SW probadas por generación > 2]
  - ⭐⭐⭐: [GPUs probadas > 1] && [Versiones de SW probadas  > 1]
  - ⭐⭐: [GPU real probada]
  - ⭐: [Prueba teórica]

## Contribuir 🤝

Agradecemos cualquier comentario sobre el funcionamiento de la herramienta, ya sea que funcione o no.
Si es necesario, podemos ofrecer asistencia para garantizar un uso adecuado y también para identificar posibles áreas de mejora.

Si necesitaste cambiar los valores, por favor comparta:

- **Valores:** valores `dword` y cambios en el archivo `.reg`.
- **Renderer:** GPU
- **Versión de SolidWorks:** versiones de SW en las que deseas habilitar RealView.

## Descargo de Responsabilidad ⚠️

Este software facilita la habilitación de funciones en SolidWorks.
Úselo a su propia discreción y responsabilidad.
SolidWorks & RealView son marcas registradas de Dassault Systèmes.
