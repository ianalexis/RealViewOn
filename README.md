# ![logo](assets/images/RealViewOn.png) ![header](assets/images/Header.png) ![logo](assets/images/RealViewOnOff.gif)

[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases)
[![GitHub stars](https://img.shields.io/github/stars/ianalexis/Real-View-On-Releases?style=flat-square&logo=github)](https://github.com/ianalexis/Real-View-On-Releases/stargazers)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=darkgreen&label=Download%20Stable&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases/releases/latest/download/RealViewOn.7z)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=orangered&label=Download%20Pre-Release&style=flat-square&include_prereleases)](https://github.com/ianalexis/Real-View-On-Releases/releases)
[![Download](https://img.shields.io/badge/Download-Last%20Build-darkred.svg?style=flat-square&logo=download)](/RealViewOn.7z?raw=true)
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white&style=flat-square)](https://isocpp.org/)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white&style=flat-square)](https://www.microsoft.com/windows/)

**RealViewOn** enables RealView in SolidWorks versions after 2010 and fixes errors in "Enhanced graphics performance".
This tool can run directly on the target PC (recomended) or manually generate the necessary file to enable RealView on another PC.

## Key Features 🌟

- **Simplifies RealView Activation 🛠️:** Creates a `.reg` file to enable RealView in a straightforward way without needing to browse regedit.
- **FULL USER CONTROL 🖐️:** This tool **DOES NOT apply any changes** directly. It simply generates a `.reg` file for you that you can read and decide to run or not.
- **Supports Old & New Methods 🕰️:** Works with both old and new methods as needed.
- **Fixes Visual & Graphics Issues 🖼️:** Resolves many common graphical errors and visual glitches users experience when enabling enhanced graphics performance.

## Operation Modes ⚙️

- **Smart 🤖:** Automatically detects and generates all keys.
- **Manual 📝:** Allows generating the files by requesting the missing information from the user when all required data cannot be obtained.

## Usage 📋

0. 🧑 MANUAL - [Download the latest version from Releases (exe or 7z) 📥](https://github.com/ianalexis/Real-View-On-Releases/releases).
1. 🤖 AUTO - The tool detects the installed versions of SolidWorks 🔍.
2. 📑 MANUAL - Select the SolidWorks version.
3. 🤖 AUTO - Search for the GPU used by that version of SolidWorks 🖥️.
   1. 🚧 In case no Renderer is found, the user will be prompted to select:
      1. 🔍 Renderers found in the registry.
      2. 🖥️ Display Adapters found in the Device Manager.
      3. ✍️ Manually enter the GPU model.
         1. 📝 Go to the Device Manager and search for the GPU under Display Adapters.
         2. 💻 Open the GPU properties and copy the name of the device under the Details tab.
         3. ⌨️ Enter the device name into the program and press Enter.
4. 🤖 AUTO - A custom `.reg` file will be generated📝.
5. ✨ MANUAL - Execution of the `.reg` file.
   1. 🕵️ OPTIONAL - Review (with any text editor) the `.reg` file.
   2. 🚀 MANUAL - Execute the `.reg` file by double clicking on it.

## Solutions if something does not work 👩‍🔧🖥️
In case you find errors modify the `dword` values of the file with the examples for your brand to commented in the file.

### RealView does not work
Modify the `dword` values of GL2Shaders.

### Sketchs and visual errors
Modify the `dword` values of the brand.

## Contribute 🤝

We welcome any feedback regarding the functionality of the tool, whether it works or not.
If needed, we are happy to assist you , not only to ensure proper usage but also to identify potential areas for improvement.

### Workarounds
A great way to help us is to share with us the values of your SolidWorks installation.
Just download and run the [GetWorka.ps1](https://github.com/ianalexis/Real-View-On-Releases/blob/main/GetWorka.ps1) file and share the generated file with us.
It is simple, fast, safe, requires no technical knowledge and will help us to improve the tool.

### If you needed to change the values, please share:

- **Values:** `dword` values and changes in the `.reg` file.
- **Renderer:** GPU
- **SolidWorks version:** SW versions you want to enable RealView on.

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

## Disclaimer ⚠️

This software facilitates enabling features in SolidWorks. Use it at your own discretion and responsibility.
SolidWorks & RealView are registered trademarks of Dassault Systèmes.

___

# ![logo](assets/images/RealViewOn.png) ![header](assets/images/Header.png) ![logo](assets/images/RealViewOnOff.gif)ES

[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases)
[![GitHub stars](https://img.shields.io/github/stars/ianalexis/Real-View-On-Releases?style=flat-square&logo=github)](https://github.com/ianalexis/Real-View-On-Releases/stargazers)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=darkgreen&label=Descarga%20Estable&style=flat-square)](https://github.com/ianalexis/Real-View-On-Releases/releases/latest/download/RealViewOn.7z)
[![Version](https://img.shields.io/github/v/release/ianalexis/Real-View-On-Releases?color=orangered&label=Descarga%20Pre-Release&style=flat-square&include_prereleases)](https://github.com/ianalexis/Real-View-On-Releases/releases)
[![Download](https://img.shields.io/badge/Descarga-Ultima%20Compilación-darkred.svg?style=flat-square&logo=download)](/RealViewOn.7z?raw=true)
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white&style=flat-square)](https://isocpp.org/)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white&style=flat-square)](https://www.microsoft.com/windows/)

**RealViewOn** habilita RealView en versiones de SolidWorks posteriores a 2010 y soluciona errores en el modo de "Rendimiento gráfico mejorado".
La herramienta puede ejecutarse directamente en el PC objetivo (recomendado) o generar manualmente el archivo necesario para habilitar RealView en otro PC.

## Características Principales 🌟

- **Simplifica la Activación de RealView 🛠️:** Crea un archivo `.reg` para habilitar RealView de una manera sencilla sin necesidad de navegar por regedit.
- **CONTROL TOTAL DEL USUARIO 🖐️:** Esta herramienta **NO aplica ningún cambio directamente**. Simplemente genera un archivo `.reg` que puedes leer y decidir si ejecutarlo o no.
- **Soporta Métodos Antiguos y Nuevos 🕰️:** Funciona con métodos antiguos y nuevos según sea necesario.
- **Soluciona Problemas Visuales y Gráficos 🖼️:** Resuelve muchos errores gráficos comunes y fallos visuales que los usuarios experimentan al habilitar el rendimiento gráfico mejorado.

## Modos de Operación ⚙️

- **Inteligente 🤖:** Detecta y genera automáticamente todas las claves.
- **Manual 📝:** Permite generar el `.reg` solicitando al usuario la información que no se puede obtener automáticamente.

## Uso 📋

0. 🧑 MANUAL - [Descargue la última versión desde Releases (exe o 7z) 📥](https://github.com/ianalexis/Real-View-On-Releases/releases).
1. 🤖 AUTO - El sistema detecta las versiones instaladas de SolidWorks 🔍.
2. 📑 MANUAL - Seleccione la versión de SolidWorks.
3. 🤖 AUTO - Busca la GPU utilizada por esa versión de SolidWorks 🖥️.
   1. 🚧 En caso de no encontrar GPU, se solicitará al usuario que seleccione:
      1. 📝 Renders encontrados en el registro.
      2. 🖥️ Adaptadores de pantalla encontrados en el Administrador de dispositivos.
      3. ✍️ Ingresar manualmente el modelo de la GPU.
         1. 💻 Ingrese en el Administrador de dispositivos y busque la GPU dentro de Adaptadores de pantalla.
         2. 📋 Abra las propiedades de la GPU y copie el nombre del dispositivo dentro de la pestaña Detalles.
         3. ✍️ Ingrese el nombre del dispositivo en el programa y presione Enter.
4. 🤖 AUTO - Se generará un archivo `.reg` personalizado 📝.
5. ✨ MANUAL - Ejecución del archivo `.reg`.
   1. 🕵️ OPCIONAL - Revise (con cualquier editor de texto) el `.reg`
   2. 🚀 MANUAL - Ejecute el archivo `.reg` haciendo doble click sobre el.

## Soluciones si algo no funciona 👩‍🔧🖥️
En caso de encontrar errores modifique los valores `dword` del archivo con los ejemplos para su marca comentados en el archivo.

### RealView no funciona
Modifique los valores `dword` de GL2Shaders.

### Sketchs y errores visuales
Modifique los valores `dword` de la marca.

## Contribuir 🤝

Agradecemos cualquier comentario sobre el funcionamiento de la herramienta, ya sea que funcione o no.
Si es necesario, podemos ofrecer asistencia para garantizar un uso adecuado y también para identificar posibles áreas de mejora.

### Workarounds
Una gran manera de ayudarnos es compartirnos los valores de tu instalacion de SolidWorks.
Solamente deberas ejecutar el archivo [GetWorka.ps1](https://github.com/ianalexis/Real-View-On-Releases/blob/main/GetWorka.ps1) y compartirnos el archivo generado.
Es simple, rapido , seguro , no requiere conocimientos tecnicos y nos ayudara a mejorar la herramienta.

### Si necesitaste cambiar los valores, por favor comparta:

- **Valores:** valores `dword` y cambios en el archivo `.reg`.
- **Renderer:** GPU
- **Versión de SolidWorks:** versiones de SW en las que deseas habilitar RealView.

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

## Descargo de Responsabilidad ⚠️

Este software facilita la habilitación de funciones en SolidWorks.
Úselo a su propia discreción y responsabilidad.
SolidWorks & RealView son marcas registradas de Dassault Systèmes.
