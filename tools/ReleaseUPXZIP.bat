@echo off
chcp 65001 >nul
setlocal

REM Copia, comprime con UPX y empaqueta en RealViewOn.7z la consola y, si esta
REM compilada, la GUI (RealViewOnGUI.exe). En la solucion la consola depende de la
REM GUI para que este paso la encuentre ya compilada.
REM
REM Este script corre como PostBuildEvent, tambien en CI. Ahi no hay nadie para
REM apretar una tecla: un "pause" dejaria el job colgado hasta el timeout (6 h en
REM GitHub Actions) en lugar de fallar. GitHub Actions define CI=true.
set PAUSA=pause
if defined CI set PAUSA=echo [CI] Error detectado, se continua sin pausa.

REM Detectar si estamos ejecutando desde la carpeta tools o desde la raíz del proyecto
if exist ".\upx.exe" (
    REM Ejecutándose desde la carpeta tools (manual)
    set BASE_PATH=..
    set TOOLS_PATH=.
) else (
    REM Ejecutándose desde la raíz del proyecto (VS)
    set BASE_PATH=.
    set TOOLS_PATH=.\tools
)

REM Configurar rutas usando las variables base
set SOURCE_FILE="%BASE_PATH%\x64\Release\RealViewOn.exe"
set DEST_FILE="%BASE_PATH%\RealViewOn.exe"
set SOURCE_GUI="%BASE_PATH%\x64\Release\RealViewOnGUI.exe"
set DEST_GUI="%BASE_PATH%\RealViewOnGUI.exe"
set UPX_EXEC="%TOOLS_PATH%\upx.exe"
set COMPRESSED_FILE="%BASE_PATH%\RealViewOn.7z"
set SEVEN_ZIP_EXEC="%TOOLS_PATH%\7zr.exe"

if not exist %SOURCE_FILE% (
    echo Archivo %SOURCE_FILE% no encontrado.
    %PAUSA%
    exit /b
)

if exist %DEST_FILE% (
    del %DEST_FILE%
    if errorlevel 1 (
        echo Error al eliminar el archivo existente.
        %PAUSA%
        exit /b
    )
) else (
    echo El archivo %DEST_FILE% no existe, se saltea la eliminación.
)

if exist %DEST_GUI% (
    del %DEST_GUI%
    if errorlevel 1 (
        echo Error al eliminar el archivo existente de la GUI.
        %PAUSA%
        exit /b
    )
)

if exist %COMPRESSED_FILE% (
    del %COMPRESSED_FILE%
    if errorlevel 1 (
        echo Error al eliminar el archivo ZIP existente.
        %PAUSA%
        exit /b
    )
) else (
    echo El archivo %COMPRESSED_FILE% no existe, se saltea la eliminación.
)

copy %SOURCE_FILE% %DEST_FILE%
if errorlevel 1 (
    echo Error al copiar el archivo.
    %PAUSA%
    exit /b
)
echo Archivo copiado a la raíz del proyecto.

if not exist %UPX_EXEC% (
    echo %UPX_EXEC% no encontrado.
    %PAUSA%
    exit /b
)

%UPX_EXEC% --ultra-brute %DEST_FILE%
if errorlevel 1 (
    echo Error al comprimir el archivo.
    %PAUSA%
    exit /b
)
echo Compresión completada.

set ARCHIVOS=%DEST_FILE%
if exist %SOURCE_GUI% (
    copy %SOURCE_GUI% %DEST_GUI%
    if errorlevel 1 (
        echo Error al copiar la GUI.
        %PAUSA%
        exit /b
    )
    %UPX_EXEC% --ultra-brute %DEST_GUI%
    if errorlevel 1 (
        echo Error al comprimir la GUI.
        %PAUSA%
        exit /b
    )
    set ARCHIVOS=%DEST_FILE% %DEST_GUI%
    echo GUI copiada y comprimida.
) else (
    echo %SOURCE_GUI% no encontrado, se empaqueta solo la consola.
)

if not exist %SEVEN_ZIP_EXEC% (
    echo %SEVEN_ZIP_EXEC% no encontrado.
    %PAUSA%
    exit /b
)

%SEVEN_ZIP_EXEC% a %COMPRESSED_FILE% %ARCHIVOS%
if errorlevel 1 (
    echo Error al comprimir el archivo con 7-Zip.
    %PAUSA%
    exit /b
)
echo Compresión con 7-Zip completada.

echo.
echo ===========================================================
echo.
echo FINALIZADO CORRECTAMENTE. CERRANDO
echo.
echo ===========================================================
echo.
exit