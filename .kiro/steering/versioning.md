# Versioning

Git tags follow semantic versioning without a `v` prefix.

- Format: `MAJOR.MINOR.PATCH` (e.g. `0.0.2`, `1.0.0`, `2.1.3`)
- Do NOT prefix with `v` (use `0.0.3`, not `v0.0.3`)
- Increment PATCH for bugfixes and small changes
- Increment MINOR for new features that don't break saves or the Lua API
- Increment MAJOR for breaking changes (save format, removed Lua globals, restructured components)
