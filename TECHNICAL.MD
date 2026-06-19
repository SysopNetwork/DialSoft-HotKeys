# TECHNICAL.MD — DialSoft HotKeys

Technical reference for developers working with or extending HotKeys.

---

## Module Architecture

HotKeys is a standard MBBS DLL module. It exports a single `init__dialhot()` entry point that MBBS calls at startup. Inside `init__dialhot()` the module registers a `module block` struct with the BBS core, which wires up all remaining callbacks.

The primary interception mechanism is `btuchi()` — an MBBS API call that installs a per-character input filter (`chrprc`) on a user's channel. This filter runs before the BBS processes each keystroke, giving the module first access to every character the user types.

---

## Registered Callbacks

| Callback | Trigger | Purpose |
|---|---|---|
| `init__dialhot()` | BBS startup | Opens message file and database, registers module, allocates per-user state, sets defaults |
| `ulogon()` | User logon | Clears per-session state; optionally shows logon advertisement |
| `uinput()` | User selects this module | Handles CONFIG, S (system keys), P (personal keys) menu |
| `ulogof()` | User logoff or lost carrier | Removes character-processing hook; clears per-session state |
| `udeltd(uid)` | Account deleted | Removes user's personal hotkey record from Btrieve database |
| `shtdwn()` | BBS shutdown | Closes message file and Btrieve database |
| `glogbl()` | Any global command | Handles /HOTON, /HOTOFF, /HOTLIST, /HOTKEYS |
| `runtime()` | Timer (rtkick, every 1 s) | Enables/disables `chrprc` hook on each active channel |
| `chrprc(chan, c)` | Per-character input hook | Core hotkey interception — maps keystrokes to commands |

---

## Data Structures

### configrec (1,024 bytes)

System-wide configuration stored in the Btrieve NOMSG record (keyed on `userid="Sysop"`, `modnam="DIALHOT"`).

```
Offset  Size  Field
     0    30  userid          Btrieve key ("Sysop")
    30    25  modnam          Module name ("DIALHOT")
    55     2  reserved1       Reserved (formerly: installation date)
    57     2  reserved2       Reserved (formerly: last-run date)
    59     2  reserved3       Reserved (formerly: days running)
    61     8  reserved4       Reserved (formerly: activation code)
    69     2  flags           SHOWLGNMSG (0x0001) — show logon ad
    71    16  syskey          Access key for sysop config
    87    16  hotkey          Access key for hotkeys
   103    16  clrkey          Access key for color input
   119   315  desc[15][21]    System hotkey descriptions
   434   465  command[15][31] System hotkey commands
   899    16  pkey            Access key for personal hotkeys
   915    16  enablecmd       Enable command (/HOTON)
   931    16  disablecmd      Disable command (/HOTOFF)
   947    16  listcmd         List command (/HOTLIST)
   963    16  helpcmd         Help command (/HOTKEYS)
   979    45  spare           Reserved padding to 1,024 bytes
```

**Packing:** `configrec` must be packed to 1-byte alignment (`#pragma pack(push,1)`) to match the original Borland-compiled on-disk layout. `sizeof(configrec)` must equal 1,024. Do not remove the pack pragma.

### hotrec (1,000 bytes)

Per-user personal hotkey record stored in `DIALHOT.DAT`, keyed on `userid`.

```
Offset  Size  Field
     0    30  userid          Btrieve key (user account ID)
    30   610  command[10][61] Personal hotkey commands (10 slots)
   640   360  spare           Reserved padding to 1,000 bytes
```

Use `~` within a command string to separate multiple commands that execute in sequence.

### hotinf (per-session, in-memory only)

Per-channel runtime state, allocated as an array of `nterms` entries at startup.

```
Field    Size  Purpose
flags       1  CHRPRC (0x01) hooked, CHGFOR (0x02) color pending, CHGBCK (0x04), CANCLR (0x08), OUTPUT (0x10)
fcolor      1  Current foreground color index (1–16)
bcolor      1  Current background color index (1–8)
clrcnt      1  Number of pending color insertions in current input
lastinp     1  Previous character (ESC-sequence detection)
pcmd        1  Personal hotkey slot being triggered (0–9)
clrchg[30] 60  Backspace-undo stack for injected color sequences
```

---

## Hotkey Processing Flow

`chrprc(chan, c)` is called for every keystroke on a hooked channel:

