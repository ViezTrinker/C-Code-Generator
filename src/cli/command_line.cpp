/*!
 *\file command_line.cpp
 *\brief Headless CLI modes (codegen, self-test, help).
 */
#include "cli/command_line.h"

#include <iostream>
#include <string>
#include <string_view>

#include "build/build_runner.h"
#include "codegen/c_codegen.h"
#include "model/graph_document.h"
#include "model/graph_validator.h"
#include "serialize/cgen_serializer.h"

namespace Cgen
{
   namespace
   {
      enum class CodegenMode: uint8_t
      {
         WriteOnly = 0,
         Compile,
         Run
      };

      void PrintCliUsage(void)
      {
         std::cerr
            << "Graphical C Code Generator\n"
            << "https://github.com/ViezTrinker/Graphical-C-Code-Generator\n"
            << "\n"
            << "Usage:\n"
            << "  c_code_generator.exe\n"
            << "  c_code_generator.exe --self-test\n"
            << "  c_code_generator.exe --codegen <file.cgen> [--compile | --run]\n"
            << "\n"
            << "  --codegen <file>   Generate C into build_out/ (write only).\n"
            << "  --compile          Also compile with gcc after codegen.\n"
            << "  --run              Compile and run (implies --compile).\n";
      }

      int32_t RunSelfTest(void)
      {
         GraphDocument document;
         document.Reset();

         const NodeId startId = document.GetNodes().front().id;
         const NodeId declId =
            document.AddNode(BlockType::VariableDecl, 200.0f, 80.0f);
         const NodeId litId =
            document.AddNode(BlockType::Literal, 40.0f, 200.0f);
         const NodeId printfId =
            document.AddNode(BlockType::Printf, 400.0f, 80.0f);
         const NodeId endId =
            document.AddNode(BlockType::End, 600.0f, 80.0f);
         const NodeId litArgId =
            document.AddNode(BlockType::VariableRef, 250.0f, 220.0f);

         Node* pDecl = document.FindNodeMutable(declId);
         Node* pLit = document.FindNodeMutable(litId);
         Node* pPrintf = document.FindNodeMutable(printfId);
         Node* pLitArg = document.FindNodeMutable(litArgId);
         if ((pDecl == nullptr) || (pLit == nullptr) || (pPrintf == nullptr) ||
             (pLitArg == nullptr))
         {
            std::cerr << "Self-test failed to create nodes.\n";
            return 1;
         }
         pDecl->properties["name"] = "value";
         pDecl->properties["type"] = "int32_t";
         pLit->properties["value"] = "42";
         pLit->properties["type"] = "int32_t";
         pPrintf->properties["format"] = "hello value=%d\\n";
         pLitArg->properties["name"] = "value";

         document.Connect(startId, "Next", declId, "In", nullptr);
         document.Connect(litId, "Value", declId, "Init", nullptr);
         document.Connect(declId, "Next", printfId, "In", nullptr);
         document.Connect(litArgId, "Value", printfId, "Arg0", nullptr);
         document.Connect(printfId, "Next", endId, "In", nullptr);

         const CodegenOutput generated = GenerateCSource(document);
         if (IsErr(generated.result))
         {
            std::cerr << "Codegen failed:\n" << generated.diagnostics << "\n";
            return 1;
         }

         BuildRunner runner("build_out");
         runner.SetArtifactBaseName("self_test");
         if (IsErr(runner.WriteSource(generated.source)))
         {
            std::cerr << "Failed to write .c file\n";
            return 1;
         }

         const Result saveResult =
            SaveCgenFile(document, "build_out/self_test.cgen");
         if (IsErr(saveResult))
         {
            std::cerr << "Failed to save self_test.cgen\n";
            return 1;
         }

         GraphDocument loaded;
         std::string diagnostics;
         const Result loadResult =
            LoadCgenFile("build_out/self_test.cgen", &loaded, &diagnostics);
         if (IsErr(loadResult))
         {
            std::cerr << "Failed to reload .cgen:\n" << diagnostics << "\n";
            return 1;
         }

         const BuildResult buildResult = runner.Compile();
         std::cout << buildResult.command << "\n" << buildResult.output;
         if (IsErr(buildResult.result))
         {
            std::cerr << "Compile failed, exit=" << buildResult.exitCode << "\n";
            return 1;
         }
         const BuildResult runResult = runner.Run();
         std::cout << runResult.output;
         if (IsErr(runResult.result))
         {
            std::cerr << "Run failed, exit=" << runResult.exitCode << "\n";
            return 1;
         }
         std::cout << "Self-test passed.\n";
         return 0;
      }

