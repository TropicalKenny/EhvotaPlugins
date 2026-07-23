[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [string] $Executable,

    [Parameter(Mandatory)]
    [ValidateSet(100, 125, 150, 175, 200)]
    [int] $ExpectedScalePercent,

    [ValidateSet('Dpi', 'Journeys', 'Core', 'Full')]
    [string] $Matrix = 'Dpi',

    [string] $OutputRoot = 'generated\ui-review'
)

$ErrorActionPreference = 'Stop'

$isWindowsHost = $PSVersionTable.PSEdition -eq 'Desktop' -or $IsWindows
if (-not $isWindowsHost) {
    throw 'UI review capture must run in PowerShell on Windows 10 or Windows 11.'
}

$executableFile = Get-Item -LiteralPath $Executable -ErrorAction Stop
if ($executableFile.PSIsContainer -or $executableFile.Extension -ne '.exe') {
    throw "Executable must identify the built Ehvota UI Preview .exe: $Executable"
}

Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
if (-not ('EhvotaNativeCapture' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;

public static class EhvotaNativeCapture
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Rect
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [DllImport("user32.dll")]
    public static extern uint GetDpiForWindow(IntPtr window);

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool SetForegroundWindow(IntPtr window);

    [DllImport("user32.dll")]
    public static extern IntPtr GetForegroundWindow();

    [DllImport("user32.dll")]
    public static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);

    [DllImport("dwmapi.dll")]
    public static extern int DwmGetWindowAttribute(
        IntPtr window,
        int attribute,
        out Rect value,
        int valueSize);
}
'@
}

function New-CaptureItem {
    param(
        [string] $Scenario,
        [string] $Layout,
        [int] $Width,
        [int] $Height,
        [int] $Step = 0
    )

    [pscustomobject]@{
        scenario = $Scenario
        layout   = $Layout
        width    = $Width
        height   = $Height
        step     = $Step
    }
}

$primaryJourneys = @(
    'reference-ready',
    'compare-chorus',
    'mapping-disconnected',
    'components-no-drums',
    'capture-running',
    'proposal-review',
    'reftime-120',
    'stem-prep-queue'
)

$requiredStates = @(
    'low-confidence',
    'render-safe',
    'model-missing',
    'library-empty',
    'reference-missing-stem',
    'reference-invalid-length',
    'capture-failed',
    'proposal-applied',
    'bridge-unsupported',
    'high-cpu',
    'model-license-error',
    'stem-prep-cancelled',
    'installer-repair'
)

if ($Matrix -eq 'Dpi') {
    $captures = @(
        (New-CaptureItem 'compare-chorus' 'compact-minimum' 720 500),
        (New-CaptureItem 'reftime-120' 'compact-minimum' 720 500),
        (New-CaptureItem 'stem-prep-queue' 'compact-minimum' 720 500)
    )
}
elseif ($Matrix -eq 'Journeys') {
    $journeyStepCounts = [ordered]@{
        'reference-ready'       = 7
        'compare-chorus'        = 7
        'mapping-disconnected'  = 7
        'components-no-drums'   = 6
        'capture-running'       = 6
        'proposal-review'       = 8
        'reftime-120'           = 8
        'stem-prep-queue'       = 10
    }

    $captures = @()
    foreach ($scenario in $journeyStepCounts.Keys) {
        for ($step = 1; $step -le $journeyStepCounts[$scenario]; $step++) {
            $captures += New-CaptureItem $scenario 'standard' 1180 760 $step
        }
    }
}
else {
    $captures = @()
    foreach ($scenario in $primaryJourneys) {
        $captures += New-CaptureItem $scenario 'standard' 1180 760
    }

    if ($Matrix -eq 'Full') {
        foreach ($scenario in $requiredStates) {
            $captures += New-CaptureItem $scenario 'standard' 1180 760
        }
    }

    foreach ($scenario in @('compare-chorus', 'reftime-120', 'stem-prep-queue')) {
        $captures += New-CaptureItem $scenario 'compact' 720 640
        $captures += New-CaptureItem $scenario 'wide' 1440 900
    }
}

