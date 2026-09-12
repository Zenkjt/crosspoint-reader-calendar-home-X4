# X4newH — WORKING RULES

**Repository:** `Zenkjt/crosspoint-reader-calendar-home-X4`  
**Branch:** `X4newH`

## 1. Source of truth
1. The committed `X4newH` branch is the implementation source of truth.
2. At every new session, verify the current `X4newH` HEAD SHA.
3. Read relevant files from that exact HEAD before preparing changes.
4. Old ZIPs, local extractions, historical commits, PR diffs, and old replacement packages are reference/history only.
5. Documentation describes architecture/workflow; live committed source controls actual implementation.

## 2. Inspect before editing
Before changing code:
1. Verify repository and branch.
2. Verify current HEAD.
3. Inspect relevant current files.
4. Compare source against documentation.
5. Report discrepancies explicitly.
6. Choose the smallest necessary change.
7. Only then prepare replacement files.

If the current source cannot be verified, stop rather than editing from a stale copy.

## 3. Replacement-file workflow
The user does not manually edit firmware source.

For code changes:
1. Assistant prepares the complete replacement file.
2. Assistant provides it for download.
3. The file keeps its repository-relative path.
4. User replaces/uploads it to GitHub.
5. User commits to `X4newH`.
6. User pulls/builds/runs the simulator on the Raspberry Pi 400.
7. User reports screenshot/log/result.
8. Next session starts from the new `X4newH` HEAD.

Do not ask the user to reconstruct code from snippets.

## 4. Scope discipline
- Make the smallest change that satisfies the task.
- Do not modify unrelated files.
- Do not casually redesign architecture.
- Do not introduce duplicate state/input frameworks.
- Do not scatter theme-specific conditionals through shared code.
- Preserve working behavior unless explicitly changed.

## 5. Architecture
`HomeActivity` owns Home state/navigation/prepared data.  
`HomeRenderContext` transports display data.  
`RoundedRaffTheme` owns RoundedRaff rendering.  
`CalendarService` owns calendar network/cache/data acquisition.

`RoundedRaffTheme` must not contain HTTP, SD-cache, GPIO, or business logic.

## 6. Power rule
One centralized `PowerButtonClickDetector` handles multi-clicks.

- single Power → existing Power behavior unchanged;
- double Power → `CalendarService::refresh()`;
- long Power → existing sleep/power behavior unchanged;
- Power + DOWN → existing screenshot behavior unchanged.

Do not create another detector in an Activity.

## 7. UI validation
For significant UI changes:
1. Build simulator.
2. Run simulator.
3. Inspect actual 480×800 output.
4. Check overlap/clipping/alignment.
5. Check menu count/selection.
6. Check other themes are not polluted.

Source coordinates alone are not visual validation.

## 8. Commit discipline
- Prefer small semantic commits.
- Review diff before committing.
- Do not silently rewrite history.
- Do not claim a GitHub commit/push succeeded without confirmation.
- User controls the commit/push step.

## 9. Communication
Always state:
- source/HEAD used;
- files changed;
- exact repository paths;
- intentionally untouched areas;
- whether a file is committed or only a candidate.
