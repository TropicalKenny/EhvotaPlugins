# Build and test

Status: JUCE target and portable tests defined; Windows execution remains pending.

## Supported product environment

- Windows 10 or Windows 11, x64.
- Visual Studio or Build Tools with **Desktop development with C++**.
- MSVC x64 toolset and a Windows 10/11 SDK.
- CMake 3.22 or newer, matching JUCE's documented minimum.
- Git.
- Ninja is required by the provided `Ninja Multi-Config` Windows preset but is not a product runtime dependency.
- FL Studio x64 is required for authoritative host validation.
- Inno Setup and the Windows SDK signing tools are later release dependencies.

Do not install preview/Insider compiler components for the baseline unless a documented requirement makes them necessary.

## Windows machine audit

Before configuring the project, capture:

```powershell
Get-ComputerInfo |
  Select-Object WindowsProductName, WindowsVersion, OsBuildNumber,
                CsSystemType, CsProcessors, CsTotalPhysicalMemory

git --version
cmake --version
ninja --version
where.exe cl
where.exe msbuild
where.exe signtool
where.exe iscc
```

Run compiler/build commands from **Developer PowerShell for VS** or after importing the Visual Studio developer environment. The audit requires `cl` to be active because the preset uses Ninja rather than MSBuild to discover MSVC. Do not mistake an unrelated `signtool` executable for the Windows SDK Authenticode tool.

The repository will add `tools/check-windows-dev.ps1` to turn this into a repeatable redacted diagnostic.

## Planned configure/build commands

Clone or initialize the pinned dependency:

```powershell
git submodule update --init --recursive
```

Then run from **Developer PowerShell for Visual Studio**:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Release:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

The first Phase 1 target will be:

```powershell
cmake --build --preset windows-msvc-debug --target EhvotaUIDemo
.\build\windows-msvc-debug\apps\ui-preview\EhvotaUIDemo.exe
```

Exact output paths must be updated from observed CMake output rather than assumed.

## JUCE dependency

The user accepted JUCE 8 Starter and the JUCE 8 EULA on 2026-07-23. The repository pins JUCE 8.0.9 at commit:

```text
f72bad64d29715216226685810c5196bd0d79d77
```

The default source is `third_party/JUCE`. To use an equivalent accepted local checkout:

```powershell
$env:JUCE_ROOT = 'C:\Dev\JUCE'
cmake --preset windows-msvc
```

CMake never downloads JUCE or accepts its licence automatically. Verify CMake/MSVC compatibility on Windows before treating the dependency gate as complete.

## UI Preview validation

Run the standalone preview outside FL Studio and exercise every scenario in `docs/USER_FLOWS.md`.

At each of 100%, 125%, 150%, 175%, and 200% Windows display scaling:

1. Start the application on the scaled display.
2. Test minimum, default, wide, and maximized layouts.
3. Resize continuously across layout transitions.
4. Check text, focus rings, hit targets, graph labels, popovers, tooltips, and long names.
5. If multiple DPI displays exist, move the open window between them.
6. Change scaling while the app is running and confirm a crisp, correctly reflowed result.
7. Save screenshots with scenario, window size, scale, theme, and commit in the filename/metadata.

Keyboard/accessibility:

1. Complete every primary action without a mouse.
2. Verify logical Tab and Shift+Tab order.
3. Verify arrow navigation inside composite controls.
4. Verify Space/Enter activation, Escape dismissal, and visible focus.
5. Use Windows Narrator to confirm control name, role, value, and state.
6. Run Windows Accessibility Insights or UIA Verify when available.
7. Test high contrast and reduced-motion behavior.

Review artefacts belong under `generated/ui-review/` and must not enter production binaries.

## VST3 development staging

Once plugin targets exist, stage x64 bundles without administrator privileges:

```text
%LOCALAPPDATA%\Programs\Common\VST3\Ehvota\
```

The release installer uses:

```text
C:\Program Files\Common Files\VST3\Ehvota\
```

Do not copy a single DLL and call it a complete VST3 package; validate the bundle structure and `moduleinfo.json` where used.

## FL Studio validation outline

1. Open Plugin Manager and add/confirm the standard VST3 path.
2. Find installed plugins and confirm vendor/name/category/bitness.
3. Test insert, duplicate instances, save/reopen, preset/state recall, automation, renaming, and link recovery.
4. Exercise sample rates, buffer sizes, tempo automation, meter changes, loops, transport jumps, and offline export.
5. Test Smart Disable both enabled and disabled; time-based or metering behavior must not silently break.
6. Inspect reported latency and PDC. Intentional RefTime musical delay must not appear as technical plugin latency.
7. Verify monitoring/reference/simulation audio cannot enter export.
8. Record FL Studio version, Windows build, sample rate, buffer, plugin build/commit, and result in `docs/FL_STUDIO_VALIDATION.md`.

## Evidence policy

- A successful configure is not a successful build.
- A successful build is not a passing test suite.
- A standalone run is not VST3 validation.
- VST3 validation is not FL Studio validation.
- Linux output is not Windows evidence.
- A screenshot is layout evidence only for its recorded state, size, scale, and build.
- Failures and skipped platform gates must remain visible.

## First-party references

- [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- [Install the Microsoft C++ Build Tools](https://learn.microsoft.com/en-us/cpp/overview/acquire-msvc)
- [High DPI desktop development on Windows](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows)
- [Windows accessibility testing](https://learn.microsoft.com/en-us/windows/apps/design/accessibility/accessibility-testing)
- [Steinberg VST3 package structure](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BFormat.html)
- [Steinberg VST3 locations](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Locations%2BFormat/Plugin%2BLocations.html)
- [FL Studio Plugin Wrapper](https://www.image-line.com/fl-studio-learning-content/fl-studio-online-manual/html/plugins/wrapper.htm)
- [FL Studio installing plugins](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)
- [Inno Setup help](https://jrsoftware.org/ishelp/contents.htm)
