[CmdletBinding()]
param(
    [switch] $Json
)

$ErrorActionPreference = 'Stop'

function Get-CommandRecord {
    param(
        [Parameter(Mandatory)]
        [string] $Name,

        [string[]] $VersionArguments = @('--version'),

        [bool] $Required = $true
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -eq $command) {
        return [ordered]@{
            name     = $Name
            required = $Required
            found    = $false
            path     = $null
            version  = $null
        }
    }

    $version = $null
    try {
        $version = (& $command.Source @VersionArguments 2>&1 | Select-Object -First 1).ToString().Trim()
    }
    catch {
        $version = "Found; version query failed: $($_.Exception.Message)"
    }

    return [ordered]@{
        name     = $Name
        required = $Required
        found    = $true
        path     = $command.Source
        version  = $version
    }
}

function Find-VisualStudio {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
        return [ordered]@{
            found        = $false
            installation = $null
            version      = $null
            hasCppTools  = $false
        }
    }

    $installation = (& $vswhere -latest -products '*' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath | Select-Object -First 1).Trim()

    if ([string]::IsNullOrWhiteSpace($installation)) {
        return [ordered]@{
            found        = $true
            installation = $null
            version      = $null
            hasCppTools  = $false
        }
    }

    $version = (& $vswhere -latest -products '*' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationVersion | Select-Object -First 1).Trim()

    return [ordered]@{
        found        = $true
        installation = $installation
        version      = $version
        hasCppTools  = $true
    }
}

function Find-File {
    param(
        [string[]] $Candidates = @()
    )

    foreach ($candidate in $Candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }

    return $null
}

$isWindowsHost = $PSVersionTable.PSEdition -eq 'Desktop' -or $IsWindows
if (-not $isWindowsHost) {
    throw 'This audit must run in PowerShell on Windows 10 or Windows 11.'
}

$os = Get-CimInstance Win32_OperatingSystem
$computer = Get-CimInstance Win32_ComputerSystem
$videoControllers = @(Get-CimInstance Win32_VideoController | ForEach-Object { $_.Name })
$visualStudio = Find-VisualStudio

$signtoolCandidates = @(
    Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" `
        -Filter signtool.exe -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match '\\x64\\signtool\.exe$' } |
        Sort-Object FullName -Descending |
        ForEach-Object { $_.FullName }
)

$innoCandidates = @(
    (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe')
)

$flStudioCandidates = @(
    (Join-Path $env:ProgramFiles 'Image-Line\FL Studio 2025\FL64.exe'),
    (Join-Path $env:ProgramFiles 'Image-Line\FL Studio 2024\FL64.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Image-Line\FL Studio 2025\FL64.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Image-Line\FL Studio 2024\FL64.exe')
)

$tools = @(
    (Get-CommandRecord -Name git),
    (Get-CommandRecord -Name cmake),
    (Get-CommandRecord -Name ninja),
    (Get-CommandRecord -Name cl -VersionArguments @()),
    (Get-CommandRecord -Name python -VersionArguments @('--version') -Required $false)
)

$cmakeTool = $tools | Where-Object { $_.name -eq 'cmake' } | Select-Object -First 1
$cmakeVersionSupported = $false
if ($cmakeTool.found -and $cmakeTool.version -match '(\d+)\.(\d+)\.(\d+)') {
    $cmakeVersionSupported = [version] $Matches[0] -ge [version] '3.22.0'
}

$windowsVersionSupported = [version] $os.Version -ge [version] '10.0'

$juceRootConfigured = -not [string]::IsNullOrWhiteSpace($env:JUCE_ROOT)
$juceRootExists = $juceRootConfigured -and (Test-Path -LiteralPath $env:JUCE_ROOT -PathType Container)

$result = [ordered]@{
    auditVersion = 1
    capturedAt   = (Get-Date).ToUniversalTime().ToString('o')
    powerShell = [ordered]@{
        edition = $PSVersionTable.PSEdition
        version = $PSVersionTable.PSVersion.ToString()
    }
    operatingSystem = [ordered]@{
        caption        = $os.Caption
        version        = $os.Version
        buildNumber    = $os.BuildNumber
        architecture   = $os.OSArchitecture
        is64Bit        = [Environment]::Is64BitOperatingSystem
        supported      = $windowsVersionSupported
    }
    hardware = [ordered]@{
        manufacturer = $computer.Manufacturer
        model        = $computer.Model
        logicalCpus  = $computer.NumberOfLogicalProcessors
        memoryBytes  = [int64] $computer.TotalPhysicalMemory
        video        = $videoControllers
    }
    visualStudio = $visualStudio
    tools = $tools
    windowsSdk = [ordered]@{
        signToolX64 = (Find-File -Candidates $signtoolCandidates)
    }
    innoSetup = [ordered]@{
        compiler = (Find-File -Candidates $innoCandidates)
    }
    flStudio = [ordered]@{
        executable = (Find-File -Candidates $flStudioCandidates)
    }
    juce = [ordered]@{
        rootConfigured = $juceRootConfigured
        rootExists     = $juceRootExists
    }
    requirements = [ordered]@{
        cmakeAtLeast3_22 = $cmakeVersionSupported
        msvcShellActive  = [bool] ($tools | Where-Object { $_.name -eq 'cl' -and $_.found })
    }
    vst3 = [ordered]@{
        userDevelopmentPath = Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3\Ehvota'
        globalInstallPath   = Join-Path $env:CommonProgramFiles 'VST3\Ehvota'
    }
}

$requiredToolsPresent = @($tools | Where-Object { $_.required } | ForEach-Object { $_.found }) -notcontains $false
$baselineReady = (
    $result.operatingSystem.is64Bit -and
    $result.operatingSystem.supported -and
    $visualStudio.hasCppTools -and
    $requiredToolsPresent -and
    $cmakeVersionSupported
)

$result['baselineReady'] = $baselineReady

if ($Json) {
    $result | ConvertTo-Json -Depth 8
}
else {
    Write-Host "Ehvota Windows development audit"
    Write-Host "OS: $($os.Caption) $($os.Version) build $($os.BuildNumber) ($($os.OSArchitecture))"
    Write-Host "PowerShell: $($result.powerShell.edition) $($result.powerShell.version)"
    Write-Host "Visual Studio C++ tools: $($visualStudio.hasCppTools)"

    foreach ($tool in $tools) {
        $requirement = if ($tool.required) { 'required' } else { 'optional' }
        Write-Host "$($tool.name): found=$($tool.found), $requirement, version=$($tool.version)"
    }

    Write-Host "Windows SDK signtool x64: $($result.windowsSdk.signToolX64)"
    Write-Host "Inno Setup compiler: $($result.innoSetup.compiler)"
    Write-Host "FL Studio: $($result.flStudio.executable)"
    Write-Host "JUCE_ROOT configured and valid: $juceRootExists"
    Write-Host "CMake >= 3.22: $cmakeVersionSupported"
    Write-Host "MSVC developer shell active: $($result.requirements.msvcShellActive)"
    Write-Host "Baseline configure readiness: $baselineReady"
}

if (-not $baselineReady) {
    exit 1
}
