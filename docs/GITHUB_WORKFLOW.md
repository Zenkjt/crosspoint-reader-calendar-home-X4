# X4newH — GITHUB WORKFLOW

**Repository:** `Zenkjt/crosspoint-reader-calendar-home-X4`  
**Branch:** `X4newH`

## 1. Development loop
```text
verify X4newH HEAD
    ↓
inspect current source
    ↓
prepare complete replacement file(s)
    ↓
user uploads/replaces on GitHub
    ↓
user commits
    ↓
pull on Raspberry Pi 400
    ↓
build simulator
    ↓
run simulator
    ↓
inspect result
    ↓
next change
```

## 2. Before editing
Verify:
- repository;
- branch = X4newH;
- HEAD SHA;
- relevant source files;
- recent commits.

## 3. Replacement-file convention
Every code replacement should include:
- exact repository-relative path;
- complete file;
- what changed;
- what was deliberately untouched;
- suggested commit message.

No manual reconstruction.

## 4. User commit responsibility
Assistant prepares; user replaces/commits.

Never claim a commit/push succeeded unless GitHub confirms it.

## 5. Simulator
Development machine:
- Raspberry Pi 400;
- Debian/Raspberry Pi OS Trixie;
- aarch64;
- PlatformIO;
- SDL2.

Typical:
```bash
cd ~/crosspoint-reader-calendar-home-X4-X4newH
source ~/pio-venv/bin/activate
pio run -e simulator
pio run -e simulator -t run_simulator
```

Sync:
```bash
git fetch origin
git reset --hard origin/X4newH
git rev-parse --short HEAD
```

Do not use destructive cleanup unless explicitly requested.

## 6. Simulator compatibility
`platformio.ini` uses:
```text
extra_configs = platformio.local.ini
```

The local simulator config contains:
```text
-Dmemcpy_P=memcpy
```

Do not remove it unless revalidated.

## 7. Validation order
1. build;
2. run;
3. inspect 480×800 screen;
4. review diff;
5. confirm no unrelated files changed;
6. physical X4 test when relevant;
7. commit.

## 8. Power validation
Always test:
```text
single Power
double Power
long Power
Power + DOWN
wake + Power release
```

Expected:
```text
single → existing behavior
double → CalendarService refresh
long → sleep/power
Power + DOWN → screenshot
wake release → no false click
```

## 9. Release policy
Release workflows are tag-driven.

Do not create a release tag merely for ordinary source iteration. Prefer simulator validation first.

## 10. History
- no silent history rewrites;
- prefer semantic commits;
- no force-push unless explicitly requested;
- no branch deletion unless explicitly requested.
