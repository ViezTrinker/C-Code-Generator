# Architecture

Graphical C Code Generator is a C++17 desktop app: an SFML flowchart editor that emits C99, optionally formats it, then compiles and runs with `gcc`.

Everything application-owned lives in the **`Cgen`** namespace.

---

## Build targets

| Target | Role |
| --- | --- |
| **`cgen_core`** (static lib) | Graph IR, validation, codegen, `.cgen` I/O, undo snapshots, block placement helpers |
| **`c_code_generator`** (exe) | SFML UI, toolbar/palette/canvas, `BuildRunner`, interactive `ProcessSession`, CLI entry |
| **`cgen_unit_tests`** (optional) | GoogleTest against `cgen_core` |

Headless codegen and most model logic do **not** need SFML; the GUI and process I/O sit in the executable.

```text
┌─────────────────────────────────────────────────────────────┐
│  c_code_generator.exe                                       │
│  main.cpp  │  App / Canvas / Palette / …  │  BuildRunner    │
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
  .cgen file                    User edits on canvas
       │                              │
       ▼                              ▼
 LoadCgenFile ──────────► GraphDocument ◄──── Palette / Properties
                               │
                               ├─► ValidateGraph  (issues → Compiler pane)
                               │
                               ▼
                         GenerateCSource
                               │
                               ▼
                    BuildRunner::WriteSource
                               │
              optional TryClangFormatSource
                               │
                    Compile (gcc) / Run (ProcessSession)
```

1. **IR** — `GraphDocument` holds `Node`s and `Edge`s (plus document metadata).
2. **Validate** — `ValidateGraph` reports errors/warnings (reachability, Call arity, …).
3. **Generate** — `GenerateCSource` walks control flow from `Start` and each `FunctionDef`.
4. **Artifacts** — write `build_out/<name>.c`, optionally `clang-format -i`, then `gcc`.
5. **GUI Run** — `ProcessSession` streams stdin/stdout into Program Output.

GUI Generate still writes C when there are validation/codegen diagnostics (issues are shown for inspection). CLI `--codegen` **refuses to write** if validation reports an **Error**.

---

## GUI shell

`App` owns the window and wires panels:

| Region | Component |
| --- | --- |
| Top | `Toolbar` (file, Generate, Build, Run, layout) |
| Left | `Palette` (place blocks) |
| Center | `CanvasView` (graph interaction) |
| Right | `PropertyPanel` (node or Document fields) |
| Bottom | `LogPane` Program Output \| Compiler |
| Overlay | Generated C view / Help (`LogPane`) |

Selection flows: canvas → `SyncSelectionUi` → property panel. Undo uses `DocumentHistory` (`GraphSnapshot`, depth 64).

---

## CLI vs GUI

| Mode | Entry |
| --- | --- |
| GUI | `c_code_generator.exe` (no args) → `App::Run` |
| Headless | `--codegen <file.cgen> [--compile \| --run]` |
| Smoke | `--self-test` (tiny graph → save/load → compile → run) |

See [Codegen & build](codegen-and-build.md) for the shared pipeline.