      int32_t RunCodegenFile(std::string_view filePath, CodegenMode mode)
      {
         GraphDocument document;
         std::string diagnostics;
         const Result loadResult =
            LoadCgenFile(filePath, &document, &diagnostics);
         if (IsErr(loadResult))
         {
            std::cerr << "Failed to load .cgen:\n" << diagnostics << "\n";
            return 1;
         }

         const ValidationReport validation = ValidateGraph(document);
         bool hasError = false;
         for (size_t index = 0; index < validation.issues.size(); ++index)
         {
            const ValidationIssue& issue = validation.issues[index];
            const char* pLabel =
               (issue.severity == ValidationSeverity::Error) ? "error"
                                                              : "warning";
            std::cerr << "[" << pLabel << "] " << issue.message << "\n";
            if (issue.severity == ValidationSeverity::Error)
            {
               hasError = true;
            }
         }
         if (hasError)
         {
            std::cerr << "Validation failed; not writing C.\n";
            return 1;
         }

         const CodegenOutput generated = GenerateCSource(document);
         if (IsErr(generated.result))
         {
            std::cerr << "Codegen failed:\n" << generated.diagnostics << "\n";
            return 1;
         }

         BuildRunner runner("build_out");
         runner.SetArtifactBaseName(filePath);
         if (IsErr(runner.WriteSource(generated.source)))
         {
            std::cerr << "Failed to write .c file\n";
            return 1;
         }
         std::cout << "Wrote " << runner.GetSourcePath() << "\n";

         if (mode == CodegenMode::WriteOnly)
         {
            return 0;
         }

         const BuildResult buildResult = runner.Compile();
         std::cout << buildResult.command << "\n" << buildResult.output;
         if (IsErr(buildResult.result))
         {
            std::cerr << "Compile failed, exit=" << buildResult.exitCode << "\n";
            return 1;
         }

         if (mode == CodegenMode::Compile)
         {
            return 0;
         }

         const BuildResult runResult = runner.Run();
         std::cout << runResult.output;
         if (IsErr(runResult.result))
         {
            std::cerr << "Run failed, exit=" << runResult.exitCode << "\n";
            return 1;
         }
         return 0;
      }
   } // namespace

   int32_t RunCommandLine(int32_t argc, char** pArgv)
   {
      if ((argc <= 1) || (pArgv == nullptr))
      {
         PrintCliUsage();
         return 1;
      }

      const std::string_view first(pArgv[1]);
      if (first == "--self-test")
      {
         return RunSelfTest();
      }
      if ((first == "--help") || (first == "-h"))
      {
         PrintCliUsage();
         return 0;
      }
      if (first != "--codegen")
      {
         PrintCliUsage();
         return 1;
      }
      if (argc < 3)
      {
         PrintCliUsage();
         return 1;
      }

      const std::string_view filePath(pArgv[2]);
      CodegenMode mode = CodegenMode::WriteOnly;
      for (int32_t argIndex = 3; argIndex < argc; ++argIndex)
      {
         const std::string_view flag(pArgv[argIndex]);
         if (flag == "--compile")
         {
            if (mode != CodegenMode::Run)
            {
               mode = CodegenMode::Compile;
            }
            continue;
         }
         if (flag == "--run")
         {
            mode = CodegenMode::Run;
            continue;
         }
         std::cerr << "Unknown flag: " << flag << "\n";
         PrintCliUsage();
         return 1;
      }

      return RunCodegenFile(filePath, mode);
   }
} // namespace Cgen
