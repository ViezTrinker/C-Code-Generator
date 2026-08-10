# Modules

Sources are split by concern under `include/<module>/` and `src/<module>/`. This page maps folders to types and entry points.

---

## `model/` — graph IR

| Header / source | Responsibility |
| --- | --- |
| `graph_document.h/.cpp` | Editable document: nodes, edges, viewport, dirty flag, file path, `fileDescription`, clang-format preference |
| `node.h` + `node_factory.cpp` | `Node`, `CreateNode`, BlockType tables, port sync helpers |
| `block_type.h` | `BlockType` enum; `BlockTypeToString` / `FromString` / labels / help / `IsExpressionBlock` |
| `port.h`, `edge.h` | Port and edge structs |
| `c_type.h/.cpp` | `CType` / `PrimitiveType`; parse strings; `AreTypesCompatible` for wiring |
| `graph_validator.h/.cpp` | `ValidateGraph` → `ValidationReport`; `BuildNodeSeverityMap` for canvas badges |
| `graph_layout.h/.cpp` | Control-flow tidy layout (`ApplyAutoLayout`) |
| `graph_align.h/.cpp` | Grid snap and left/top align |
| `graph_clipboard.h/.cpp` | Copy/paste subgraph with remapped ids |
| `result.h` | Shared `Result` + `IsOk` / `IsErr` |

**Key APIs:** `GraphDocument::AddNode`, `RemoveNode`, `Connect`, `RemoveEdge`, `CaptureGraph` / `RestoreGraph`, `CreateNode`, `SyncNodePortTypes`, `SyncAllNodePorts`, `SyncFunctionDefParams`, `SyncCallArgPorts`.

---

## `codegen/` — C99 emission

| Header / source | Responsibility |
| --- | --- |
| `c_codegen.h/.cpp` | Walk the graph and emit a full translation unit |

**Key APIs:** `GenerateCSource(document)` → `CodegenOutput` (`source`, `diagnostics`, `result`); `GenerateCSnippet` for the property-panel preview.

Internally (anonymous namespace): statement chains from control ports, expression trees from data ports, file-scope struct/enum/typedef/globals/functions, then `main` from `Start`.

---

## `serialize/` — projects on disk

| Header / source | Responsibility |
| --- | --- |
| `cgen_serializer.h/.cpp` | Load/save `.cgen` (CGEN 1 + JSON) |
| `autosave.h/.cpp` | Crash-recovery paths, write/load/clear `.cgen.tmp` snapshots |

**Key APIs:** `SaveCgenFile`, `LoadCgenFile` (calls `SyncAllNodePorts` after load); `WriteAutosave`, `LoadAutosave`, `ClearAutosave`, `CrashAutosavePath`, `SiblingAutosavePath`.

Details: [`.cgen` format](cgen-format.md).

---

## `build/` — artifacts and processes

Compiled into the **executable** (not `cgen_core`).

| Header / source | Responsibility |
| --- | --- |
| `build_runner.h/.cpp` | Write `.c`, optional clang-format, `gcc` compile, synchronous run |
| `process_session.h/.cpp` | Interactive child process for GUI **Run** (stdin/stdout) |

**Key APIs:** `WriteSource`, `TryClangFormatSource`, `Compile`, `Run`; `ProcessSession::Start` / `WriteStdin` / `ConsumeOutput` / `Stop`.

---

## `gui/` — SFML interface

| Header / source | Responsibility |
| --- | --- |
| `app.h/.cpp` | Top-level window, layout, toolbar, theme, Generate/Build/Run, autosave / crash restore |
| `canvas_view.h/.cpp` | Pan/zoom, select, wire, place, clipboard, tidy, minimap, FunctionDef collapse, validation outlines |
| `palette.h/.cpp` | Collapsible block groups + filter + drag-and-drop place |
| `toolbar.h/.cpp` | File / build / layout / Theme / Help actions |
| `property_panel.h/.cpp` | Edit properties; Document fields when nothing selected |
| `log_pane.h/.cpp` | Text panes (Compiler with click-to-jump, Program Output + stdin, overlays) |
| `ui_theme.h/.cpp` | Dark/Light SFML color palettes (`GetUiTheme`) |
| `ui_theme_id.h/.cpp` | Theme id + string conversion (also in `cgen_core`) |
| `hover_tooltip.h/.cpp` | SFML draw for hover tips |
| `hover_tooltip_text.h/.cpp` | Palette / toolbar tip strings (also in `cgen_core`) |
| `document_history.h/.cpp` | Undo/redo (`GraphSnapshot`) — also in `cgen_core` |
| `block_placement.h/.cpp` | Non-overlapping placement + title-band / port layout math — also in `cgen_core` |
| `context_menu.h/.cpp` | Delete block / delete wire |

**Key APIs:** `App::Run`, `CanvasView::PlaceBlock` / `SetValidationSeverityMap` / `HandleMouse*`, `Palette::HandleClick` (`BeginDrag`) / `FinishBlockDrag` / `DrawDragGhost`, `PropertyPanel::SetSelection`, `DocumentHistory::PushCheckpoint`.

---

## Entry point

| File | Role |
| --- | --- |
| `src/main.cpp` | No args → GUI; `--codegen` / `--compile` / `--run`; `--self-test`; `--help` |

---

## Tests

`tests/test_*.cpp` exercise `cgen_core` (document, types, validator, severity map, codegen, serializer, autosave, layout, align, placement, history, theme ids, tooltip text). They do not drive the SFML UI.
