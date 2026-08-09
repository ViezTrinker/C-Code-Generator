# C Code Generator

Visual flowchart editor (SFML) that generates, compiles, and runs C99 code.

## Requirements

- CMake 3.20+
- A C++17 compiler (Visual Studio 2022/2026, or MinGW g++)
- Network on the **first** configure/build (to download SFML unless already cached)
- `gcc` on `PATH` if you use the in-app **Build/Run** buttons

External libraries are handled for you:

| Dependency | How it is provided |
| --- | --- |
| nlohmann_json | Vendored in `third_party/nlohmann/json.hpp` |
| SFML 3 | Auto-downloaded into `third_party/sfml` by `scripts/build.ps1`, or via CMake FetchContent |

No vcpkg / manual SFML install is required.

## Easiest build (Windows)

From the repo root in PowerShell:

```powershell
.\scripts\build.ps1
```

That script downloads SFML once, configures CMake, and builds. Optional:

```powershell
.\scripts\build.ps1 -Configuration Debug
.\scripts\build.ps1 -Generator "Visual Studio 17 2022"
```

## Visual Studio

1. **File → Open → Folder…** and select this repository
2. Pick **x64-Debug** or **x64-Release**
3. Build (first configure downloads SFML if needed)

`CMakeSettings.json` / `CMakePresets.json` do **not** need a vcpkg toolchain file.

## Manual CMake

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```powershell
.\build\c_code_generator.exe
.\build\c_code_generator.exe --self-test
.\build\c_code_generator.exe --codegen examples\add_two_integers.cgen
.\build\c_code_generator.exe --codegen examples\add_two_integers.cgen --compile
.\build\c_code_generator.exe --codegen examples\add_two_integers.cgen --run
```

`--codegen` writes C into `build_out/` only. Add `--compile` to invoke gcc, or `--run` to compile and execute (stdin is only needed with `--run`). Validation errors abort the write; warnings print to stderr.

