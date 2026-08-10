# `.cgen` format

Projects save as **`.cgen`** files: a one-line magic header plus JSON.

Implemented by `SaveCgenFile` / `LoadCgenFile` in `serialize/cgen_serializer.*`.

---

## On-disk shape

```text
CGEN 1
{
  "nextNodeId": …,
  "nextEdgeId": …,
  "viewport": { "x": …, "y": …, "zoom": … },
  "fileDescription": "…",
  "clangFormat": true,
  "orthogonalWires": false,
  "types": [],
  "nodes": [ … ],
  "edges": [ … ]
}
```

- Line 1 must be exactly `CGEN 1` (version marker).  
- The remainder is pretty-printed JSON (`nlohmann::json`).  
- `types` is reserved (currently an empty array).

---

## Nodes

Each node object includes at least:

| Field | Meaning |
| --- | --- |
| `id` | `NodeId` |
| `type` | Stable string from `BlockTypeToString` (e.g. `"For"`, `"DerefLoad"`) |
| `posX`, `posY` | Canvas position |
| `properties` | String map (block-specific) |

Ports may be written for round-trips; on load, ports are reconciled via `CreateNode` defaults + `SyncAllNodePorts` so property-driven types stay consistent.

---

## Edges

| Field | Meaning |
| --- | --- |
| `id` | `EdgeId` |
| `fromNodeId`, `fromPort` | Source |
| `toNodeId`, `toPort` | Destination |

Port names are strings matching the live `Port::name` values (`"Next"`, `"Arg0"`, `"Param1"`, …).

---

## Load / save behavior

- **Save** — serializes the current `GraphDocument` (including viewport and document flags).  
- **Load** — parses header + JSON, rebuilds the document, sets file path, clears dirty, then **`SyncAllNodePorts`**.  
- Unknown `type` strings fail the load.  
- Missing file / bad header → `Result` error (`RejectsMissingFile` / `RejectsInvalidHeader` in tests).

Examples under `examples/*.cgen` are valid CGEN 1 projects and are good references when extending the format.

---

## Autosave / crash recovery

Autosave reuses the same CGEN 1 payload via `serialize/autosave.*` (not a separate format).

| File | Role |
| --- | --- |
| `%LOCALAPPDATA%\GraphicalCCodeGenerator\autosave.cgen.tmp` | Primary crash-recovery snapshot (while the document is dirty) |
| `autosave.cgen.tmp.meta` | Original document path (may be empty for untitled) |
| `<project>.cgen.tmp` | Optional sibling next to a saved `.cgen` |

Behavior (GUI):

- While dirty, `App` writes an autosave about every **30 seconds**.  
- On startup, if the crash autosave exists, a restore prompt is shown.  
- Successful **Save** / **New** / **Open** clears the autosave files.  
- `LoadAutosave` reloads the graph, restores the original path from metadata, and marks the document dirty.
