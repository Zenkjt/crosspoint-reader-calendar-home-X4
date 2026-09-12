# X4newH — DESIGN BOOK / ARCHITECTURE

**Repository:** `Zenkjt/crosspoint-reader-calendar-home-X4`  
**Branch:** `X4newH`

## 1. Core separation
```text
HomeActivity
    ↓
HomeRenderContext
    ↓
Theme renderer
```

`HomeActivity` = state/navigation/prepared data.  
`HomeRenderContext` = display-data transport.  
`RoundedRaffTheme` = visual renderer.

## 2. Calendar service
```text
GitHub Action
    ↓
today.json
    ↓ HTTPS
CalendarService
    ├── validate
    ├── cache valid result
    └── fallback to last valid cache
    ↓
CalendarData
    ↓
HomeActivity
    ↓
HomeRenderContext
    ↓
RoundedRaffTheme
```

Timezone: `Asia/Ho_Chi_Minh`.

Owner data travels in the same JSON payload and can change remotely without reflashing firmware.

No large calendar database, NTP lunar engine, or remote-calendar business logic belongs in the firmware UI layer.

## 3. Offline policy
```text
HTTPS fetch
   ↓
valid? ── yes → use + cache
   │
   no
   ↓
last valid cache
   ↓
if none → safe defaults
```

Network operations must have bounded timeouts.

## 4. Power architecture
```text
HalGPIO
   ↓
MappedInputManager
   ↓
PowerButtonClickDetector
   ├── single → existing Power behavior
   ├── double → CalendarService::refresh()
   └── long → existing sleep/power behavior
```

Current detector window: 400 ms; maximum finalized clicks: 3.

One detector only. No Activity-local Power timers.

X4 Pro frontlight double-click behavior is separate and must not be copied into X4.

## 5. Wi-Fi ownership
If Wi-Fi is already connected:
- reuse it;
- do not shut it down.

If Wi-Fi is OFF/disconnected and CalendarService starts it:
- connect;
- refresh;
- disconnect;
- turn Wi-Fi OFF.

This keeps manual calendar refresh from changing the device's normal sleep/network lifecycle.

## 6. RoundedRaff geometry
```text
10..310    Calendar       h=300
320..680   Reading card   h=360
690..760   Bottom menu    h=70
760..800   Button hints   h=40
```

Reading card:
- y=320;
- h=360;
- divider x=230;
- two-column ratio locked.

## 7. Reading Card design
Only headings are centered:
```text
ĐANG ĐỌC             THÔNG TIN
```

Cover is centered in the left-column cover area and sits inside a light-gray padded area.

All book/owner information remains left-aligned.

Owner typography is subordinate, compact and regular weight.

## 8. Menu design
The Home controller creates the dynamic menu.

Expected order:
```text
Đọc tiếp
Duyệt file
Sách gần đây
[OPDS]
Truyền file
Cài đặt
```

OPDS is inserted only when available.

The renderer consumes existing labels/icons; it does not hardcode actions.

Current visual direction: icon + label.

## 9. Theme isolation
```text
Lyra / Classic / Lyra Extended
    → existing Home behavior

RoundedRaff
    → custom calendar + reading card + horizontal menu
```

If a RoundedRaff change affects another theme, stop and repair the architecture before continuing.

## 10. Calendar day bitmap
The solar day is the focal calendar element.

Use the supplied bitmap digits, not scaled ordinary text.

For two digits, center the combined group as one unit.

## 11. Input
Reuse the existing infrastructure:
- UP/LEFT = previous;
- DOWN/RIGHT = next;
- circular wrap;
- Confirm = activate;
- touch and hardware share selector state.

No second navigation framework.

## 12. Back
From Home:
`Back → most recently read book`.

## 13. Rejected architecture: Calendar Editor
The earlier on-device Calendar Editor proposal is rejected.

Do not implement:
- Calendar Selector;
- Virtual Calendar button;
- Calendar Editor activity;
- 6×6 day selector;
- weekday/lunar selectors.

Reason: production calendar data comes from the GitHub-generated `today.json`.

## 14. Accepted architecture: explicit Power refresh
Double-click Power is an explicit manual calendar-refresh action.

It is not tied to:
- deep-sleep entry;
- wake;
- boot;
- power-off preparation.

## 15. Architectural invariant
Services acquire data. Activities own state/navigation. Themes render.

That separation should remain the default design principle for future changes.
