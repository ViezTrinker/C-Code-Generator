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
```

(MSVC multi-config generators may place the exe under `build\Release\`.)

## Example projects

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
.\build\c_code_generator.exe --codegen examples\live_clock.cgen
```

This runs until you stop it (Ctrl+C). In the GUI, use **Open** → Build → Run and watch the Program Output pane.

### Password generator

[`examples/password_generator.cgen`](examples/password_generator.cgen) asks for:

1. Total password length  
2. How many digits  
3. How many special characters (`!@#$%&*?`)  
4. How many uppercase letters  

Lowercase count is `total - digits - specials - uppers`. It fills a `char` buffer, shuffles it, and prints the password.

```powershell
# Headless (pipe answers: total digits specials uppers)
"16`n3`n2`n3" | .\build\c_code_generator.exe --codegen examples\password_generator.cgen
```

### Prime number detector

[`examples/prime_number_detector.cgen`](examples/prime_number_detector.cgen) asks for an integer and reports whether it is prime.

- `2` and other primes print `N is prime.`
- Composites list proper divisors only (exclude `1` and `N`), e.g. `8` → `8 is not prime. Divisible by: 2 4`
- Values `< 2` are treated as not prime

```powershell
"8" | .\build\c_code_generator.exe --codegen examples\prime_number_detector.cgen
"7" | .\build\c_code_generator.exe --codegen examples\prime_number_detector.cgen
```

## Usage

- Place blocks from the left palette
- Drag nodes; connect amber **control** ports and blue **data** ports
- Right-click a port to remove its wire; Delete removes the selected node
- Edit properties on the right (click a field, type, Enter to commit)
- **Generate C** writes `build_out/<cgen-name>.c` (e.g. `add_two_integers.cgen` → `add_two_integers.c`; unsaved docs use `untitled.c`)
- **Build** runs `gcc`; logs appear in the Compiler pane
- **Run** starts the program in the background and streams stdout into Program Output; type into the input line at the bottom of that pane for `scanf` / Enter prompts; **Stop** kills infinite loops
- **Save/Open** uses the `.cgen` (CGEN 1 + JSON) project format

## Layout

- `include/`, `src/` — application code
- `cmake/Dependencies.cmake` — SFML / JSON wiring
- `third_party/nlohmann/` — vendored JSON header
- `third_party/sfml/` — local SFML sources (created by the build script; gitignored)
- `scripts/build.ps1` — one-shot fetch + build
- `examples/` — sample `.cgen` projects
- `build_out/` — generated C and executable artifacts
