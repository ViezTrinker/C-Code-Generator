# Architecture

Graphical C Code Generator is a C++17 desktop app: an SFML flowchart editor that emits C99, optionally formats it, then compiles and runs with `gcc`.

Everything application-owned lives in the **`Cgen`** namespace.

---

## Build targets

| Target | Role |
| --- | --- |
| **`cgen_core`** (static lib) | Graph IR, validation, codegen, `.cgen` I/O, autosave helpers, undo snapshots, block placement, **node style** (size/color helpers), theme ids, tooltip text |
| **`c_code_generator`** (exe) | SFML UI, toolbar/palette/canvas, themes, CLI (`command_line`), `BuildRunner`, interactive `ProcessSession` |
| **`cgen_unit_tests`** (optional) | GoogleTest against `cgen_core` |

Headless codegen and most model logic do **not** need SFML; the GUI and process I/O sit in the executable.

```text
┌─────────────────────────────────────────────────────────────┐
│  c_code_generator.exe                                       │
│  main.cpp  │  App / CLI / Canvas / …  │  BuildRunner    │
└─────────────┬───────────────────────────────┬───────────────┘
              │ links                         │
              ▼                               ▼
        ┌───────────┐                  gcc / clang-format
        │ cgen_core │                  (external tools)
        │ model     │
        │ codegen   │
        │ serialize │
        └───────────┘
```

---

## Repository layout

| Path | Purpose |
| --- | --- |
| `include/`, `src/` | Application sources (headers mirror modules) |
| `tests/` | Unit tests |
| `examples/` | Sample `.cgen` projects (+ checked-in `build_out/*.c`) |
| `cmake/` | Dependency bootstrap (JSON, SFML) |
| `scripts/build.ps1` | One-shot Windows configure/build |
| `docs/` | This documentation + screenshots |
| `third_party/` | Vendored nlohmann JSON; GoogleTest submodule; local SFML (often gitignored) |
| `build_out/` | Generated `.c` / executables at runtime |

---

## Runtime data flow

```text
  .cgen file                    User edits on canvas / properties
       │                              │
       ▼                              ▼
 LoadCgenFile ──────────► GraphDocument ◄──── Palette drag-drop / Properties
                               │
                               ├─► ValidateGraph  (debounced while editing)
                               │      ├─► Compiler pane issues strip (click → jump)
                               │      └─► Canvas badges (red/yellow outlines)
                               │
                               ├─► Autosave (.cgen.tmp / crash recovery)
                               │
                               ▼
                         GenerateCSource  (+ optional per-block /* comments */)
                               │
                               ▼
                    BuildRunner::WriteSource
                               │
              optional TryClangFormatSource
                               │
                    Compile (gcc) / Run (ProcessSession)
```

1. **IR** — `GraphDocument` holds `Node`s and `Edge`s (plus document metadata).
2. **Validate** — `ValidateGraph` reports errors/warnings. The GUI refreshes issues **live** (debounced) into the Compiler pane and canvas badges; Generate/Build can replace the Compiler text with codegen/gcc output.
3. **Generate** — `GenerateCSource` walks control flow from `Start` and each `FunctionDef`, including optional per-block `comment` properties.
4. **Artifacts** — write `build_out/<name>.c`, optionally `clang-format -i`, then `gcc`.
5. **GUI Run** — `ProcessSession` streams stdin/stdout into Program Output.
6. **Autosave** — while dirty, periodic snapshots protect long edits; a crash-recovery prompt can restore on next launch.

GUI Generate still writes C when there are validation/codegen diagnostics (issues are shown for inspection). CLI `--codegen` **refuses to write** if validation reports an **Error**.

---

## GUI shell

`App` owns the window and wires panels:

| Region | Component |
| --- | --- |
| Top | `Toolbar` (file, Generate, Build, Run, layout, Ortho, Theme, About, Help) |
| Left | `Palette` (drag blocks onto the canvas) |
| Center | `CanvasView` (graph, resize handle, per-block colors, validation badges, wire reconnect, optional orthogonal wires) |
| Right | `PropertyPanel` (node / Document / style fields; caret editing) |
| Bottom | `LogPane` Program Output (stdin caret) \| Compiler (live issues / build log) |
| Overlay | Generated C / Help / About (`LogPane`); palette drag ghost; context menu |

Selection flows: canvas → `SyncSelectionUi` → property panel + **request live validation**. Undo uses `DocumentHistory` (`GraphSnapshot`, depth 64). Theme preference is stored under `%LOCALAPPDATA%\GraphicalCCodeGenerator\`. UI **Theme** (chrome) is separate from per-block `fillColor` / `textColor` presets.

---

## CLI vs GUI

| Mode | Entry |
| --- | --- |
| GUI | `c_code_generator.exe` (no args) → `App::Run` |
| Headless | `RunCommandLine` → `--codegen <file.cgen> [--compile \| --run]` |
| Smoke | `--self-test` (tiny graph → save/load → compile → run) |

`src/main.cpp` only chooses GUI vs CLI; CLI logic lives in `cli/command_line.*`.

See [Codegen & build](codegen-and-build.md) for the shared pipeline.
