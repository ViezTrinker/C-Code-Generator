/*!
 *\file main.cpp
 *\brief Entry point for the C code generator application.
 */
#include <iostream>
#include <string_view>

#include "build/build_runner.h"
#include "codegen/c_codegen.h"
#include "gui/app.h"
#include "model/graph_document.h"
#include "serialize/cgen_serializer.h"

namespace
{
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

   int32_t RunCodegenFile(std::string_view filePath)
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
      return 0;
   }
} // namespace

int main(int argc, char** pArgv)
{
   if (argc > 1)
   {
      const std::string_view arg(pArgv[1]);
      if (arg == "--self-test")
      {
         return RunSelfTest();
      }
      if ((arg == "--codegen") && (argc > 2))
      {
         return RunCodegenFile(pArgv[2]);
      }
   }

   Cgen::App app;
   return app.Run();
}
