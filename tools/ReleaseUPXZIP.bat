@echo off
chcp 65001 >nul
setlocal

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
set UPX_EXEC="%TOOLS_PATH%\upx.exe"
set COMPRESSED_FILE="%BASE_PATH%\RealViewOn.7z"
set SEVEN_ZIP_EXEC="%TOOLS_PATH%\7zr.exe"

if not exist %SOURCE_FILE% (
    echo Archivo %SOURCE_FILE% no encontrado.
    pause
    exit /b
)

if exist %DEST_FILE% (
    del %DEST_FILE%
    if %errorlevel% neq 0 (
        echo Error al eliminar el archivo existente.
        pause
        exit /b
    )
) else (
    echo El archivo %DEST_FILE% no existe, se saltea la eliminación.
)

if exist %COMPRESSED_FILE% (
    del %COMPRESSED_FILE%
    if %errorlevel% neq 0 (
        echo Error al eliminar el archivo ZIP existente.
        pause
        exit /b
    )
) else (
    echo El archivo %COMPRESSED_FILE% no existe, se saltea la eliminación.
)

copy %SOURCE_FILE% %DEST_FILE%
if %errorlevel% neq 0 (
    echo Error al copiar el archivo.
    pause
    exit /b
)
echo Archivo copiado a la raíz del proyecto.

if not exist %UPX_EXEC% (
    echo %UPX_EXEC% no encontrado.
    pause
    exit /b
)

%UPX_EXEC% --ultra-brute %DEST_FILE%
if %errorlevel% neq 0 (
    echo Error al comprimir el archivo.
    pause
    exit /b
)
echo Compresión completada.

if not exist %SEVEN_ZIP_EXEC% (
    echo %SEVEN_ZIP_EXEC% no encontrado.
    pause
    exit /b
)

%SEVEN_ZIP_EXEC% a %COMPRESSED_FILE% %DEST_FILE%
if %errorlevel% neq 0 (
    echo Error al comprimir el archivo con 7-Zip.
    pause
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