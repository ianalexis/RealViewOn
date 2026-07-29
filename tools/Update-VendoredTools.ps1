#Requires -Version 7.0
<#
.SYNOPSIS
    Compara los binarios vendorizados en tools/ contra el ultimo release estable de
    su repositorio de origen y, opcionalmente, los actualiza.

.DESCRIPTION
    Las versiones fijadas viven en tools/tools.lock.json. Para cada herramienta el
    script:
      1. Lista los releases del repo de origen y descarta drafts, prereleases y
         cualquier tag/nombre que parezca alpha/beta/rc.
      2. Elige el estable mas nuevo comparando numeros de version (no fechas: un
         parche de una linea vieja publicado despues no debe ganar).
      3. Si hay novedad y no se paso -CheckOnly, descarga el asset, extrae el
         ejecutable, verifica arquitectura, corre una prueba funcional real y
         recien entonces reemplaza el binario y actualiza el lock.

    NO hace commit ni abre PRs: de eso se encarga
    .github/workflows/update-tools.yml. Asi el script se puede correr a mano.

.PARAMETER Id
    Id de la herramienta en el lock ('upx', '7zr') o 'all' para todas.

.PARAMETER CheckOnly
    Solo compara versiones e informa. No descarga ni modifica nada.

.PARAMETER SkipFunctionalTest
    Omite la prueba funcional. Solo para depurar; el workflow nunca la usa.

.PARAMETER GitHubToken
    Token opcional para la API de GitHub. Sin token el limite es 60 req/hora por
    IP, que en los runners compartidos se agota.

.PARAMETER PrBodyPath
    Si se indica y hubo actualizacion, escribe ahi el cuerpo markdown del PR. El
    workflow lo pasa a `gh pr create --body-file`. Se genera aca y no en el YAML
    porque un here-string de PowerShell no puede vivir dentro de un bloque
    escalar de YAML (el `"@` de cierre tiene que ir en la columna 0).

.EXAMPLE
    pwsh tools/Update-VendoredTools.ps1 -CheckOnly
    Informa si hay actualizaciones sin tocar el working tree.

.EXAMPLE
    pwsh tools/Update-VendoredTools.ps1 -Id upx
    Actualiza tools/upx.exe y tools/tools.lock.json si hay un estable mas nuevo.