$sessionName = '{0}-{1}pct-{2}' -f `
    (Get-Date -Format 'yyyyMMdd-HHmmss'), $ExpectedScalePercent, $Matrix.ToLowerInvariant()
$sessionDirectory = Join-Path $OutputRoot $sessionName
New-Item -ItemType Directory -Path $sessionDirectory -Force | Out-Null

$executableHash = (Get-FileHash -LiteralPath $executableFile.FullName -Algorithm SHA256).Hash
$gitCommit = $null
try {
    $gitCommit = (& git rev-parse HEAD 2>$null | Select-Object -First 1).Trim()
}
catch {
    $gitCommit = $null
}

$records = @()
$perMonitorAwareV2 = [IntPtr]::new(-4)
$previousDpiContext = [EhvotaNativeCapture]::SetThreadDpiAwarenessContext($perMonitorAwareV2)

try {
    foreach ($capture in $captures) {
        $process = $null
        try {
            $arguments = @(
                "--scenario=$($capture.scenario)",
                "--width=$($capture.width)",
                "--height=$($capture.height)"
            )
            if ($capture.step -gt 0) {
                $arguments += "--step=$($capture.step)"
            }
            $process = Start-Process `
                -FilePath $executableFile.FullName `
                -ArgumentList $arguments `
                -PassThru

            try {
                [void] $process.WaitForInputIdle(10000)
            }
            catch {
                # Continue to the explicit main-window timeout below.
            }

            $deadline = (Get-Date).AddSeconds(10)
            do {
                Start-Sleep -Milliseconds 100
                $process.Refresh()
                $window = $process.MainWindowHandle
            } while ($window -eq [IntPtr]::Zero -and -not $process.HasExited -and (Get-Date) -lt $deadline)

            if ($process.HasExited) {
                throw "Preview exited before capture with code $($process.ExitCode)."
            }
            if ($window -eq [IntPtr]::Zero) {
                throw 'Preview did not expose a main window within 10 seconds.'
            }

            $dpi = [int] [EhvotaNativeCapture]::GetDpiForWindow($window)
            $scalePercent = [int] [Math]::Round(($dpi / 96.0) * 100.0)
            if ($scalePercent -ne $ExpectedScalePercent) {
                throw "Window DPI is $dpi ($scalePercent%), expected $ExpectedScalePercent%. Move PowerShell to the intended display or change Windows Display scaling, then rerun."
            }

            if (-not [EhvotaNativeCapture]::SetForegroundWindow($window)) {
                throw 'Windows did not allow the preview window to come to the foreground.'
            }
            Start-Sleep -Milliseconds 650
            if ([EhvotaNativeCapture]::GetForegroundWindow() -ne $window) {
                throw 'Preview is not the foreground window; native capture could be occluded.'
            }

            $rectangle = [EhvotaNativeCapture+Rect]::new()
            $dwmResult = [EhvotaNativeCapture]::DwmGetWindowAttribute(
                $window,
                9,
                [ref] $rectangle,
                [Runtime.InteropServices.Marshal]::SizeOf($rectangle))
            if ($dwmResult -ne 0) {
                throw "DwmGetWindowAttribute failed with HRESULT $dwmResult."
            }

            $pixelWidth = $rectangle.Right - $rectangle.Left
            $pixelHeight = $rectangle.Bottom - $rectangle.Top
            if ($pixelWidth -le 0 -or $pixelHeight -le 0) {
                throw "Invalid native window bounds: ${pixelWidth}x${pixelHeight}."
            }

            $virtualScreen = [System.Windows.Forms.SystemInformation]::VirtualScreen
            if ($rectangle.Left -lt $virtualScreen.Left `
                -or $rectangle.Top -lt $virtualScreen.Top `
                -or $rectangle.Right -gt $virtualScreen.Right `
                -or $rectangle.Bottom -gt $virtualScreen.Bottom) {
                throw 'The preview window does not fit fully within the Windows virtual desktop. Use the Dpi matrix, a higher-resolution display, or a lower display scale; clipped screenshots are rejected.'
            }

            $stepSuffix = if ($capture.step -gt 0) {
                '-step{0:D2}' -f $capture.step
            }
            else {
                ''
            }
            $fileName = '{0}{1}-{2}-{3}x{4}-{5}pct.png' -f `
                $capture.scenario, $stepSuffix, $capture.layout, `
                $capture.width, $capture.height, $scalePercent
            $outputPath = Join-Path $sessionDirectory $fileName
            $bitmap = New-Object System.Drawing.Bitmap $pixelWidth, $pixelHeight
            $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.CopyFromScreen(
                    $rectangle.Left,
                    $rectangle.Top,
                    0,
                    0,
                    $bitmap.Size,
                    [System.Drawing.CopyPixelOperation]::SourceCopy)
                $bitmap.Save($outputPath, [System.Drawing.Imaging.ImageFormat]::Png)
            }
            finally {
                $graphics.Dispose()
                $bitmap.Dispose()
            }

            $recordedJourneyStep = $null
            if ($capture.step -gt 0) {
                $recordedJourneyStep = $capture.step
            }

            $records += [pscustomobject]@{
                scenario          = $capture.scenario
                journeyStep       = $recordedJourneyStep
                layout            = $capture.layout
                logicalContent    = [pscustomobject]@{
                    width  = $capture.width
                    height = $capture.height
                }
                windowDpi         = $dpi
                scalePercent      = $scalePercent
                nativeWindow      = [pscustomobject]@{
                    width  = $pixelWidth
                    height = $pixelHeight
                }
                file              = $fileName
                sha256            = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash
            }
        }
        finally {
            if ($null -ne $process -and -not $process.HasExited) {
                [void] $process.CloseMainWindow()
                if (-not $process.WaitForExit(3000)) {
                    Stop-Process -Id $process.Id -Force
                }
            }
        }
    }
}
finally {
    if ($previousDpiContext -ne [IntPtr]::Zero) {
        [void] [EhvotaNativeCapture]::SetThreadDpiAwarenessContext($previousDpiContext)
    }
}

$os = Get-CimInstance Win32_OperatingSystem
$manifest = [ordered]@{
    schemaVersion        = 2
    capturedAt           = (Get-Date).ToUniversalTime().ToString('o')
    matrix               = $Matrix
    expectedScalePercent = $ExpectedScalePercent
    operatingSystem      = [ordered]@{
        caption     = $os.Caption
        version     = $os.Version
        buildNumber = $os.BuildNumber
    }
    executable           = $executableFile.FullName
    executableSha256     = $executableHash
    gitCommit            = $gitCommit
    captures             = $records
}

$manifestPath = Join-Path $sessionDirectory 'manifest.json'
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

Write-Host "Captured $($records.Count) native Windows preview images."
Write-Host "Manifest: $manifestPath"
