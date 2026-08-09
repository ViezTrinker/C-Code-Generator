/*!
 *\file test_c_codegen.cpp
 *\brief Unit tests for C99 code generation.
 */
#include <gtest/gtest.h>

#include <string>

#include "codegen/c_codegen.h"
#include "model/graph_document.h"

TEST(CCodegenTest, EmitsIncludesAndMain)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId printfId =
      document.AddNode(Cgen::BlockType::Printf, 200.0f, 40.0f);
   Cgen::Node* pPrintf = document.FindNodeMutable(printfId);
   ASSERT_NE(pPrintf, nullptr);
   pPrintf->properties["format"] = "hello\\n";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 360.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", printfId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result));
   EXPECT_NE(output.source.find("#include <stdio.h>"), std::string::npos);
   EXPECT_NE(output.source.find("int main(void)"), std::string::npos);
   EXPECT_NE(output.source.find("printf(\"hello\\n\")"), std::string::npos);
   EXPECT_NE(output.source.find("return 0;"), std::string::npos);
}

TEST(CCodegenTest, EmitsVariableAndAssignment)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 160.0f, 40.0f);
   Cgen::Node* pDecl = document.FindNodeMutable(declId);
   ASSERT_NE(pDecl, nullptr);
   pDecl->properties["name"] = "value";
   pDecl->properties["type"] = "int32_t";

   const Cgen::NodeId literalId =
      document.AddNode(Cgen::BlockType::Literal, 160.0f, 140.0f);
   Cgen::Node* pLiteral = document.FindNodeMutable(literalId);
   ASSERT_NE(pLiteral, nullptr);
   pLiteral->properties["value"] = "42";

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 320.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(literalId, "Value", declId, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result));
   EXPECT_NE(output.source.find("int32_t value = 42;"), std::string::npos);
}

TEST(CCodegenTest, EmitsScanfChar)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 160.0f, 40.0f);
   Cgen::Node* pDecl = document.FindNodeMutable(declId);
   ASSERT_NE(pDecl, nullptr);
   pDecl->properties["name"] = "ch";
   pDecl->properties["type"] = "char";

   const Cgen::NodeId scanfId =
      document.AddNode(Cgen::BlockType::ScanfChar, 320.0f, 40.0f);
   Cgen::Node* pScanf = document.FindNodeMutable(scanfId);
   ASSERT_NE(pScanf, nullptr);
   pScanf->properties["target"] = "ch";
   pScanf->properties["prompt"] = "char: ";

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 480.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", scanfId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(scanfId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result));
   EXPECT_NE(output.source.find("scanf(\" %c\", &ch)"), std::string::npos);
}
