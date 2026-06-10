# CHANGELOG.MD

All notable changes to DialSoft HotKeys are documented in this file.

---

## [2.7] - Unreleased

### Added
- Windows version resource (`DIALHOT.RC`): DLL now carries embedded file properties — product name "DialSoft HotKeys", version 2.7.0.0, company "Sysop Network", copyright "Copyright © Sysop Network".

### Fixed
- ANSI display in DIALHOT.MSG: reordered 18 locations where a CP437 high-byte character (box-drawing, 0x80–0xFF range) immediately preceded an ESC byte. That ordering caused some terminal paths to render the ESC as a stray `?` and display the following ANSI code as literal text. Placing the ANSI sequence before the adjacent box-drawing character resolves it without affecting the visual output.
- Color picker (TAB key): added explicit `\x1b` prefix to all entries in `fcolors[]` and `bcolors[]` in HOT.H. HOT.H was re-created from scratch during the VS migration; the original Borland source embedded raw ESC bytes (invisible to file readers) that were not carried forward. These strings are output via `chious()` directly to the terminal and require the `\x1b` escape in the C string literal.
- Config screen blank on entry: fixed incorrect ordering in `coptions()` — `fsdscb` was accessed to set `FFFAVD` on the ACT field before `fsdbkg(fsdrft())` had been called. `fsdbkg` is what initializes the `fsdscb` global pointer; accessing it before that call is undefined behavior and produced a blank config screen. The FFFAVD assignment now follows `fsdbkg`, matching the original source order.
- Screen routing wrong (S shows config, P shows S, config blank): incremented `CNF12`, `CNF21`, and `CNF30` in DIALHOT.H from 24/25/26 to 25/26/27. The MBBS v10 MSG compiler assigns a message number to `LEVEL99` (the mid-file section marker), whereas the original Borland `wgsmsx` compiler skipped it. The one-position offset caused each FSD screen to load the previous screen's background template.
- Config/system-keys data corruption: added `#pragma pack(push,1)` / `#pragma pack(pop)` around `configrec` in HOT.H. The `modnam[25]` field leaves the struct at byte offset 55 (odd); without explicit 1-byte packing the MSVC compiler inserts a pad byte before the following `USHORT`, shifting every subsequent field by one byte and misreading all config data from the Btrieve NOMSG record. The original Borland `ctdll` build used tight (1-byte) packing, so `sizeof(configrec)` must equal 1024 to match the on-disk layout.

### Changed
- Migrated build system from Borland C++ (DIALHOT.MAK) to Microsoft Visual Studio 2022 (DIALHOT.sln / DIALHOT.vcxproj).
- Replaced Borland-specific build tools (ctdll, ltdll, wgsmsx) with standard MSVC toolchain.
- Replaced legacy Borland header includes in HOT.H with MBBS v10 SDK headers (gcomm.h, majorbbs.h, fsdbbs.h).
- Replaced auto-generated DIALHOT.H (formerly produced by wgsmsx) with manually maintained message constants header.
- Added type compatibility shims in HOT.H: `RETVAL` → `GBOOL` and `FINRET` → `SHORT` (Borland-era types absent from v10 SDK).
- Added `#undef REGISTERED` in HOT.H after system includes to resolve conflict with Windows SDK nb30.h.
- Updated DIALHOT.MDF developer field to Sysop Network.
- Added DIALHOT.BCR Btrieve database configuration file for v10 compatibility.
- Updated project directory structure: source under SRC\, distribution files under DIST\, documentation under DOCS\.
- Added BUILDING.MD, CHANGELOG.MD, and .gitignore.
- Verified: project builds cleanly with zero warnings and zero errors under MSVC v143 (VS 2022), Win32 target.

### Open-Source Transition
- Removed commercial licensing system: activation code validation, 14-day demo countdown, expiration checks.
- Removed `actcode()` and `cvfy()` functions.
- Removed `DialHot` and `badacts` variables.
- Removed `REGISTERED` and `DEMOMODE` flag definitions.
- Removed `#undef REGISTERED` workaround (no longer needed).
- Simplified `init__dialhot()`: config is now loaded unconditionally; defaults are written on first run.
- Activation code field in CNF12 is now read-only (FFFAVD) and always blank; activation code validation removed.
- Preserved `configrec` struct layout for backward data-file compatibility; formerly licensing fields marked reserved.
- Demo/registration messages (NOTREG, ISDEMO, REGNOW) are never displayed; all call sites removed.
- Updated version string from "DialSoft HotKeys 2.6" to "HotKeys 2.7".
- Updated DIALHOT.MDF developer field to Sysop Network.
- Added MIT LICENSE file (Copyright © Sysop Network).
- Populated DIST\2.7\ with release artifacts: DIALHOT.DLL, DIALHOT.MDF, DIALHOT.MSG, DIALHOT.BCR.

### Preserved
- All source logic in HOT.C is unchanged from the original v2.6 release.
- Data structure layouts (configrec, hotrec, hotinf) are unchanged.
- Btrieve database format (DIALHOT.DAT) is unchanged.
- Message file content (DIALHOT.MSG) is unchanged.
- Module interface block structure and callback functions are unchanged.

---

## [2.6] - 1997-07-10

Original commercial release by Richard Iglar / DialSoft.

- Support for 15 system-defined global hotkeys (CTRL+key).
- Support for 10 user-defined personal hotkeys (SHIFT+1 through SHIFT+0).
- ANSI foreground and background color control via TAB / CTRL-F / CTRL-B.
- CTRL-Z help, CTRL-J disable, CTRL-A/ESC abort, CTRL-L clear screen, CTRL-R redraw.
- NO MSG design: all configuration is stored online, no MSG recompilation required.
- Commercial licensing with 14-day demo mode.
- Compatible with MajorBBS 6.25 and Worldgroup 1.x through 3.x.
