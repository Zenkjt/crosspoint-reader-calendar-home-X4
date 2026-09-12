# X4newH — CURRENT STATE

**Last documentation update:** 2026-09-12  
**Repository:** `Zenkjt/crosspoint-reader-calendar-home-X4`  
**Branch:** `X4newH`  
**Verified branch HEAD:** `eedab26b3827cc2bc94be413b7afd04d02670f24`  
**Latest commit:** `Add files via upload`

> The live `X4newH` branch remains authoritative for source code.

## 1. Target
- Xteink X4
- ESP32-C3
- Home: logical 480×800 portrait
- Development/simulator: Raspberry Pi 400
- Workflow: GitHub/Web + GitHub Actions + simulator

## 2. Home geometry
```text
10..310    Calendar       h=300
320..680   Reading card   h=360
690..760   Bottom menu    h=70
760..800   Button hints   h=40
```

Reading card:
- two columns;
- divider `x=230`;
- ratio is locked.

## 3. Latest RoundedRaff UI direction

### Calendar
```text
today.json
  ↓
CalendarService
  ↓
CalendarData/cache
  ↓
HomeActivity
  ↓
HomeRenderContext
  ↓
RoundedRaffTheme
```

Timezone: `Asia/Ho_Chi_Minh`.

The large solar day uses the supplied bitmap digits. The large digit has already rendered correctly in a physical X4 test.

### Reading card
Keep divider `x=230` and the existing two-column ratio.

Only headings are centered:
- `ĐANG ĐỌC` centered in left column;
- `THÔNG TIN` centered in right column.

Cover:
- horizontally centered in left column;
- vertically centered in a dedicated cover area;
- light-gray surrounding area with visible padding.

Everything else stays left-aligned:
- title;
- author;
- progress text/bar/percentage/page count;
- owner;
- dividers.

### Bottom menu
Latest accepted visual direction is **icon + label**.

Dynamic order:
1. Đọc tiếp — Book
2. Duyệt file — Folder
3. Sách gần đây — Recent
4. OPDS — Library, only when available
5. Truyền file — Transfer
6. Cài đặt — Settings

Renderer must use the existing dynamic `menuLabel()` / `rowIcon()` data and must not hardcode menu contents.

## 4. Calendar service
`CalendarService` owns HTTPS fetch, JSON validation, cache and current `CalendarData`.

Endpoint:
`https://raw.githubusercontent.com/Zenkjt/x4-calendar-tool/main/today.json`

On fetch failure, retain last valid data.

## 5. Power double-click
Locked path:
```text
Power
  ↓
MappedInputManager
  ↓
PowerButtonClickDetector
  ├── 1 click → existing Power behavior
  ├── 2 clicks → CalendarService::refresh()
  └── long → existing sleep/power path
```

Current detector:
- timeout = 400 ms;
- max clicks = 3;
- release-based centralized detection.

Wake-release and screenshot-combo guards reset the detector.

`main.cpp` already contains the X4 double-click refresh action.

## 6. Wi-Fi lifecycle candidate
A replacement `CalendarService.cpp` has been prepared to:
- reuse an already-connected Wi-Fi session;
- otherwise use the saved SSID/credential;
- connect temporarily;
- wait up to 6 seconds;
- fetch/validate/save calendar data;
- turn Wi-Fi OFF again if this refresh started it;
- leave pre-existing Wi-Fi connections alone.

## 7. Uncommitted candidates
UI candidate:
`src/components/themes/RoundedRaffTheme.cpp`
Local artifact:
`RoundedRaffTheme_X4newH_FINAL_UI.cpp`

Calendar/Wi-Fi candidate:
`src/services/calendar/CalendarService.cpp`
Local artifact:
`CalendarService_X4newH_DoubleClickWiFiSession.cpp`

Both are **candidates only until the user validates and commits them**.

## 8. Already validated
- GitHub calendar generator/workflow works.
- `today.json` integration exists.
- Large bitmap calendar digit renders correctly on physical X4.
- Power double-click architecture has been audited against single click, long press, wake, screenshot, and X4 Pro frontlight behavior.

## 9. Must not regress
- X3 support.
- X4 sleep/wake behavior.
- screenshot Power + DOWN.
- existing single Power semantics.
- Reading Card column ratio.
- theme isolation.
- centralized click detector.
- service/theme separation.

## 10. Immediate next work
1. Validate/commit latest RoundedRaff UI replacement.
2. Validate/commit CalendarService Wi-Fi-session replacement.
3. Build/run simulator.
4. Physically test Power double-click.
5. Confirm Wi-Fi returns to OFF when the refresh itself started it.