#>
[CmdletBinding()]
param(
    [string] $Id = 'all',
    [string] $RepoRoot,
    [switch] $CheckOnly,
    [switch] $SkipFunctionalTest,
    [string] $GitHubToken = $env:GITHUB_TOKEN,
    [string] $PrBodyPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

# Tags/nombres que no son estables. El flag `prerelease` de GitHub no alcanza:
# ninguno de los dos upstreams lo usa, asi que un "26.03 beta" llegaria como
# release normal.
$PatronInestable = '(?i)(alpha|beta|preview|snapshot|nightly|(^|[^a-z])rc([^a-z]|\d|$))'

function Resolve-RepoRoot {
    param([string] $Provisto)
    if ($Provisto) { return (Resolve-Path $Provisto).Path }
    return (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
}

function Get-LockPath { param([string] $Raiz) Join-Path $Raiz 'tools/tools.lock.json' }

function Read-Lock {
    param([string] $Raiz)
    $ruta = Get-LockPath $Raiz
    if (-not (Test-Path $ruta)) { throw "No se encontro el lock: $ruta" }
    return Get-Content $ruta -Raw -Encoding utf8 | ConvertFrom-Json
}

function Write-Lock {
    param([string] $Raiz, $Lock)
    # Se reserializa el archivo completo. Para que el diff del PR muestre solo los
    # campos que cambiaron, el lock versionado tiene que estar escrito exactamente
    # con este formato (ver la prueba de idempotencia en tools/README.md).
    $json = ($Lock | ConvertTo-Json -Depth 10)
    $json = $json -replace "`r`n", "`n"
    Set-Content -Path (Get-LockPath $Raiz) -Value ($json + "`n") -Encoding utf8NoBOM -NoNewline
}

function Get-VersionSortKey {
    param([string] $Version)
    $partes = @($Version -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    if ($partes.Count -eq 0) { return '0000000000' }
    return (($partes | ForEach-Object { $_.ToString('0000000000') }) -join '.')
}

# Devuelve -1 si A < B, 0 si son iguales, 1 si A > B. Compara componente por
# componente, asi sirve igual para "5.1.1" (UPX) que para "26.00" (7-Zip).
function Compare-ToolVersion {
    param([string] $A, [string] $B)
    $pa = @($A -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    $pb = @($B -split '[^0-9]+' | Where-Object { $_ -ne '' } | ForEach-Object { [int]$_ })
    $n = [Math]::Max($pa.Count, $pb.Count)
    for ($i = 0; $i -lt $n; $i++) {
        $x = if ($i -lt $pa.Count) { $pa[$i] } else { 0 }
        $y = if ($i -lt $pb.Count) { $pb[$i] } else { 0 }
        if ($x -lt $y) { return -1 }
        if ($x -gt $y) { return 1 }
    }
    return 0
}

function Get-ApiHeaders {
    $h = @{ 'Accept' = 'application/vnd.github+json'; 'User-Agent' = 'RealViewOn-update-tools' }
    if ($GitHubToken) { $h['Authorization'] = "Bearer $GitHubToken" }
    return $h
}

function Get-LatestStableRelease {
    param([string] $Repo)
    $releases = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases?per_page=50" -Headers (Get-ApiHeaders)
    $estables = @($releases | Where-Object {
        (-not $_.draft) -and (-not $_.prerelease) -and
        ($_.tag_name -notmatch $PatronInestable) -and
        ([string]::IsNullOrEmpty($_.name) -or ($_.name -notmatch $PatronInestable))
    })
    if ($estables.Count -eq 0) { throw "$Repo no tiene ningun release estable en las ultimas 50 entradas." }
    return ($estables | Sort-Object -Property @{ Expression = { Get-VersionSortKey ($_.tag_name) } } -Descending)[0]
}

function Get-VersionFromTag {
    param([string] $Tag)
    return ($Tag -replace '^[vV]', '')
}

function Get-PeMachine {
    param([string] $Ruta)
    $fs = [System.IO.File]::OpenRead($Ruta)
    try {
        $br = New-Object System.IO.BinaryReader($fs)
        $fs.Position = 0x3C
        $peOffset = $br.ReadInt32()
        $fs.Position = $peOffset
        if ($br.ReadUInt32() -ne 0x00004550) { throw "$Ruta no es un PE valido (falta la firma 'PE')." }
        switch ($br.ReadUInt16()) {
            0x8664  { 'x64' }
            0x014c  { 'x86' }
            0xAA64  { 'ARM64' }
            default { 'desconocida' }
        }
    } finally { $fs.Dispose() }
}

function Get-Sha256 { param([string] $Ruta) (Get-FileHash -Path $Ruta -Algorithm SHA256).Hash }

# --- Pruebas funcionales -----------------------------------------------------
# Un binario que descarga bien pero no comprime no sirve. Estas pruebas usan los
# mismos flags que el pipeline de release, para que el PR llegue ya verificado.

function Test-BinarioUpx {
    param([string] $Exe, [string] $VersionEsperada, [string] $SujetoPe, [string] $Trabajo)
    $version = (& $Exe --version 2>&1) -join "`n"
    if ($version -notmatch [regex]::Escape($VersionEsperada)) {
        throw "'upx --version' no reporta $VersionEsperada. Salida: $version"
    }

    $copia = Join-Path $Trabajo 'sujeto.exe'
    Copy-Item -LiteralPath $SujetoPe -Destination $copia -Force
    $antes = (Get-Item $copia).Length

    # --ultra-brute es el flag que usa ReleaseUPXZIP.bat y msbuild.yml.
    & $Exe --ultra-brute -q $copia *> $null
    if ($LASTEXITCODE -ne 0) { throw "'upx --ultra-brute' fallo con exit code $LASTEXITCODE." }
    $despues = (Get-Item $copia).Length
    if ($despues -ge $antes) { throw "UPX no redujo el ejecutable ($antes -> $despues bytes)." }

    & $Exe -t $copia *> $null
    if ($LASTEXITCODE -ne 0) { throw "'upx -t' rechazo el ejecutable comprimido." }

    # Lo que realmente importa: el binario comprimido todavia corre.
    $salida = (& $copia 2>&1) -join "`n"
    if ([string]::IsNullOrWhiteSpace($salida)) { throw "El ejecutable comprimido no produjo salida al ejecutarse." }

    $pct = [math]::Round(((($antes - $despues) / $antes) * 100), 1)
    return "comprimio el sujeto de prueba $antes -> $despues bytes (-$pct%), 'upx -t' OK, y el binario comprimido sigue ejecutandose"
}

function Test-Binario7zr {
    param([string] $Exe, [string] $VersionEsperada, [string] $Trabajo)
    $banner = (& $Exe 2>&1) -join "`n"
    if ($banner -notmatch [regex]::Escape($VersionEsperada)) {
        throw "El banner de 7zr no reporta $VersionEsperada. Salida: $banner"
    }

    $origen = Join-Path $Trabajo 'origen'
    New-Item -ItemType Directory -Force -Path $origen | Out-Null
    Set-Content -Path (Join-Path $origen 'texto.txt') -Value ('RealViewOn ' * 800) -Encoding utf8
    Copy-Item -LiteralPath $Exe -Destination (Join-Path $origen 'binario.exe') -Force
    $esperado = Get-Sha256 (Join-Path $origen 'binario.exe')

    # Mismos flags que usa el paso "Prepare release assets".
    $archivo = Join-Path $Trabajo 'prueba.7z'
    & $Exe a -t7z -mx=9 -md=1m -ms=on $archivo (Join-Path $origen '*') *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr a' fallo con exit code $LASTEXITCODE." }
    if (-not (Test-Path $archivo)) { throw "'7zr a' no genero el archivo." }

    & $Exe t $archivo *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr t' fallo con exit code $LASTEXITCODE." }

    # Round trip completo: extraer y comparar hash, no solo confiar en el exit code.
    $destino = Join-Path $Trabajo 'extraido'
    & $Exe x $archivo "-o$destino" -y *> $null
    if ($LASTEXITCODE -ne 0) { throw "'7zr x' fallo con exit code $LASTEXITCODE." }
    $obtenido = Get-Sha256 (Join-Path $destino 'binario.exe')
    if ($obtenido -ne $esperado) { throw "El round trip corrompio los datos: $esperado != $obtenido." }

    $tam = (Get-Item $archivo).Length
    return "creo un .7z de $tam bytes con los flags de release, '7zr t' OK, y el round trip extraccion+SHA256 coincide"
}

function Invoke-PruebaFuncional {
    param($Herramienta, [string] $Exe, [string] $Version, [string] $Raiz, [string] $Trabajo)
    switch ($Herramienta.id) {
        'upx' {
            # Se comprime el 7zr.exe del repo: un PE real, chico y a mano.
            $sujeto = Join-Path $Raiz 'tools/7zr.exe'
            if (-not (Test-Path $sujeto)) { throw "Falta el sujeto de prueba $sujeto." }
            return Test-BinarioUpx -Exe $Exe -VersionEsperada $Version -SujetoPe $sujeto -Trabajo $Trabajo
        }
        '7zr'   { return Test-Binario7zr -Exe $Exe -VersionEsperada $Version -Trabajo $Trabajo }
        default { return "sin prueba funcional definida para '$($Herramienta.id)'" }
    }
}

# --- Descarga y reemplazo ----------------------------------------------------

function Resolve-AssetName {
    param([string] $Plantilla, [string] $Version)
    return ($Plantilla -replace '\{version\}', $Version)
}

function Get-BinarioDelRelease {
    param($Herramienta, $Release, [string] $Version, [string] $Trabajo)
    $nombreAsset = Resolve-AssetName $Herramienta.asset $Version
    $asset = $Release.assets | Where-Object { $_.name -eq $nombreAsset } | Select-Object -First 1
    if (-not $asset) {
        $disponibles = ($Release.assets | ForEach-Object { $_.name }) -join ', '
        throw "El release $($Release.tag_name) de $($Herramienta.repo) no publica '$nombreAsset'. Assets disponibles: $disponibles"
    }

    $descarga = Join-Path $Trabajo $nombreAsset
    Write-Host "    descargando $nombreAsset ($([math]::Round($asset.size / 1KB, 1)) KB)"
    Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $descarga -UseBasicParsing -Headers @{ 'User-Agent' = 'RealViewOn-update-tools' }

    if ($Herramienta.archiveMember) {
        $miembro = Resolve-AssetName $Herramienta.archiveMember $Version
        $extraido = Join-Path $Trabajo 'zip'
        Expand-Archive -LiteralPath $descarga -DestinationPath $extraido -Force
        $ruta = Join-Path $extraido ($miembro -replace '/', [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path $ruta)) {
            $contenido = (Get-ChildItem $extraido -Recurse -File | ForEach-Object { $_.FullName.Substring($extraido.Length + 1) }) -join ', '
            throw "El asset no contiene '$miembro'. Contenido: $contenido"
        }
        return [pscustomobject]@{ Binario = $ruta; AssetUrl = $asset.browser_download_url; AssetName = $nombreAsset }
    }

    return [pscustomobject]@{ Binario = $descarga; AssetUrl = $asset.browser_download_url; AssetName = $nombreAsset }
}

function Invoke-ToolCheck {
    param($Herramienta, [string] $Raiz)

    Write-Host "==> $($Herramienta.name) [$($Herramienta.id)]"
    $rutaLocal = Join-Path $Raiz $Herramienta.path
    if (-not (Test-Path $rutaLocal)) { throw "Falta el binario vendorizado: $rutaLocal" }

    # El lock es la referencia, pero si no coincide con el binario en disco alguien
    # lo cambio a mano y el resto del razonamiento no seria confiable.
    $shaLocal = Get-Sha256 $rutaLocal
    if ($Herramienta.sha256 -and ($shaLocal -ne $Herramienta.sha256)) {
        throw ("El SHA256 de $($Herramienta.path) no coincide con tools.lock.json.`n" +
               "  en disco : $shaLocal`n" +
               "  en lock  : $($Herramienta.sha256)`n" +
               "Actualiza el lock a mano antes de dejar que el bot lo maneje.")
    }

    $release = Get-LatestStableRelease -Repo $Herramienta.repo
    $ultima = Get-VersionFromTag $release.tag_name
    $actual = $Herramienta.version
    $cmp = Compare-ToolVersion -A $ultima -B $actual

    Write-Host "    fijada: $actual    ultimo estable: $ultima ($($release.tag_name))"

    $resultado = [pscustomobject]@{
        Id              = $Herramienta.id
        Nombre          = $Herramienta.name
        Ruta            = $Herramienta.path
        Repo            = $Herramienta.repo
        VersionActual   = $actual
        VersionNueva    = $ultima
        Tag             = $release.tag_name
        ReleaseUrl      = $release.html_url
        # Formato fijo: ConvertFrom-Json entrega un DateTime y castearlo a string
        # usaria la cultura del runner.
        PublicadoEl     = ([datetime]$release.published_at).ToUniversalTime().ToString('yyyy-MM-dd')
        HayNovedad      = ($cmp -gt 0)
        Sha256Anterior  = $shaLocal
        Sha256Nuevo     = $null
        TamanoAnterior  = (Get-Item $rutaLocal).Length
        TamanoNuevo     = $null
        AssetName       = $null
        AssetUrl        = $null
        Arquitectura    = $Herramienta.arch
        Verificacion    = $null
        Actualizado     = $false
    }

    if ($cmp -lt 0) {
        Write-Host "    la version fijada es MAS NUEVA que el ultimo estable publicado; no se toca." -ForegroundColor Yellow
        return $resultado
    }
    if ($cmp -eq 0) {
        Write-Host "    al dia." -ForegroundColor Green
        return $resultado
    }

    Write-Host "    hay novedad: $actual -> $ultima" -ForegroundColor Cyan
    if ($CheckOnly) { return $resultado }

    $trabajo = Join-Path ([System.IO.Path]::GetTempPath()) ("rvo-tools-" + $Herramienta.id + "-" + [System.Guid]::NewGuid().ToString('N').Substring(0, 8))
    New-Item -ItemType Directory -Force -Path $trabajo | Out-Null
    try {
        $descarga = Get-BinarioDelRelease -Herramienta $Herramienta -Release $release -Version $ultima -Trabajo $trabajo
        $nuevo = $descarga.Binario

        # Arquitectura: sin esto un cambio de nombre de asset podria meter un x86
        # donde habia un x64 sin que nadie lo note en el diff de un binario.
        $arch = Get-PeMachine $nuevo
        if ($arch -ne $Herramienta.arch) {
            throw "Arquitectura inesperada en $($descarga.AssetName): se esperaba $($Herramienta.arch) y es $arch."
        }

        if ($SkipFunctionalTest) {
            $resultado.Verificacion = 'OMITIDA (-SkipFunctionalTest)'
            Write-Host "    prueba funcional OMITIDA" -ForegroundColor Yellow
        } else {
            $resultado.Verificacion = Invoke-PruebaFuncional -Herramienta $Herramienta -Exe $nuevo -Version $ultima -Raiz $Raiz -Trabajo $trabajo
            Write-Host "    prueba funcional OK: $($resultado.Verificacion)" -ForegroundColor Green
        }

        # La prueba de UPX comprime una copia, pero por si acaso se re-descarga el
        # hash del binario que efectivamente se copia al repo.
        $resultado.Sha256Nuevo = Get-Sha256 $nuevo
        $resultado.TamanoNuevo = (Get-Item $nuevo).Length
        $resultado.AssetName = $descarga.AssetName
        $resultado.AssetUrl = $descarga.AssetUrl

        Copy-Item -LiteralPath $nuevo -Destination $rutaLocal -Force
        $Herramienta.version = $ultima
        $Herramienta.sha256 = $resultado.Sha256Nuevo
        $resultado.Actualizado = $true
        Write-Host "    actualizado $($Herramienta.path)" -ForegroundColor Green
    } finally {
        Remove-Item -LiteralPath $trabajo -Recurse -Force -ErrorAction SilentlyContinue
    }

    return $resultado
}

# --- Cuerpo del PR -----------------------------------------------------------
# Plantilla en here-string de comilla simple: sin interpolacion, asi los backticks
# de markdown quedan literales y no hay que escaparlos.

function New-CuerpoPr {
    param($Resultado, $Herramienta)

    $plantilla = @'
## {NOMBRE} `{VIEJA}` -> `{NUEVA}`

Actualizacion automatica de un binario vendorizado, detectada por
[`update-tools.yml`](.github/workflows/update-tools.yml).

|         | Antes         | Despues       |
| ---     | ---           | ---           |
| Version | `{VIEJA}`     | `{NUEVA}`     |
| SHA256  | `{SHA_VIEJO}` | `{SHA_NUEVO}` |
| Tamano  | {TAM_VIEJO} bytes | {TAM_NUEVO} bytes |

- **Release de origen:** {RELEASE_URL} (tag `{TAG}`, publicado {PUBLICADO})
- **Asset descargado:** [`{ASSET}`]({ASSET_URL})
- **Arquitectura:** `{ARCH}`, verificada leyendo el encabezado PE. Coincide con la fijada en el lock.
- **Usado por:** {USADO_POR}

### Verificacion automatica

El bot no se limito a descargar el binario: corrio una prueba funcional con los
mismos flags que usa el pipeline de release.

> {VERIFICACION}

### Que revisar a mano

Este PR reemplaza un ejecutable, asi que **el diff no es legible**. Antes de
aprobar, compara el SHA256 de la tabla contra el que publica el proveedor:

- UPX: <https://github.com/upx/upx/releases>
- 7-Zip: <https://www.7-zip.org/download.html>

Ninguno de los dos upstreams publica un archivo de checksums en sus releases de
GitHub, asi que el bot **no verifica procedencia criptografica**: solo comprueba
que el binario venga del repositorio oficial, que tenga la arquitectura esperada y
que funcione. Comparar el hash queda a cargo de quien revisa.

---
Este PR lo creo `GITHUB_TOKEN`, por lo que **no dispara otros workflows**.
'@

    $mapa = [ordered]@{
        '{NOMBRE}'      = [string]$Resultado.Nombre
        '{VIEJA}'       = [string]$Resultado.VersionActual
        '{NUEVA}'       = [string]$Resultado.VersionNueva
        '{SHA_VIEJO}'   = [string]$Resultado.Sha256Anterior
        '{SHA_NUEVO}'   = [string]$Resultado.Sha256Nuevo
        '{TAM_VIEJO}'   = [string]$Resultado.TamanoAnterior
        '{TAM_NUEVO}'   = [string]$Resultado.TamanoNuevo
        '{RELEASE_URL}' = [string]$Resultado.ReleaseUrl
        '{TAG}'         = [string]$Resultado.Tag
        '{PUBLICADO}'   = [string]$Resultado.PublicadoEl
        '{ASSET}'       = [string]$Resultado.AssetName
        '{ASSET_URL}'   = [string]$Resultado.AssetUrl
        '{ARCH}'        = [string]$Resultado.Arquitectura
        '{USADO_POR}'   = [string]$Herramienta.usedBy
        '{VERIFICACION}' = [string]$Resultado.Verificacion
    }

    $md = $plantilla
    foreach ($clave in $mapa.Keys) { $md = $md.Replace($clave, $mapa[$clave]) }
    return $md
}

# --- Principal ---------------------------------------------------------------

$raiz = Resolve-RepoRoot $RepoRoot
$lock = Read-Lock $raiz

# El @() externo es necesario: una rama de if que devuelve un array vacio se
# colapsa a $null al asignarla, y con Set-StrictMode el .Count siguiente explota
# con un error que no dice nada.
$seleccionadas = @(if ($Id -eq 'all') { $lock.tools } else { $lock.tools | Where-Object { $_.id -eq $Id } })
if ($seleccionadas.Count -eq 0) {
    $validos = (@($lock.tools) | ForEach-Object { $_.id }) -join ', '
    throw "No hay ninguna herramienta con id '$Id' en tools/tools.lock.json. Ids validos: $validos"
}

$resultados = @()
foreach ($h in $seleccionadas) {
    $r = Invoke-ToolCheck -Herramienta $h -Raiz $raiz
    $resultados += $r
    if ($r.Actualizado -and $PrBodyPath) {
        New-CuerpoPr -Resultado $r -Herramienta $h |
            Set-Content -Path $PrBodyPath -Encoding utf8NoBOM
        Write-Host "    cuerpo del PR escrito en $PrBodyPath"
    }
}

if (@($resultados | Where-Object { $_.Actualizado }).Count -gt 0) { Write-Lock -Raiz $raiz -Lock $lock }

# Salidas para GitHub Actions (un solo id por corrida en el workflow).
if ($env:GITHUB_OUTPUT -and $resultados.Count -eq 1) {
    $r = $resultados[0]
    $pares = [ordered]@{
        updated          = $r.Actualizado.ToString().ToLower()
        has_update       = $r.HayNovedad.ToString().ToLower()
        current_version  = $r.VersionActual
        latest_version   = $r.VersionNueva
        tag              = $r.Tag
        release_url      = [string]$r.ReleaseUrl
        asset_name       = [string]$r.AssetName
        asset_url        = [string]$r.AssetUrl
        sha256_old       = [string]$r.Sha256Anterior
        sha256_new       = [string]$r.Sha256Nuevo
        size_old         = [string]$r.TamanoAnterior
        size_new         = [string]$r.TamanoNuevo
        arch             = [string]$r.Arquitectura
        verification     = [string]$r.Verificacion
        name             = [string]$r.Nombre
        repo             = [string]$r.Repo
        path             = [string]$r.Ruta
        published_at     = [string]$r.PublicadoEl
    }
    # GITHUB_OUTPUT usa "clave=valor" por linea: un salto de linea en un valor
    # rompe el formato (y es un vector de inyeccion conocido). Todos los valores de
    # aca son de una sola linea, pero se aplanan por si acaso.
    foreach ($k in $pares.Keys) {
        $v = ([string]$pares[$k]) -replace '[\r\n]+', ' '
        "$k=$v" | Out-File -FilePath $env:GITHUB_OUTPUT -Append -Encoding utf8
    }
}

Write-Host ''
$resultados | Format-Table Id, VersionActual, VersionNueva, HayNovedad, Actualizado -AutoSize
return $resultados
