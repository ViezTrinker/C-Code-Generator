# Codegen and build

How a `GraphDocument` becomes a running program.

---

## Pipeline

```text
ValidateGraph
      │
GenerateCSource  ──►  CodegenOutput { source, diagnostics, result }
      │
BuildRunner::WriteSource  ──►  build_out/<base>.c
      │
[optional] TryClangFormatSource   (GUI + Document clangFormat=Yes)
      │
BuildRunner::Compile  ──►  gcc -std=c99 … -o build_out/<base>.exe
      │
Run: ProcessSession (GUI)  or  BuildRunner::Run (CLI / sync)
```

Artifact base name comes from the `.cgen` path stem (`SetArtifactBaseName`), or `untitled` when unsaved.

---

## Code generation (`GenerateCSource`)

High-level emission order:

1. Doxygen `\file` / `\brief` (from stem + `fileDescription`, or a default brief)  
2. `#include`s driven by used features (`stdio`, `stdint`, `stdbool`, `stdlib`, …)  
3. File-scope **Struct** / **Enum** / **Typedef** decls  
4. **GlobalDecl**s  
5. **FunctionDef** bodies (from each function’s `Body` chain)  
6. `main` — control chain from `Start`’s `Next`  

Statements follow control wires (`Next`, branches, loop bodies). Expressions are built by walking **incoming data** edges (`EmitExpression` / `EmitInputExpression`). Temps may be cached per node id during a emit pass.

Failures append to `diagnostics` and may still produce partial source for inspection.

Property panel preview uses `GenerateCSnippet` for a single selected node’s approximate C.

---

## Validation vs write policy

| Context | Validation **Error** | Validation **Warning** |
| --- | --- | --- |
| GUI **Generate C** | Shown in Compiler; `.c` still written | Shown; write continues |
| CLI `--codegen` | Abort; no write | Printed to stderr; write continues |

---

## clang-format

`BuildRunner::TryClangFormatSource` runs:

```text
clang-format -i "<sourcePath>"
```

- Triggered from the GUI only when Document `clangFormat` is enabled (`1` / Yes).  
- If `clang-format` is missing from `PATH`, Generate treats that as non-fatal (Compiler message notes apply-or-skip).  
- CLI `--codegen` does not invoke clang-format today.

---

## Compile and run

**Compile** shells out to `gcc` with C99, warnings, and `-O` flags (see Compiler pane for the exact command).

**GUI Run**

1. Ensure sources exist (may Generate + Build first).  
2. `ProcessSession::Start` launches the exe.  
3. Stdout/stderr → Program Output; the bottom input line → stdin.  
4. **Stop** kills the child.

**CLI `--run`** uses `BuildRunner::Run` (synchronous; suitable for piped stdin).

---

## Related entry points

| Caller | Functions |
| --- | --- |
| `App::GenerateCode` | `ValidateGraph`, `GenerateCSource`, `WriteSource`, optional format, show source overlay; canvas badges refreshed via selection sync |
| `App::BuildCode` / `RunProgram` | Generate if needed, then compile / session |
| `App` main loop | Periodic autosave while dirty; crash-restore prompt at startup |
| `main` `--codegen` | Load → validate → generate → write → optional compile/run |
