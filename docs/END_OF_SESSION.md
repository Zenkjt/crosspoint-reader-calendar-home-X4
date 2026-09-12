# X4newH — END OF SESSION

## 1. Source checkpoint
Record:
- repository;
- branch;
- current HEAD SHA;
- last commit;
- changed files;
- intentionally untouched files.

## 2. Test checkpoint
Record:
- simulator build;
- simulator run;
- screenshot/visual result;
- physical X4 result if applicable;
- warnings/errors.

## 3. Documentation
Update as needed:
- `CURRENT_STATE.md`;
- `DESIGN_BOOK.md`;
- `WORKING_RULES.md`;
- `BACKUP_POLICY.md`;
- `GITHUB_WORKFLOW.md`.

## 4. Candidate files
For every uncommitted replacement:
```text
Repository path:
Local artifact:
Purpose:
Status: candidate / validated / committed
```

Never leave commit status ambiguous.

## 5. Work status
Use:
```text
DONE
IN PROGRESS
NEXT
DEFERRED
REJECTED
```

## 6. Architecture check
Verify:
- RoundedRaff rendering remains isolated;
- no duplicate input framework;
- no HTTP/cache/GPIO/business logic in the theme;
- Power single/double/long remain separated;
- X4 Pro-specific logic was not applied to X4;
- reader/sleep behavior was not unintentionally changed.

## 7. Commit checkpoint
If committed:
```text
GitHub X4newH HEAD = authoritative
```

If not:
```text
UNCOMMITTED
```

must be stated clearly.

## 8. Required repository documentation
```text
docs/
├── WORKING_RULES.md
├── CURRENT_STATE.md
├── DESIGN_BOOK.md
├── BACKUP_POLICY.md
├── GITHUB_WORKFLOW.md
└── END_OF_SESSION.md
```

## 9. Next-session opening
1. Verify `X4newH` HEAD.
2. Read `CURRENT_STATE.md`.
3. Read `WORKING_RULES.md`.
4. Read `DESIGN_BOOK.md`.
5. Inspect relevant source at that HEAD.
6. Compare documentation vs source.
7. Report discrepancies before editing.

## 10. Final handoff questions
```text
WHERE ARE WE?
WHAT IS COMMITTED?
WHAT IS NOT COMMITTED?
WHAT WORKS?
WHAT FAILED?
WHAT IS NEXT?
WHAT MUST NOT BE TOUCHED?
```
