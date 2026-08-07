---
name: sonar-issues
description: Query the SonarCloud static-analysis issues already scanned for this project. Invoke this skill when checking what SonarQube flags on a file, reviewing changed files against static analysis, or triaging the project's open code smells and bugs.
---

# Checking SonarCloud Issues

The repo is analysed by SonarCloud as project `dotBunny_NEXUS` in org `dotbunny`. This skill reads the **already-scanned** results. For the rules themselves and how to write to them, use the `coding-style` skill.

## Before you trust the numbers

The API returns issues from the **last analysed revision**, not the working tree. Uncommitted or unpushed code has never been scanned, so a clean result on a file you just edited means nothing. Always check freshness first:

```bash
curl -s "https://sonarcloud.io/api/project_analyses/search?project=dotBunny_NEXUS&ps=1" \
  | python -c "import sys,json;a=json.load(sys.stdin)['analyses'][0];print(a['date'],a.get('revision'))"
cd .. && git rev-parse HEAD
```

If the revisions differ, say so when reporting — the results describe an older commit. Never present scanned issues as if they covered local edits.

## Mechanics

- Use the **Bash** tool with `curl`. `WebFetch` cannot resolve `sonarcloud.io` in this environment, and the API needs no auth for this project.
- `organization=dotbunny` is **required** on every call; omitting it returns `400 The 'organization' parameter is missing`.
- File paths are **repo-root-relative** (`Plugins/Core/Source/...`). The working directory is `TestProject/`, so `cd ..` before running `git` commands that produce paths.
- Scope a query to a file with `componentKeys=dotBunny_NEXUS:<path>`. The `files=` and `inNewCodePeriod=` parameters are silently **ignored** — don't use them, they return the whole project and look like a pass.
- Rule descriptions come back empty from the API. For the prose rationale open the UI: `https://sonarcloud.io/organizations/dotbunny/rules?open=cpp%3AS2681&q=cpp%3AS2681`.

## Queries

Set the base once per shell call:

```bash
B="https://sonarcloud.io/api/issues/search?organization=dotbunny&resolved=false"
```

### Everything open, grouped by rule

The triage starting point.

```bash
curl -s "$B&componentKeys=dotBunny_NEXUS&facets=rules&ps=1" | python -c "
import sys,json
d=json.load(sys.stdin)
print('open issues:', d['total'])
for f in d['facets']:
    if f['property']=='rules':
        for v in f['values']: print(f\"{v['count']:5}  {v['val']}\")
"
```

### One file

```bash
curl -s "$B&componentKeys=dotBunny_NEXUS:Plugins/WorldAssembly/Source/NexusWorldAssemblyEditor/Private/Operations/NWorldAssemblyEditorCellOperations.cpp&ps=100" | python -c "
import sys,json
for i in json.load(sys.stdin)['issues']:
    print(f\"line {i.get('line','?'):>5}  {i['rule']:14} {i['message']}\")
"
```

### One rule, across the project

```bash
curl -s "$B&componentKeys=dotBunny_NEXUS&rules=cpp:S2681&ps=100" | python -c "
import sys,json
for i in json.load(sys.stdin)['issues']:
    print(f\"{i['component'].split(':',1)[-1]}:{i.get('line','?')}  {i['message']}\")
"
```

### Files changed on this branch

The common review flow. Reports a count per file, skipping clean ones.

```bash
cd ..
for F in $(git diff --name-only main...HEAD | grep -E '\.(cpp|h)$'); do
  N=$(curl -s "https://sonarcloud.io/api/issues/search?organization=dotbunny&resolved=false&componentKeys=dotBunny_NEXUS:$F&ps=1" \
      | python -c "import sys,json;print(json.load(sys.stdin).get('total','ERR'))")
  [ "$N" != "0" ] && echo "$N  $F"
done
```

Swap `main...HEAD` for `HEAD` (with `git diff-tree --no-commit-id --name-only -r HEAD`) to scope to the last commit, or `--cached` for staged work — but remember those paths may not be in the analysed revision yet.

### Quality gate status

```bash
curl -s "https://sonarcloud.io/api/qualitygates/project_status?projectKey=dotBunny_NEXUS" | python -c "
import sys,json
s=json.load(sys.stdin)['projectStatus']
print('gate:', s['status'])
for c in s['conditions']: print(f\"  {c['metricKey']:34} {c['status']:6} {c.get('actualValue')} (threshold {c.get('errorThreshold')})\")
"
```

The gate is scored on **new code** only (`new_reliability_rating`, `new_maintainability_rating`, `new_duplicated_lines_density`, …), so it can read `OK` while the project still carries a backlog of open issues. Report both — a passing gate is not "no issues".

## Acting on results

Pagination defaults to 100 per page (`ps`, max 500) with `p` for the page; the project's open set is small enough that one page is usually enough, but check `total` against what you received before concluding.

For each issue on code in scope, decide between two outcomes and say which you chose:

1. **Fix it.** Default. Most hits are mechanical (missing braces, an unused local, an assignment inside a condition).
2. **Suppress it** with the `#SONARQUBE-DISABLE-CPP_S<id>` marker convention documented in the `coding-style` skill — only when the code is deliberate and the reason can be stated in one line. A marker records an accepted violation; it does not clear the issue from the dashboard.

Do not fix issues in files outside the scope you were asked about — the backlog is pre-existing and unrelated churn makes a diff harder to review. Mention what you saw and leave it.
