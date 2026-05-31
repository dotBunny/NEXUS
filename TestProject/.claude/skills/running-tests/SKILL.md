---
name: running-tests
description: Describes how to run unit tests, test cases, or performance tests. Invoke this skill whenever running tests for any NEXUS plugin.
---

# Running Tests

## Prerequisites

Build the project first. Use the `build` skill to compile `NEXUSEditor` if any source files changed.

```powershell
# Unit tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="___PROJECTROOT___\Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests;Quit" -nullrhi

# Functional tests (no -nullrhi: these tests render to screen and require a real graphics context)
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="___PROJECTROOT___\Staging\TestResults" -log -ExecCmds="Automation RunTest Tests.Nexus;Quit"

# Performance tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="___PROJECTROOT___\Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.PerfTests;Quit" -nullrhi
```

## Reading Results

**The log is the authoritative source of truth — read it, not the JSON report.** Always pass `-ReportExportPath` as an absolute path (`___PROJECTROOT___\Staging\TestResults`); when relative it resolves against the process working directory and a previous run's `index.json` is frequently left in place, so a stale file looks like a passing run.

The reliable, always-written record is the log at `___PROJECTROOT___\Saved\Logs\NEXUS.log` (the active run; previous runs rotate to `NEXUS-backup-<timestamp>.log`). Per-test outcomes appear as:

```
LogAutomationController: Display: Test Completed. Result={Success} Name={...} Path={...}
```

To confirm a run, grep the log for `Result={` — every `Result={Success}` passed; any `Result={Fail` failed. This works even when `index.json` was not refreshed.

The optional report files under `Staging/TestResults/`:

| File | Contents |
|---|---|
| `index.json` | Machine-readable results — **verify `reportCreatedOn` is from this run before trusting it** (it can be stale; see above) |
| `index.md` | Human-readable summary table (only available when run through GitHub actions) |
| `performance.csv` | Timer data extracted from perf tests (only present after a perf run through GitHub actions) |

## Filtering Tests

To run only a specific plugin's tests, narrow the filter prefix:

```powershell
# Only ActorPools unit tests
& "___UEROOT___\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "___PROJECTROOT___\NEXUS.uproject" -unattended -nopause -testexit="Automation Test Queue Empty" -ReportExportPath="___PROJECTROOT___\Staging\TestResults" -log -ExecCmds="Automation RunTest NEXUS.UnitTests.NActorPools;Quit" -nullrhi
```

To run a single test, use its fully-qualified name:

```powershell
-ExecCmds="Automation RunTest NEXUS::UnitTests::NActorPools::LeakCheck::DontForceDestroy;Quit"
```

The test name is the string passed as the second argument to the `N_TEST_*` macro.

## Debugging Failures

If a run crashes or tests produce unexpected results, check `___PROJECTROOT___\Saved\Logs\NEXUS.log` for `Error:` or `Warning:` lines, and the `Result={...}` line for the specific test. A failed assertion (`ADD_ERROR` / `CHECK_*`) is logged immediately before its `Test Completed` line.
