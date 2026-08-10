# Graph model

The editor edits an in-memory **flowchart IR**. Codegen and validation consume that IR; they do not parse C.

---

## Core types

| Type | Meaning |
| --- | --- |
| `GraphDocument` | Nodes, edges, id counters, viewport, file metadata |
| `Node` | One block: `BlockType`, position, `ports`, string `properties` |
| `Edge` | Wire from `(fromNodeId, fromPort)` to `(toNodeId, toPort)` |
| `Port` | Named socket: Control or Data, In or Out, optional `CType`, visibility |
| `BlockType` | Kind of block (`Start`, `If`, `Call`, `DerefLoad`, …) |
| `CType` | Parsed type for data ports (`int32_t`, `Hero*`, `FILE*`, `bool`, …) |

Ids are `uint64_t` (`NodeId`, `EdgeId`). Properties are a `std::map`-like string dictionary (e.g. `name`, `type`, `function`, `paramCount`).

---

## Ports and wiring

- **Control** ports (amber in the UI): execution order (`In`, `Next`, `Then`, `Else`, `Body`, …).
- **Data** ports (blue): expressions (`Value`, `Cond`, `Arg0`, `Ptr`, …).

`GraphDocument::Connect` enforces:

1. Out → In only  
2. Same `PortKind` (control↔control, data↔data)  
3. For data: `AreTypesCompatible` on the port types  
4. At most one outgoing edge per out port and one incoming edge per in port  

`Start` cannot be removed.

---

## Creating and syncing nodes

`CreateNode(id, BlockType, x, y)` builds default ports and properties for that block (implemented in `node_factory.cpp`).

After properties change or a file loads, sync helpers keep ports honest:

| Helper | Effect |
| --- | --- |
| `SyncNodePortTypes` | Apply Decl/Ref/Assign/`type` (and Deref Ptr/Value) to ports |
| `SyncFunctionDefParams` | Param0–7 ports from `paramCount` / `paramNName` / `paramNType` |
| `SyncCallArgPorts` | Call Arg labels/types from the matching FunctionDef |
| `SyncPrintfArgVisibility` | Hide unused Printf args from the format string |
| `SyncAllNodePorts` | Run the above across the document (used after load) |

Expression vs statement: `IsExpressionBlock(BlockType)` — expressions feed data wires; statements participate in control chains.

---

## Document metadata

| Field | Use |
| --- | --- |
| `fileDescription` | Becomes doxygen `\brief` in generated C |
| `ClangFormatOnGenerate` | GUI Generate may run `clang-format -i` when Yes |
| Viewport `x` / `y` / `zoom` | Canvas camera (saved in `.cgen`) |

Clear the canvas selection to edit Document fields in the property panel.

---

## Validation

`ValidateGraph(document)` returns a `ValidationReport` of `ValidationIssue`s (`Error` or `Warning`) with optional `nodeId`.

`BuildNodeSeverityMap(report, &map)` collapses issues per node (**Error** wins over **Warning**) for canvas drawing.

Typical checks:

- No End reachable from Start  
- Missing required data wires (e.g. If `Cond`, Switch `Value`)  
- Undeclared variable references  
- Break/Continue outside a loop body  
- Unused declarations (warning)  
- Call missing `function` / unknown FunctionDef / arity mismatch / unused Param ports  
- Unreachable End (warning)  

**Where issues show up**

| Surface | Behavior |
| --- | --- |
| Compiler pane | Listed on Generate; click a line → `JumpToValidationNode` |
| Canvas badges | Red outline = Error, yellow = Warning (live refresh via `SyncSelectionUi`) |

Hovering a port still shows its name in a tip; on-canvas port labels sit below the block title band so they do not overlap titles such as `Else If`.

---

## Editor helpers (still model/)

| Module | Role |
| --- | --- |
| `graph_layout` | Left-to-right tidy of control flow |
| `graph_align` | Snap to grid; align left/top of selection |
| `graph_clipboard` | Serialize selection → paste with new ids |

Undo stores a `GraphSnapshot` (nodes/edges/counters/metadata — not viewport) via `DocumentHistory`.
