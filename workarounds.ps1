# Importar el módulo de registro
Import-Module -Name 'Microsoft.PowerShell.Management'

# Definir la ruta base del registro
$basePath = "HKCU:\SOFTWARE\SolidWorks"

# Obtener todas las claves del registro bajo la ruta base
$keys = Get-ChildItem -Path $basePath -Recurse

# Crear una lista para almacenar los resultados
$result = @()

# Iterar sobre cada clave y buscar valores llamados 'Workarounds'
foreach ($key in $keys) {
    $workaround = Get-ItemProperty -Path $key.PSPath -Name 'Workarounds' -ErrorAction SilentlyContinue
    if ($workaround) {
        $result += [PSCustomObject]@{
            Path  = $key.PSPath
            Value = $workaround.Workarounds
        }
    }
}

# Obtener la ruta del script actual
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Exportar los resultados a un archivo CSV en la misma carpeta del script
$result | Export-Csv -Path "$scriptPath\workarounds.csv" -NoTypeInformation -Encoding UTF8