1. **ESC-sequence filter** — If the previous character was ESC (27) and current is `[`, set `lastinp=255` and suppress. If `lastinp==255`, suppress and reset. This discards cursor-key and function-key sequences before they reach hotkey logic.
2. **High-ASCII pass-through** — Characters above 127 are passed through unchanged (return `c`).
3. **Personal hotkeys** — If the line buffer is empty (`!btuibw`) and the character is a Shift+number symbol (`!@#$%^&*()`), set `pcmd` and schedule `poll_personal()` via `begin_polling()`. The keystroke echoes normally.
4. **Printable characters** — If a color change is pending (`CHGFOR` / `CHGBCK`), inject the ANSI color sequence into the input buffer before returning the character.
5. **Control characters** — Mapped to actions:
   - Ctrl-A / ESC: abort current input
   - Ctrl-B: cycle background color
   - Ctrl-F / Tab: cycle foreground color
   - Backtick: reverse-cycle foreground color
   - Ctrl-H / DEL: backspace with color-undo
   - Ctrl-J: inject the disable command (`/HOTOFF`)
   - Ctrl-L: clear screen (`[0m[2J`)
   - Ctrl-R: redraw current input line
   - Ctrl-Z: inject the help command (`/HOTKEYS`)
   - Enter: clear color state, pass through
   - Ctrl-C through Ctrl-Y (excluding reserved): mapped to indices 0–14 in `config.command[]`. If a command is defined for that slot, inject it into the input buffer and return Enter.

---

## Hotkey Key Mapping

Control keys are mapped to `config.command[]` indices as follows:

| Key | Index | Key | Index | Key | Index |
|---|---|---|---|---|---|
| Ctrl-C | 0 | Ctrl-K | 4 | Ctrl-T | 9 |
| Ctrl-D | 1 | Ctrl-N | 5 | Ctrl-U | 10 |
| Ctrl-E | 2 | Ctrl-O | 6 | Ctrl-V | 11 |
| Ctrl-G | 3 | Ctrl-P | 7 | Ctrl-W | 12 |
| | | Ctrl-Q | 8 | Ctrl-X | 13 |
| | | | | Ctrl-Y | 14 |

Keys not in this table (Ctrl-A, Ctrl-B, Ctrl-F, Ctrl-H, Ctrl-I/Tab, Ctrl-J, Ctrl-L, Ctrl-R, Ctrl-S, Ctrl-Z, ESC, Enter) are reserved for built-in functions and cannot be assigned to system hotkeys.

---

## Files

| File | Location | Purpose |
|---|---|---|
| `HOT.C` | `SRC\` | Module implementation |
| `HOT.H` | `SRC\` | Structs, constants, static data |
| `DIALHOT.H` | `SRC\` | MSG message number constants |
| `DIALHOT.MSG` | `SRC\` | Message source file (human-readable) |
| `DIALHOT.MDF` | `SRC\` | Module description file |
| `DIALHOT.DAT` | `SRC\` | Empty Btrieve database template |
| `DIALHOT.VIR` | `SRC\` | Virtual information record |
| `DIALHOT.BCR` | `SRC\` | Btrieve database configuration reference |
| `DIALHOT.RC` | `SRC\` | Windows version resource |
| `DIALHOT_EXP.DEF` | `SRC\` | Module export definition file |
| `DIALHOT.vcxproj` | `SRC\` | Visual Studio project file |
| `DIALHOT.sln` | `/` | Visual Studio solution file |

---

## Message File Notes

`DIALHOT.MSG` is the human-readable message source. The BBS compiles it to `DIALHOT.MCV` at startup.

The message number constants in `DIALHOT.H` must remain in sync with the named entries in `DIALHOT.MSG`. The MBBS v10 MSG compiler assigns a number to every named entry including `LEVEL` section markers. `DIALHOT.MSG` contains a `LEVEL99` entry between `PKEYFTR` and `CNF12`; constants defined after that point (`CNF12`, `CNF21`, `CNF30`) are one higher than equivalent values compiled with the original Borland `wgsmsx` tool.

`DIALHOT.MSG` contains binary content: ESC bytes (0x1B) and CP437 box-drawing characters (0x80–0xFF). Edit it only with a binary-aware editor. Do not save it through a tool that re-encodes line endings or strips high-byte characters.

---

## Build Compatibility

The original module was compiled with Borland C++ using `ctdll`. The port to MSVC required three compatibility adjustments:

**1. Struct packing** — Borland's `ctdll` compiled with 1-byte struct alignment. MSVC defaults to 8-byte alignment. `configrec` in `HOT.H` is wrapped with `#pragma pack(push,1)` / `#pragma pack(pop)` to preserve the exact on-disk layout of the Btrieve NOMSG record.

**2. Type aliases** — `RETVAL` and `FINRET` are Borland-era types absent from the MBBS v10 SDK. `HOT.H` defines them as `typedef GBOOL RETVAL` and `typedef SHORT FINRET` so `HOT.C` compiles without modification.

**3. MSG constant offset** — `DIALHOT.H` constants `CNF12`, `CNF21`, and `CNF30` are set one higher than the original Borland-generated header to account for the MBBS v10 compiler counting `LEVEL` section markers as message numbers.

---

## Data Compatibility

The `DIALHOT.DAT` Btrieve database format and the `configrec` / `hotrec` on-disk layouts are unchanged from the original 1997 release. An existing database from a previous Worldgroup installation is compatible with this build provided the struct sizes remain 1,024 bytes (`configrec`) and 1,000 bytes (`hotrec`).
