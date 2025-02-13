@echo off
chcp 65001 >nul
setlocal

set SOURCE_FILE="x64\Release\RealViewOn.exe"
set DEST_FILE="%~dp0RealViewOn.exe"
set UPX_EXEC="upx.exe"
set COMPRESSED_FILE="%~dp0RealViewOn.7z"

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
echo Archivo copiado a la raíz del directorio donde se ejecuta el script.

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

set SEVEN_ZIP_EXEC="C:\Program Files\7-Zip\7z.exe"

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