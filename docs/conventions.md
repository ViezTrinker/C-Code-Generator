# Conventions

Project conventions are enforced in review and mirrored in user rules. New code should match existing files under `include/` and `src/`.

---

## Language and style

| Rule | Practice |
| --- | --- |
| Standard | C++17, no compiler extensions |
| Namespace | `Cgen` only; close with `} // namespace Cgen` |
| Indent | Three spaces; Allman braces |
| Names | `PascalCase` types/functions; `camelCase` variables; `_private` members; `pPointer` params |
| Integers | Prefer `uint8_t` / `int32_t` / `uint64_t` over plain `int` |
| Empty params | Write `void` — `Run(void)` |
| Headers | Include guards (`#ifndef FOO_H`); self-contained; listed in `CMakeLists.txt` |
| Includes | Own header first, then `<…>`, then `"project…"` (alpha within each block) |
| Lambdas | **Do not use** — prefer named helpers / loops / switches |
| Comments | File + declaration doxygen: `/*! \brief … \param[in] … */` |

---

## Results instead of bare bools

Fallible APIs return `Cgen::Result` (or a richer struct that embeds `Result`). Use `IsOk` / `IsErr` at call sites.

Boolean returns are fine for predicates named `Is…` / `Has…` / `Contains…`.

Flag-like parameters prefer enums (`ClangFormatOnGenerate::Yes` / `No`) over raw `bool` arguments.

---

## Graph / GUI habits

- Mutate the document through `GraphDocument` APIs; push `DocumentHistory` checkpoints before user edits that should undo.  
- After changing type-related properties, call the appropriate `Sync*` helper.  
- Keep port names stable — they are part of the `.cgen` edge schema and codegen emitters.  
- `BlockType` string ids must remain stable for file compatibility (`BlockTypeToString` / `FromString`).

---

## Library split

| Put in `cgen_core` | Put in the exe |
| --- | --- |
| Model, validator, codegen, serializer | SFML widgets, `App`, file dialogs |
| Document history, block placement math | `BuildRunner`, `ProcessSession` |
| Anything unit-tested without a window | Anything that needs a live OS process UI |

When adding a feature: prefer implementing logic in `cgen_core` and a thin GUI binding, then add a GoogleTest case under `tests/`.
