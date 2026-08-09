/*!
 *\file main.cpp
 *\brief Entry point for the C code generator application.
 */
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "build/build_runner.h"
#include "codegen/c_codegen.h"
#include "gui/app.h"
#include "model/graph_document.h"
#include "model/graph_validator.h"
#include "serialize/cgen_serializer.h"

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
      Cgen::GraphDocument document;
      document.Reset();

      const Cgen::NodeId startId = document.GetNodes().front().id;
      const Cgen::NodeId declId =
         document.AddNode(Cgen::BlockType::VariableDecl, 200.0f, 80.0f);
      const Cgen::NodeId litId =
         document.AddNode(Cgen::BlockType::Literal, 40.0f, 200.0f);
      const Cgen::NodeId printfId =
         document.AddNode(Cgen::BlockType::Printf, 400.0f, 80.0f);
      const Cgen::NodeId endId =
         document.AddNode(Cgen::BlockType::End, 600.0f, 80.0f);
      const Cgen::NodeId litArgId =
         document.AddNode(Cgen::BlockType::VariableRef, 250.0f, 220.0f);

      Cgen::Node* pDecl = document.FindNodeMutable(declId);
      Cgen::Node* pLit = document.FindNodeMutable(litId);
      Cgen::Node* pPrintf = document.FindNodeMutable(printfId);
      Cgen::Node* pLitArg = document.FindNodeMutable(litArgId);
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

      const Cgen::CodegenOutput generated = Cgen::GenerateCSource(document);
      if (Cgen::IsErr(generated.result))
      {
         std::cerr << "Codegen failed:\n" << generated.diagnostics << "\n";
         return 1;
      }

      Cgen::BuildRunner runner("build_out");
      runner.SetArtifactBaseName("self_test");
      if (Cgen::IsErr(runner.WriteSource(generated.source)))
      {
         std::cerr << "Failed to write .c file\n";
         return 1;
      }

      const Cgen::Result saveResult =
         Cgen::SaveCgenFile(document, "build_out/self_test.cgen");
      if (Cgen::IsErr(saveResult))
      {
         std::cerr << "Failed to save self_test.cgen\n";
         return 1;
      }

      Cgen::GraphDocument loaded;
      std::string diagnostics;
      const Cgen::Result loadResult =
         Cgen::LoadCgenFile("build_out/self_test.cgen", &loaded, &diagnostics);
      if (Cgen::IsErr(loadResult))
      {
         std::cerr << "Failed to reload .cgen:\n" << diagnostics << "\n";
         return 1;
      }

      const Cgen::BuildResult buildResult = runner.Compile();
      std::cout << buildResult.command << "\n" << buildResult.output;
      if (Cgen::IsErr(buildResult.result))
      {
         std::cerr << "Compile failed, exit=" << buildResult.exitCode << "\n";
         return 1;
      }
      const Cgen::BuildResult runResult = runner.Run();
      std::cout << runResult.output;
      if (Cgen::IsErr(runResult.result))
      {
         std::cerr << "Run failed, exit=" << runResult.exitCode << "\n";
         return 1;
      }
      std::cout << "Self-test passed.\n";
      return 0;
   }

   int32_t RunCodegenFile(std::string_view filePath, CodegenMode mode)
   {
      Cgen::GraphDocument document;
      std::string diagnostics;
      const Cgen::Result loadResult =
         Cgen::LoadCgenFile(filePath, &document, &diagnostics);
      if (Cgen::IsErr(loadResult))
      {
         std::cerr << "Failed to load .cgen:\n" << diagnostics << "\n";
         return 1;
      }

      const Cgen::ValidationReport validation = Cgen::ValidateGraph(document);
      bool hasError = false;
      for (size_t index = 0; index < validation.issues.size(); ++index)
      {
         const Cgen::ValidationIssue& issue = validation.issues[index];
         const char* pLabel =
            (issue.severity == Cgen::ValidationSeverity::Error) ? "error" : "warning";
         std::cerr << "[" << pLabel << "] " << issue.message << "\n";
         if (issue.severity == Cgen::ValidationSeverity::Error)
         {
            hasError = true;
         }
      }
      if (hasError)
      {
         std::cerr << "Validation failed; not writing C.\n";
         return 1;
      }

      const Cgen::CodegenOutput generated = Cgen::GenerateCSource(document);
      if (Cgen::IsErr(generated.result))
      {
         std::cerr << "Codegen failed:\n" << generated.diagnostics << "\n";
         return 1;
      }

      Cgen::BuildRunner runner("build_out");
      runner.SetArtifactBaseName(filePath);
      if (Cgen::IsErr(runner.WriteSource(generated.source)))
      {
         std::cerr << "Failed to write .c file\n";
         return 1;
      }
      std::cout << "Wrote " << runner.GetSourcePath() << "\n";

      if (mode == CodegenMode::WriteOnly)
      {
         return 0;
      }

      const Cgen::BuildResult buildResult = runner.Compile();
      std::cout << buildResult.command << "\n" << buildResult.output;
      if (Cgen::IsErr(buildResult.result))
      {
         std::cerr << "Compile failed, exit=" << buildResult.exitCode << "\n";
         return 1;
      }

      if (mode == CodegenMode::Compile)
      {
         return 0;
      }

      const Cgen::BuildResult runResult = runner.Run();
      std::cout << runResult.output;
      if (Cgen::IsErr(runResult.result))
      {
         std::cerr << "Run failed, exit=" << runResult.exitCode << "\n";
         return 1;
      }
      return 0;
   }
} // namespace

int main(int argc, char** pArgv)
{
   if (argc <= 1)
   {
      Cgen::App app;
      return app.Run();
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
