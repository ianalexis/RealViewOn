# ![logo](RealViewOn.png) ![header](Header.png) ![logo](RealViewOnOff.gif)ES

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

Si necesitaste cambiar los valores, por favor comparta:

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