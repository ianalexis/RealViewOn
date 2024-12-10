@echo off
chcp 65001 >nul
setlocal

set SOURCE_FILE="x64\Release\RealViewOn.exe"
set DEST_FILE="%~dp0RealViewOn.exe"
set UPX_EXEC="upx.exe"

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

endlocal
pause