(MSVC multi-config generators may place the exe under `build\Release\`.)

## Unit tests

GoogleTest is included as a git submodule at `third_party/googletest` (pinned to **v1.15.2**).

```powershell
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target cgen_unit_tests
ctest --test-dir build --output-on-failure
```

Disable tests with `-DCGEN_BUILD_TESTS=OFF` if needed.

## Example projects

### Feature primers (new IR)

Short graphs that teach one feature each. Regenerate with `python scripts\generate_feature_examples.py`.

| File | Shows | Expected output |
| --- | --- | --- |
| [`examples/ex_struct_literal.cgen`](examples/ex_struct_literal.cgen) | **Struct Literal** into a typed Decl | `point = (3, 7)` |
| [`examples/ex_multi_arg_call.cgen`](examples/ex_multi_arg_call.cgen) | **Call** with `Arg0`–`Arg2` | `add3(10, 20, 12) = 42` |
| [`examples/ex_address_of.cgen`](examples/ex_address_of.cgen) | **Address Of** + `Hero*` | `hero.hp after bump = 11` |
| [`examples/ex_malloc_free.cgen`](examples/ex_malloc_free.cgen) | **Malloc** / **Free** + `uint8_t*` | `pBuf[0] = 42` |

```powershell
.\build\c_code_generator.exe --codegen examples\ex_struct_literal.cgen --run
.\build\c_code_generator.exe --codegen examples\ex_multi_arg_call.cgen --run
.\build\c_code_generator.exe --codegen examples\ex_address_of.cgen --run
.\build\c_code_generator.exe --codegen examples\ex_malloc_free.cgen --run
```

### Add two integers

Open [`examples/add_two_integers.cgen`](examples/add_two_integers.cgen) from the app (**Open**), or run it headless with `--codegen` as above.

It declares `a = 7` and `b = 11`, computes `sum = a + b`, and prints:

```text
a=7
b=11
sum=18
```

### Live clock

[`examples/live_clock.cgen`](examples/live_clock.cgen) declares year/month/day/hour/minute/second, fills them with a **Local Time** block each loop iteration, prints `YYYY-MM-DD HH:MM:SS`, then **Sleep**s 1 second in an infinite `while (1)` loop.

```powershell
.\build\c_code_generator.exe --codegen examples\live_clock.cgen --run
```

This runs until you stop it (Ctrl+C). Without `--run`, codegen only writes the `.c` file. In the GUI, use **Open** → Build → Run and watch the Program Output pane.

### Password generator

[`examples/password_generator.cgen`](examples/password_generator.cgen) asks for:

1. Total password length  
2. How many digits  
3. How many special characters (`!@#$%&*?`)  
4. How many uppercase letters  

Lowercase count is `total - digits - specials - uppers`. It fills a `char` buffer, shuffles it, and prints the password.

```powershell
# Headless (pipe answers: total digits specials uppers)
"16`n3`n2`n3" | .\build\c_code_generator.exe --codegen examples\password_generator.cgen --run
```

### Prime number detector

[`examples/prime_number_detector.cgen`](examples/prime_number_detector.cgen) asks for an integer and reports whether it is prime.

- `2` and other primes print `N is prime.`
- Composites list proper divisors only (exclude `1` and `N`), e.g. `8` → `8 is not prime. Divisible by: 2 4`
- Values `< 2` are treated as not prime

```powershell
"8" | .\build\c_code_generator.exe --codegen examples\prime_number_detector.cgen --run
"7" | .\build\c_code_generator.exe --codegen examples\prime_number_detector.cgen --run
```

### Tic Tac Toe

[`examples/tic_tac_toe.cgen`](examples/tic_tac_toe.cgen) is a larger interactive example: you (X) vs a random AI (O).

- Randomly chooses who starts
- You enter **row** and **col** as `1`–`3`
- Rejects occupied or out-of-range cells
- Prints the board after every turn
- Before each match, prints win/draw counts from `ttt_history.txt` and who leads (you vs AI)
- After a finished match (win or draw), appends a line to `ttt_history.txt` with the result and a local timestamp (`Y-M-D H:M:S`)
- After the game: press **`r`** to restart, or any other character to quit

Uses **File Open** / **File Gets** / **File Printf** / **File Close**, **Local Time**, and **Scanf Char**.

```powershell
.\build\c_code_generator.exe --codegen examples\tic_tac_toe.cgen --run
```

History lines look like `X 2026-8-9 14:37:05` (`X` = you won, `O` = AI won, `D` = draw).

### Dungeon Log

[`examples/dungeon_log.cgen`](examples/dungeon_log.cgen) is a CLI adventure that showcases nearly every block family (control, arithmetic, strings, files, time, structs, functions, **StructLiteral**, multi-arg **Call**, **Malloc**/**Free**). Regenerated by [`scripts/generate_dungeon_log.py`](scripts/generate_dungeon_log.py).

- Enter a **hero name**, then use the menu: **1** Explore / **2** Rest / **3** Status / **4** Quit
- Explore rolls a room: fight, loot, or empty; combat uses HP/ATK; loot shuffles a table and adds gold
- Rest sleeps 1s, then asks for a float heal bonus
- Events append timestamped lines to `dungeon_log.txt` (codes: `0` empty, `1` win, `2` lose, `3` loot, `4` rest)
- On quit, enter any character to finish
- **Hero** is a local (no globals): init via **Struct Literal**, helpers take `Hero*` via **Address Of**, multi-arg status fields, stamp buffer uses **Malloc**/**Free**

```powershell
python scripts\generate_dungeon_log.py
"HeroName`n`n1`n2`n1.5`n3`n4`nx`n" | .\build\c_code_generator.exe --codegen examples\dungeon_log.cgen --run
```

A checked-in C snapshot lives at [`examples/build_out/dungeon_log.c`](examples/build_out/dungeon_log.c).

## Usage

- Place blocks from the left **Blocks** panel (filter box finds blocks by name)
- Drag nodes; connect amber **control** ports and blue **data** ports (wire preview turns green/red for type compatibility)
- **Wheel** pans vertically; **Shift+wheel** pans horizontally; **Ctrl+wheel** zooms
- **Middle-drag**, **Space+drag**, or **arrow keys** also pan the canvas; hover ports for name tooltips
- **Call** accepts up to 8 args (`Arg0`–`Arg7`); **Struct Literal** builds designated initializers; set Decl/Ref/Assign `type` (e.g. `uint8_t*`, `Hero`, `FILE*`) for typed ports; **Address Of** emits `&name` for pointer args
- **Shift+click** / marquee multi-select; **Ctrl+A** select all; **Ctrl+C** / **Ctrl+V** copy/paste
- Right-click a block for **Delete Block**, or a wired port for **Delete Wire**
- **Delete** / **Backspace** removes selected blocks (not Start); **Ctrl+Z** / **Ctrl+Y** undo/redo (up to 64 steps)
- **Tidy** / **Ctrl+L** auto-layouts control flow left-to-right
- **Snap** toggles grid snap (default on) and snaps the selection; **AlignL** / **AlignT** align a multi-selection
- **Fit** / **Ctrl+0** fits the whole graph; **Fit Sel** / **Ctrl+Shift+0** fits the selection; use the bottom-right **minimap** to jump
- Edit properties on the right (Enter commits; each commit is one undo step); choice fields (`type`, `op`, `access`, `function`, `paramCount`) offer dropdowns; clear selection to edit Document `fileDescription` / `clangFormat`
- **FunctionDef** params are typed (`paramCount`, `paramNName`, `paramNType`) with **Param** ports; **Call** Arg ports pick up name/types from the target function
- **Double-click** a **FunctionDef** to collapse/expand its body
- **Generate C** writes a doxygen `\\file` / `\\brief` header (from Document description), validates the graph, writes `build_out/<cgen-name>.c`, optionally runs **clang-format** when Document `clangFormat=1`, and opens a scrollable source view (Esc closes it)
- CLI `--codegen` writes only; `--compile` / `--run` are optional
- **Build** runs `gcc`; logs appear in the Compiler pane
- **Run** starts the program in the background and streams stdout into Program Output; type into the input line at the bottom of that pane for `scanf` / Enter prompts; **Stop** kills a running program
- **?** / **F1** opens in-app help
- **Save/Open** uses the `.cgen` (CGEN 1 + JSON) project format

## Layout

- `include/`, `src/` — application code
- `tests/` — GoogleTest unit tests (`cgen_unit_tests`)
- `cmake/Dependencies.cmake` — SFML / JSON wiring
- `third_party/nlohmann/` — vendored JSON header
- `third_party/googletest/` — GoogleTest submodule (v1.15.2)
- `third_party/sfml/` — local SFML sources (created by the build script; gitignored)
- `scripts/build.ps1` — one-shot fetch + build
- `examples/` — sample `.cgen` projects
- `build_out/` — generated C and executable artifacts
