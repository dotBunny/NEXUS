---
name: changelog
description: How to write CHANGELOG.md entries, and how to decide whether a change earns one at all. Invoke this skill whenever adding or editing changelog entries for any NEXUS plugin.
---

# Changelog

Entries go in `Plugins/NEXUS/CHANGELOG.md` — one file for the whole monorepo, not one per plugin. New work is added to the **topmost release section**, which is the unreleased one (its header ends in `- ?`).

## What earns an entry

A changelog records what changed **between releases**, for someone upgrading. It is not a development log.

The test for any entry: **could someone on the last shipped release have encountered this?**

- **Yes** → it belongs in the changelog.
- **No** → it does not, however much work it was.

The case this rule exists for is a bug **introduced and fixed inside the same unreleased version**. It never reached anyone, so a `### Fixed` entry for it is wrong twice over: it is noise, and it tells readers they were affected by something that never shipped. Fold whatever survives into the `### Added` or `### Changed` entry describing the feature it belongs to, or write nothing.

### Checking whether something shipped

Do not guess from memory of the session — most of a day's fixes are to code written that same day. Check the file's history against the date on the last released section header:

```bash
p4 filelog -s <file>                         # when each revision landed
p4 print -q "//depot/path/to/File.cpp#1" | grep "<the code>"
```

Code first added *after* the last release header's date is unreleased, and its bugs are unreleased with it.

## Sections

In order, and only the ones that apply: `### Added`, `### Changed`, `### Fixed`, `### Removed`. Some releases also carry `### Core Redirects`, `### DefaultGameplayTags Redirects` or `### Experimental`.

- **Changed vs Fixed** — a deliberate change in behavior is `Changed`; behavior that was *wrong* is `Fixed`. Renaming a setting is Changed; a setting that never applied is Fixed.
- **Removed** — deleted public types, functions and parameters. Say what replaced them.

## Writing an entry

Entries are prose, not shorthand. State what changed, and enough of why that a reader can tell whether it affects them.

- Type, function, setting and property names in backticks: ``` `UNOrganComponent` ```, ``` `Maximum Range` ```.
- American English (`behavior`, `color`), matching the `coding-style` skill.
- Say the consequence when there is one a reader must act on — results that will differ, data that must be recalculated, a default that changed.
- Link issues inline where one exists: `[#125](https://github.com/dotBunny/NEXUS/issues/125)`.

## Migration notes

When a release requires the user to *do* something — recalculate authored data, re-run a commandlet, re-bake — put a `>` blockquote directly under the release header, before the first section:

```markdown
## [0.3.5] - ?

> Recalculate any `UNCell` whose hull was built before this version, as ...
```

The same scope rule applies: describe only what someone upgrading from the last release must do. Steps that matter solely to a mid-development build belong in a commit message or a conversation, not here.

## At release time

- Replace the `- ?` on the header with the release date, `YYYY-MM-DD`.
- Add the comparison link at the bottom of the file, following the existing run:

```markdown
[0.3.5]: https://github.com/dotBunny/NEXUS/compare/v0.3.2...v0.3.5
```
