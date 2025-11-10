# Scripts Directory

This directory contains utility scripts for VST Runner development.

## Files

### `pre-commit`
Git pre-commit hook that runs code formatting checks before commits.

**Installation:**
```bash
# Automatic (via setup.sh)
./setup.sh

# Manual
cp scripts/pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

**What it does:**
- Checks formatting of staged C++ files with clang-format
- Prevents commits with formatting issues
- Can be bypassed with `git commit --no-verify` (not recommended)

## Adding New Scripts

When adding scripts to this directory:

1. Add a clear description in this README
2. Make scripts executable: `chmod +x scripts/your-script`
3. Use `#!/bin/bash` or `#!/usr/bin/env python3` shebang
4. Add error handling with `set -e` for bash scripts
5. Provide usage documentation in script comments
6. Test on multiple platforms if applicable

## Future Scripts (Ideas)

- `check-deps.sh` - Verify all system dependencies
- `generate-docs.sh` - Build documentation
- `release.sh` - Automated release process
- `update-copyright.sh` - Update copyright headers
