# Branching

All development work must happen on feature branches — never commit directly to `main`.

- **Before the first commit of any task execution**, run `git branch --show-current`. If on `main`, create and switch to a feature branch immediately (`git checkout -b <branch-name>`) before writing any code.
- Branch naming: use kebab-case matching the spec name (e.g. `equipment-system`, `hit-chance-system`, `enemy-equipment-loot`)
- One branch per feature/spec. All commits for that feature go on its branch.
- Push to the feature branch, not main.
- `main` is updated only via pull request merges.
