# Documentation

Technical notes for **Graphical C Code Generator** — how the code is organized and how data flows from flowchart to C99.

| Document | Contents |
| --- | --- |
| [Architecture](architecture.md) | Layers, targets, end-to-end flow |
| [Modules](modules.md) | Folders, classes, and responsibilities |
| [Graph model](graph-model.md) | Nodes, ports, edges, types, validation |
| [Codegen & build](codegen-and-build.md) | Validate → C → clang-format → gcc → run |
| [`.cgen` format](cgen-format.md) | On-disk project file layout |
| [Conventions](conventions.md) | Style and design rules used in the sources |

User-facing build / run instructions live in the root [README](../README.md).
