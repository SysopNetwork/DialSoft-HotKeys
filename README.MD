# DialSoft HotKeys

**HotKeys** is a keyboard shortcut module for [The Major BBS v10](https://github.com/TheMajorBBS/MBBS-v10-module-SDK). It intercepts user keystrokes system-wide and maps them to BBS commands, with no message file recompilation required for configuration changes.

Originally written by Richard Iglar / DialSoft in 1997 for MajorBBS 6.x and Worldgroup 1.x–3.x. Ported to The Major BBS v10 and released as open source by [Sysop Network](https://sysopnetwork.com).

---

## Features

- **System hotkeys** — Up to 15 sysop-defined shortcuts mapped to Ctrl+key combinations (Ctrl-C through Ctrl-Y, excluding reserved keys).
- **Personal hotkeys** — Each user can define up to 10 personal shortcuts triggered by Shift+number (!, @, #, $, %, ^, &, *, (, )).
- **Color input** — Users with ANSI enabled can cycle foreground color (Tab / Ctrl-F) and background color (Ctrl-B) while typing.
- **Built-in commands** — Ctrl-Z (help), Ctrl-L (clear screen), Ctrl-R (redraw input), Ctrl-A / Esc (abort input).
- **NO MSG design** — All configuration is stored online in the Btrieve NOMSG record. No MSG recompilation is needed to change settings.
- **Global commands** — Configurable command names for enable, disable, list, and help (defaults: /HOTON, /HOTOFF, /HOTLIST, /HOTKEYS).
- **Key access control** — Separate access keys control who can use hotkeys, personal hotkeys, and color input.
- **Sysop-hidden hotkeys** — Prefix a hotkey description with `@` to hide it from regular users; sysop-key holders see it in the help display.

---

## Requirements

- The Major BBS v10
- Windows (32-bit or 64-bit) — MBBS v10 is a 32-bit server process and runs on 64-bit Windows via WOW64; the module DLL is compiled Win32 and works on both

---

## Installation

1. Copy the files from `DIST\2.7\` to your MBBS modules directory:

   | File | Purpose |
   |---|---|
   | `DIALHOT.DLL` | Module binary |
   | `DIALHOT.MDF` | Module description file |
   | `DIALHOT.MSG` | Message source file |
   | `DIALHOT.DAT` | Btrieve database (empty, initialized on first run) |
   | `DIALHOT.VIR` | Virtual information record |

2. Register the module in your MBBS configuration.

3. Start (or restart) the BBS. HotKeys will initialize and write default settings to the NOMSG record on first run.

---

## Configuration

Configuration is done online — no downtime required.

### Online config screen

Type `CONFIG` from the HotKeys module prompt (sysop access required) to open the full-screen configuration panel. From there you can set:

- **LGN** — Show a hotkey advertisement message at logon (YES / NO)
- **SYS** — Access key required to access the sysop config screen
- **HOT** — Access key required to use hotkeys
- **PKY** — Access key required to use personal hotkeys
- **CLR** — Access key required to use color input
- **HLP** — Command name for the help screen (default: `/HOTKEYS`)
- **ENBL** — Command to enable hotkeys (default: `/HOTON`)
- **DSBL** — Command to disable hotkeys (default: `/HOTOFF`)
- **LST** — Command to list personal hotkeys (default: `/HOTLIST`)

### System hotkeys

From the HotKeys module prompt, type `S` (sysop access required) to open the system-key editor. Each of the 15 slots maps a Ctrl+key to a description and a BBS command.

Prefix a description with `@` to make it visible only to users with sysop-level access (hidden from the public help listing).

### Personal hotkeys

Users type `P` from the HotKeys module prompt to open their personal hotkey editor (access controlled by the PKY key). Each slot maps a Shift+number key to any BBS command sequence. Use `~` as a separator to chain multiple commands.

---

## Building from Source

See [BUILDING.MD](BUILDING.MD) for full build instructions.

**Summary:** Open `DIALHOT.sln` in Visual Studio 2022, select **Release | Win32**, and build. The [MBBS v10 SDK](https://github.com/TheMajorBBS/MBBS-v10-module-SDK) must be installed and the `MBBSSDKDIR` environment variable set to its path.

---

## Tips / Donations

- **Cash App:** [$laudenbachm](https://cash.app/$laudenbachm)
- **Ko-fi:** [ko-fi.com/laudenbachm](https://ko-fi.com/laudenbachm)

---

## License

MIT License — see [LICENSE](LICENSE) for the full text.

Copyright © Sysop Network

Original source code copyright © Richard Iglar / DialSoft, 1997.
