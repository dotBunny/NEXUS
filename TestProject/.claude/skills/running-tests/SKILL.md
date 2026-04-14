---
name: running-tests
description: Describes how to run unit tests, test cases, or performance tests. Invoke this skill whenever running tests for any NEXUS plugin.
---

## Build

The project uses precompiled editor binaries found in the `___UEROOT___\Engine\Binaries\Win64` directory. 
There is no reason to compile the Unreal Engine or the project before executing tests.

# Running Tests

When running tests, follow these guidelines exactly.

```powershell
# Unit tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests;Quit" -nullrhi

# Functional tests (no -nullrhi: these tests render to screen and require a real graphics context)
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest Tests.Nexus;Quit"

# Performance tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.PerfTests;Quit" -nullrhi
```

Results are written to `Staging/TestResults/`:

| File | Contents |
|---|---|
| `index.json` | Full machine-readable test results |
| `index.md` | Human-readable summary table (only available when ran through GitHub actions) |
| `performance.csv` | Timer data extracted from perf tests (only present after a perf run through GitHub actions) |

## Filtering Tests

To run only a specific plugin's tests, narrow the filter prefix:

```powershell
# Only ActorPools unit tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests.NActorPools;Quit" -nullrhi
```

To run a single test, use its fully-qualified name:

```powershell
-ExecCmds="Automation RunTest NEXUS::UnitTests::NActorPools::LeakCheck::DontForceDestroy;Quit"
```

The test name is the string passed as the second argument to the `N_TEST_*` macro.

## Debugging Failures

If a run crashes or tests produce unexpected results, check the editor log:

`TestProject/Saved/Logs/NEXUS.log`

Look for lines containing `Error:` or `Warning:` to identify the root cause.