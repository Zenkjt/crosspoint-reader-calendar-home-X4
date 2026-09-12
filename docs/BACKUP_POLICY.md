# X4newH — BACKUP POLICY

## 1. Principle
GitHub `X4newH` commits are the authoritative checkpoints. Local backups are secondary recovery artifacts.

## 2. Backup root
Broader project backup root:
```text
~/MirrorOS_Backup
```

## 3. Module-level naming
When one module changes:
```text
MMM-<ModuleName>_PASS_YYYY-MM-DD
```

For X4 firmware, keep the same principle: scope the backup to the changed module/file set where practical.

## 4. Full checkpoint naming
For multi-module checkpoints:
```text
SprintX_<Description>_PASS
```

Example:
```text
SprintX_RoundedRaff_Calendar_DoubleClick_PASS
```

## 5. Checkpoint contents
Record:
- project state;
- changed files;
- relevant Markdown;
- build/test notes;
- commit SHA;
- simulator result;
- outstanding issues.

## 6. ZIP rule
ZIPs are transport/backup artifacts, not source of truth.

If a ZIP conflicts with `X4newH` HEAD:
```text
X4newH HEAD wins.
```

## 7. Replacement candidates
Generated replacement files may be kept until validation/commit.

After commit, GitHub becomes authoritative and the old candidate must not be treated as current source.

## 8. Sprint close
At a meaningful PASS/Sprint boundary:
1. confirm build/simulator result;
2. confirm commit SHA;
3. update `CURRENT_STATE.md`;
4. update `DESIGN_BOOK.md` if architecture changed;
5. record deferred work.

## 9. Recovery
If current state is uncertain:
1. resolve `X4newH` HEAD;
2. use source at that HEAD;
3. read documentation;
4. ignore stale local packages unless historical comparison is explicitly requested.
