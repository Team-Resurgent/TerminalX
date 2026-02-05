# TerminalX

A DOS-style command shell for the original Xbox. Run it from your dashboard or recovery environment for file management, text editing, and system control.

---

## Shell features

- **Path completion (Tab)** — Complete file and folder names as you type. Press Tab to complete the current word; if the path has a folder (e.g. `cerbios\cer`), completion keeps the path and completes the name (e.g. `cerbios\cerbios.ini`). Multiple matches complete to the common prefix.
- **Command history** — Use **Up** and **Down** arrows to recall previous commands (last 50).
- **Scrollback** — Use **Page Up** and **Page Down** to scroll through long output (e.g. from `DIR` or `TYPE`); new input scrolls back to the bottom.
- **In-line editing** — Use **Left** and **Right** arrows and **Backspace** to edit the current line before pressing Enter.
- **Colors** — Use the `COLOR` command to change text and background (e.g. `COLOR 0A` for green on black).

---

## Drive names

Use these names with the colon when switching drives (e.g. `HDD0-E:`) or in paths (e.g. `DIR HDD0-E:\cerbios`).

| Format | Example | Description |
|--------|---------|-------------|
| **HDD0-**letter | `HDD0-E:`, `HDD0-F:`, `HDD0-X:` | First hard disk partitions. Letters: **C**, **E**, **F**, **G**, **H**, **I**, **J**, **K**, **L**, **M**, **N**, **X**, **Y**, **Z**. |
| **HDD1-**letter | `HDD1-E:`, `HDD1-F:` | Second hard disk (same letter set as HDD0). |
| **MMU0**–**MMU7** | `MMU0:`, `MMU3:` | Memory units (MU) in ports 0–7. |
| **H**–**O** (single letter) | `H:`, `I:` | Short form for memory units: **H** = MMU0, **I** = MMU1, … **O** = MMU7. |

Drive names are case-insensitive (`hdd0-e:` works the same as `HDD0-E:`).

---

## Commands (with examples)

### Navigation and drives

| Command | Example | Description |
|--------|---------|-------------|
| **DRIVE:** | `HDD0-E:` | Switch to a drive. Use supported names (see [Drive names](#drive-names)), e.g. `HDD0-E:`, `MMU0:`. |
| **CD** | `CD cerbios` | Change directory. `CD` with no args shows current directory. |
| **DIR** | `DIR` / `DIR cerbios\` / `DIR /W` | List files and folders. Path is optional. |
| | `DIR /W` | Wide list format. |
| | `DIR /O:N` | Sort by name (N=name, D=date, S=size, E=extension; prefix `-` for reverse). |
| | `DIR /A:D` | Show only directories; `/A:-H` hides hidden. |
| | `DIR /P` | Pause every 23 lines. |

Paths can be relative to the current directory or use a drive: `DIR HDD0-E:\cerbios`.

---

### Files and folders

| Command | Example | Description |
|--------|---------|-------------|
| **MD** / **MKDIR** | `MD myfolder` | Create a directory. |
| **RD** / **RMDIR** | `RD myfolder` | Remove an empty directory. |
| **COPY** | `COPY config.ini config.bak` | Copy one or more files to a destination. |
| **MOVE** | `MOVE old.txt new.txt` | Move or rename files/directories. |
| **DEL** / **ERASE** | `DEL file.txt` | Delete file(s). Supports `/S` (tree), `/F` (force), `/A` (attributes). |
| **TYPE** | `TYPE cerbios\cerbios.ini` | Display contents of text file(s) (up to 64 KB per file). |
| **EDIT** | `EDIT cerbios\cerbios.ini` | Full-screen text editor. **F2** = Save, **F3** = Exit. Creates the file if it doesn’t exist. Long lines scroll horizontally. |

---

### System and display

| Command | Example | Description |
|--------|---------|-------------|
| **DATE** | `DATE` | Show or set the date (e.g. `DATE 2025-02-03`). Press Enter at the prompt to keep current. |
| **TIME** | `TIME` | Show or set the time. Press Enter at the prompt to keep current. |
| **VER** | `VER` | Show version: *Microsoft Xbox Original [TerminalX]*. |
| **COLOR** | `COLOR 0A` | Set attribute (2 hex digits: background, foreground). `COLOR` with no args = default (0A). |
| **CLS** | `CLS` | Clear the screen. |
| **SHUTDOWN** | `SHUTDOWN` / `SHUTDOWN /S` | Power off (default). |
| | `SHUTDOWN /R` | Reboot (full power cycle). |
| | `SHUTDOWN /W` | Warm reboot (reset). |
| **EXIT** | `EXIT` | Quit the shell. |

---

### Other

| Command | Example | Description |
|--------|---------|-------------|
| **ECHO** | `ECHO Hello` | Print a message. |
| | `ECHO ON` / `ECHO OFF` | Turn command echoing on or off. |
| **HELP** | `HELP` | List commands. `HELP command-name` for command help. |

---

## Path completion examples

- `dir cer` + **Tab** → `dir cerbios\` (single folder match).
- `dir cerbios\cer` + **Tab** → `dir cerbios\cerbios.ini` (completion keeps path).
- `cd HDD0-E\c` + **Tab** → completes to matching folder/file on that drive.
- `type myfile.tx` + **Tab** → completes to `myfile.txt` if unique.

---

## Editor (EDIT) keys

- **Arrows** — Move cursor.
- **Home / End** — Start/end of line.
- **Backspace / Delete** — Delete character before/under cursor.
- **Enter** — New line.
- **F2** — Save and stay in editor.
- **F3** — Exit (saves if modified).
- Status line shows row/column and hints.

---

## Requirements

- Original Xbox (or compatible).
- Built with the Xbox SDK (VC++ 7.x / Xbox project).
- Typical use: Boot to BIOS of choice and launch, the terminal will default to `HDD0-E:`.

---

## Building

Open `TerminalX.sln` in Visual Studio (with Xbox SDK), select the Xbox configuration, and build. Deploy the resulting XBE to your Xbox as required by your dashboard or recovery setup.
