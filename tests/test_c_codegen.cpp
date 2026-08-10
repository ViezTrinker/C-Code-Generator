/*!
 *\file test_c_codegen.cpp
 *\brief Unit tests for C99 code generation.
 */
#include <gtest/gtest.h>

#include <string>

#include "codegen/c_codegen.h"
#include "model/graph_document.h"
#include "model/graph_validator.h"
#include "model/node.h"

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
   EXPECT_NE(output.source.find("\\file "), std::string::npos);
}

TEST(CCodegenTest, EmitsFileDescriptionBrief)
{
   Cgen::GraphDocument document;
   document.SetFileDescription("Adds two demo integers.");
   document.SetFilePath("examples/demo.cgen");
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 200.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", nullptr)));
   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result));
   EXPECT_NE(output.source.find("\\file demo.c"), std::string::npos);
   EXPECT_NE(output.source.find("\\brief Adds two demo integers."), std::string::npos);
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

TEST(CCodegenTest, EmitsStdBoolIncludeForBoolDecl)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 160.0f, 40.0f);
   Cgen::Node* pDecl = document.FindNodeMutable(declId);
   ASSERT_NE(pDecl, nullptr);
   pDecl->properties["name"] = "ready";
   pDecl->properties["type"] = "bool";

   const Cgen::NodeId literalId =
      document.AddNode(Cgen::BlockType::Literal, 160.0f, 140.0f);
   Cgen::Node* pLiteral = document.FindNodeMutable(literalId);
   ASSERT_NE(pLiteral, nullptr);
   pLiteral->properties["value"] = "true";

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 320.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(literalId, "Value", declId, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("#include <stdbool.h>"), std::string::npos);
   EXPECT_NE(output.source.find("bool ready = true;"), std::string::npos);
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

TEST(CCodegenTest, EmitsLogicAndUnary)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 120.0f, 40.0f);
   Cgen::Node* pDecl = document.FindNodeMutable(declId);
   ASSERT_NE(pDecl, nullptr);
   pDecl->properties["name"] = "flag";

   const Cgen::NodeId leftLit =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 140.0f);
   document.FindNodeMutable(leftLit)->properties["value"] = "1";
   const Cgen::NodeId rightLit =
      document.AddNode(Cgen::BlockType::Literal, 140.0f, 140.0f);
   document.FindNodeMutable(rightLit)->properties["value"] = "0";
   const Cgen::NodeId andId = document.AddNode(Cgen::BlockType::And, 100.0f, 220.0f);
   const Cgen::NodeId notId = document.AddNode(Cgen::BlockType::Not, 100.0f, 300.0f);
   const Cgen::NodeId negLit =
      document.AddNode(Cgen::BlockType::Literal, 260.0f, 140.0f);
   document.FindNodeMutable(negLit)->properties["value"] = "5";
   const Cgen::NodeId negId = document.AddNode(Cgen::BlockType::Neg, 260.0f, 220.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(leftLit, "Value", andId, "Left", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(rightLit, "Value", andId, "Right", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(andId, "Result", notId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(negLit, "Value", negId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(notId, "Result", declId, "Init", nullptr)));

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 320.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("&&"), std::string::npos);
   EXPECT_NE(output.source.find("(!"), std::string::npos);
}

TEST(CCodegenTest, EmitsElseIfChain)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId ifId = document.AddNode(Cgen::BlockType::If, 160.0f, 40.0f);
   const Cgen::NodeId elseIfId =
      document.AddNode(Cgen::BlockType::ElseIf, 160.0f, 160.0f);
   const Cgen::NodeId condA =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 40.0f);
   document.FindNodeMutable(condA)->properties["value"] = "1";
   const Cgen::NodeId condB =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 160.0f);
   document.FindNodeMutable(condB)->properties["value"] = "2";
   const Cgen::NodeId thenPrintf =
      document.AddNode(Cgen::BlockType::Printf, 320.0f, 40.0f);
   document.FindNodeMutable(thenPrintf)->properties["format"] = "a\\n";
   const Cgen::NodeId elseIfPrintf =
      document.AddNode(Cgen::BlockType::Printf, 320.0f, 160.0f);
   document.FindNodeMutable(elseIfPrintf)->properties["format"] = "b\\n";
   const Cgen::NodeId elsePrintf =
      document.AddNode(Cgen::BlockType::Printf, 320.0f, 280.0f);
   document.FindNodeMutable(elsePrintf)->properties["format"] = "c\\n";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 480.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", ifId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(condA, "Value", ifId, "Cond", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(ifId, "Then", thenPrintf, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(ifId, "Else", elseIfId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(condB, "Value", elseIfId, "Cond", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(elseIfId, "Then", elseIfPrintf, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(elseIfId, "Else", elsePrintf, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(ifId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("if ("), std::string::npos);
   EXPECT_NE(output.source.find("else if ("), std::string::npos);
}

TEST(CCodegenTest, EmitsSwitchCaseBreakContinue)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId switchId =
      document.AddNode(Cgen::BlockType::Switch, 160.0f, 40.0f);
   const Cgen::NodeId valueLit =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 40.0f);
   document.FindNodeMutable(valueLit)->properties["value"] = "1";
   const Cgen::NodeId caseId = document.AddNode(Cgen::BlockType::Case, 160.0f, 140.0f);
   document.FindNodeMutable(caseId)->properties["value"] = "1";
   const Cgen::NodeId casePrintf =
      document.AddNode(Cgen::BlockType::Printf, 320.0f, 140.0f);
   document.FindNodeMutable(casePrintf)->properties["format"] = "one\\n";
   const Cgen::NodeId defPrintf =
      document.AddNode(Cgen::BlockType::Printf, 320.0f, 240.0f);
   document.FindNodeMutable(defPrintf)->properties["format"] = "other\\n";

   const Cgen::NodeId whileId =
      document.AddNode(Cgen::BlockType::While, 160.0f, 320.0f);
   const Cgen::NodeId whileCond =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 320.0f);
   document.FindNodeMutable(whileCond)->properties["value"] = "0";
   const Cgen::NodeId breakId =
      document.AddNode(Cgen::BlockType::Break, 320.0f, 320.0f);
   const Cgen::NodeId contId =
      document.AddNode(Cgen::BlockType::Continue, 320.0f, 400.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 480.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", switchId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(valueLit, "Value", switchId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(switchId, "Cases", caseId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(caseId, "Body", casePrintf, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(switchId, "Default", defPrintf, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(switchId, "Next", whileId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(whileCond, "Value", whileId, "Cond", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(whileId, "Body", breakId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(whileId, "Exit", contId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(contId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("switch ("), std::string::npos);
   EXPECT_NE(output.source.find("case 1:"), std::string::npos);
   EXPECT_NE(output.source.find("break;"), std::string::npos);
   EXPECT_NE(output.source.find("continue;"), std::string::npos);
}

TEST(CCodegenTest, EmitsStringsTypedIndexAndCompound)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId arrayId =
      document.AddNode(Cgen::BlockType::ArrayDecl, 120.0f, 40.0f);
   const Cgen::NodeId strcpyId =
      document.AddNode(Cgen::BlockType::StrCpy, 280.0f, 40.0f);
   document.FindNodeMutable(strcpyId)->properties["dest"] = "buffer";
   document.FindNodeMutable(strcpyId)->properties["src"] = "src";
   const Cgen::NodeId scanfLineId =
      document.AddNode(Cgen::BlockType::ScanfLine, 440.0f, 40.0f);
   const Cgen::NodeId indexId =
      document.AddNode(Cgen::BlockType::IndexAssign, 600.0f, 40.0f);
   Cgen::Node* pIndex = document.FindNodeMutable(indexId);
   pIndex->properties["array"] = "nums";
   pIndex->properties["elemType"] = "int32_t";
   const Cgen::NodeId idxLit =
      document.AddNode(Cgen::BlockType::Literal, 520.0f, 140.0f);
   document.FindNodeMutable(idxLit)->properties["value"] = "0";
   const Cgen::NodeId valLit =
      document.AddNode(Cgen::BlockType::Literal, 620.0f, 140.0f);
   document.FindNodeMutable(valLit)->properties["value"] = "7";
   const Cgen::NodeId incId = document.AddNode(Cgen::BlockType::Inc, 760.0f, 40.0f);
   document.FindNodeMutable(incId)->properties["target"] = "value";
   const Cgen::NodeId compoundId =
      document.AddNode(Cgen::BlockType::CompoundAssign, 900.0f, 40.0f);
   document.FindNodeMutable(compoundId)->properties["target"] = "value";
   document.FindNodeMutable(compoundId)->properties["op"] = "+";
   const Cgen::NodeId addLit =
      document.AddNode(Cgen::BlockType::Literal, 900.0f, 140.0f);
   document.FindNodeMutable(addLit)->properties["value"] = "3";
   const Cgen::NodeId strlenId =
      document.AddNode(Cgen::BlockType::StrLen, 100.0f, 200.0f);
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 100.0f, 280.0f);
   document.FindNodeMutable(declId)->properties["name"] = "len";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 1040.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", arrayId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(arrayId, "Next", strcpyId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(strcpyId, "Next", scanfLineId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(scanfLineId, "Next", indexId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(idxLit, "Value", indexId, "Index", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(valLit, "Value", indexId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(indexId, "Next", incId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(incId, "Next", compoundId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(addLit, "Value", compoundId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(compoundId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(strlenId, "Value", declId, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("#include <string.h>"), std::string::npos);
   EXPECT_NE(output.source.find("strcpy("), std::string::npos);
   EXPECT_NE(output.source.find("fgets("), std::string::npos);
   EXPECT_NE(output.source.find("strlen("), std::string::npos);
   EXPECT_NE(output.source.find("] = (int32_t)("), std::string::npos);
   EXPECT_NE(output.source.find("++value;"), std::string::npos);
   EXPECT_NE(output.source.find("value += "), std::string::npos);
}

TEST(CCodegenTest, EmitsCastScanfFloatAssertComment)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId floatDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 120.0f, 40.0f);
   document.FindNodeMutable(floatDecl)->properties["name"] = "value";
   document.FindNodeMutable(floatDecl)->properties["type"] = "float";

   const Cgen::NodeId scanfFloatId =
      document.AddNode(Cgen::BlockType::ScanfFloat, 280.0f, 40.0f);
   document.FindNodeMutable(scanfFloatId)->properties["target"] = "value";

   const Cgen::NodeId litId =
      document.AddNode(Cgen::BlockType::Literal, 280.0f, 140.0f);
   document.FindNodeMutable(litId)->properties["value"] = "3";
   const Cgen::NodeId castId = document.AddNode(Cgen::BlockType::Cast, 400.0f, 140.0f);
   document.FindNodeMutable(castId)->properties["toType"] = "float";
   const Cgen::NodeId intDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 520.0f, 40.0f);
   document.FindNodeMutable(intDecl)->properties["name"] = "asFloat";
   document.FindNodeMutable(intDecl)->properties["type"] = "float";

   const Cgen::NodeId commentId =
      document.AddNode(Cgen::BlockType::Comment, 680.0f, 40.0f);
   document.FindNodeMutable(commentId)->properties["text"] = "check value";

   const Cgen::NodeId condLit =
      document.AddNode(Cgen::BlockType::Literal, 680.0f, 140.0f);
   document.FindNodeMutable(condLit)->properties["value"] = "1";
   const Cgen::NodeId assertId =
      document.AddNode(Cgen::BlockType::Assert, 840.0f, 40.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 1000.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", floatDecl, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(floatDecl, "Next", scanfFloatId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(scanfFloatId, "Next", intDecl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litId, "Value", castId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(castId, "Result", intDecl, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(intDecl, "Next", commentId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(commentId, "Next", assertId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(condLit, "Value", assertId, "Cond", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(assertId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("#include <assert.h>"), std::string::npos);
   EXPECT_NE(output.source.find("scanf(\"%f\", &value)"), std::string::npos);
   EXPECT_NE(output.source.find("(float)(3)"), std::string::npos);
   EXPECT_NE(output.source.find("/* check value */"), std::string::npos);
   EXPECT_NE(output.source.find("assert(1);"), std::string::npos);
}

TEST(CCodegenTest, EmitsPerBlockCommentWhenNonEmpty)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 160.0f, 40.0f);
   Cgen::Node* pDecl = document.FindNodeMutable(declId);
   ASSERT_NE(pDecl, nullptr);
   pDecl->properties["name"] = "value";
   pDecl->properties["type"] = "int32_t";
   pDecl->properties["comment"] = "declare value";

   const Cgen::NodeId litId =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 160.0f);
   Cgen::Node* pLit = document.FindNodeMutable(litId);
   ASSERT_NE(pLit, nullptr);
   pLit->properties["value"] = "42";
   pLit->properties["comment"] = "answer";

   const Cgen::NodeId printfId =
      document.AddNode(Cgen::BlockType::Printf, 360.0f, 40.0f);
   Cgen::Node* pPrintf = document.FindNodeMutable(printfId);
   ASSERT_NE(pPrintf, nullptr);
   pPrintf->properties["format"] = "%d\\n";
   pPrintf->properties["comment"] = "   ";

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 560.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litId, "Value", declId, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", printfId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("/* declare value */"), std::string::npos);
   EXPECT_NE(output.source.find("42 /* answer */"), std::string::npos);
   EXPECT_EQ(output.source.find("/*    */"), std::string::npos);

   const std::string declSnippet = Cgen::GenerateCSnippet(document, declId);
   EXPECT_NE(declSnippet.find("/* declare value */"), std::string::npos);
}

TEST(CCodegenTest, EmitsSnippetForSelectedBlocks)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId litId = document.AddNode(Cgen::BlockType::Literal, 120.0f, 40.0f);
   document.FindNodeMutable(litId)->properties["value"] = "7";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", nullptr)));

   const std::string literalSnippet = Cgen::GenerateCSnippet(document, litId);
   EXPECT_NE(literalSnippet.find("7"), std::string::npos);

   const std::string startSnippet = Cgen::GenerateCSnippet(document, startId);
   EXPECT_NE(startSnippet.find("main"), std::string::npos);
}

TEST(CCodegenTest, EmitsFileIoAndStructFields)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId structId =
      document.AddNode(Cgen::BlockType::StructDecl, 40.0f, 200.0f);
   document.FindNodeMutable(structId)->properties["name"] = "Point";
   document.FindNodeMutable(structId)->properties["fields"] = "int32_t x; int32_t y";

   const Cgen::NodeId pointDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 120.0f, 40.0f);
   document.FindNodeMutable(pointDecl)->properties["name"] = "point";
   document.FindNodeMutable(pointDecl)->properties["type"] = "Point";

   const Cgen::NodeId fpDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 280.0f, 40.0f);
   document.FindNodeMutable(fpDecl)->properties["name"] = "fp";
   document.FindNodeMutable(fpDecl)->properties["type"] = "FILE*";

   const Cgen::NodeId openId =
      document.AddNode(Cgen::BlockType::FileOpen, 440.0f, 40.0f);
   const Cgen::NodeId writeId =
      document.AddNode(Cgen::BlockType::FileWrite, 600.0f, 40.0f);
   const Cgen::NodeId readId =
      document.AddNode(Cgen::BlockType::FileRead, 760.0f, 40.0f);
   const Cgen::NodeId printfId =
      document.AddNode(Cgen::BlockType::FilePrintf, 820.0f, 40.0f);
   document.FindNodeMutable(printfId)->properties["format"] = "%d\\n";
   const Cgen::NodeId printfArg =
      document.AddNode(Cgen::BlockType::Literal, 820.0f, 120.0f);
   document.FindNodeMutable(printfArg)->properties["value"] = "42";
   const Cgen::NodeId bufDecl =
      document.AddNode(Cgen::BlockType::ArrayDecl, 880.0f, 200.0f);
   document.FindNodeMutable(bufDecl)->properties["name"] = "line";
   document.FindNodeMutable(bufDecl)->properties["elemType"] = "char";
   document.FindNodeMutable(bufDecl)->properties["size"] = "64";
   const Cgen::NodeId okDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 1040.0f, 200.0f);
   document.FindNodeMutable(okDecl)->properties["name"] = "ok";
   const Cgen::NodeId getsId =
      document.AddNode(Cgen::BlockType::FileGets, 1200.0f, 40.0f);
   document.FindNodeMutable(getsId)->properties["target"] = "line";
   document.FindNodeMutable(getsId)->properties["size"] = "64";
   document.FindNodeMutable(getsId)->properties["status"] = "ok";
   const Cgen::NodeId closeId =
      document.AddNode(Cgen::BlockType::FileClose, 1360.0f, 40.0f);

   const Cgen::NodeId fieldVal =
      document.AddNode(Cgen::BlockType::Literal, 120.0f, 140.0f);
   document.FindNodeMutable(fieldVal)->properties["value"] = "9";
   const Cgen::NodeId fieldStore =
      document.AddNode(Cgen::BlockType::FieldStore, 280.0f, 140.0f);
   const Cgen::NodeId fieldLoad =
      document.AddNode(Cgen::BlockType::FieldLoad, 440.0f, 140.0f);
   const Cgen::NodeId copyDecl =
      document.AddNode(Cgen::BlockType::VariableDecl, 600.0f, 140.0f);
   document.FindNodeMutable(copyDecl)->properties["name"] = "copied";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 1080.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", pointDecl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(pointDecl, "Next", fpDecl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(fpDecl, "Next", bufDecl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(bufDecl, "Next", okDecl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(okDecl, "Next", openId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(openId, "Next", writeId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(writeId, "Next", readId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(readId, "Next", printfId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(printfArg, "Value", printfId, "Arg0", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", getsId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(getsId, "Next", closeId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(closeId, "Next", fieldStore, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(fieldVal, "Value", fieldStore, "Value", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(fieldStore, "Next", copyDecl, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(fieldLoad, "Value", copyDecl, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(copyDecl, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("typedef struct Point"), std::string::npos);
   EXPECT_NE(output.source.find("int32_t x;"), std::string::npos);
   EXPECT_NE(output.source.find("fopen(\"data.bin\", \"rb\")"), std::string::npos);
   EXPECT_NE(output.source.find("fwrite("), std::string::npos);
   EXPECT_NE(output.source.find("fread("), std::string::npos);
   EXPECT_NE(output.source.find("fprintf(fp, \"%d\\n\", 42);"), std::string::npos);
   EXPECT_NE(output.source.find("fgets(line, 64, fp)"), std::string::npos);
   EXPECT_NE(output.source.find("fclose(fp);"), std::string::npos);
   EXPECT_NE(output.source.find("point.x = 9;"), std::string::npos);
   EXPECT_NE(output.source.find("point.x"), std::string::npos);
}

TEST(CCodegenTest, EmitsStructDeclBeforeGlobalOfStructType)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId structId =
      document.AddNode(Cgen::BlockType::StructDecl, 40.0f, -40.0f);
   document.FindNodeMutable(structId)->properties["name"] = "Hero";
   document.FindNodeMutable(structId)->properties["fields"] =
      "int32_t hp; int32_t atk";

   const Cgen::NodeId globalId =
      document.AddNode(Cgen::BlockType::GlobalDecl, 40.0f, 40.0f);
   document.FindNodeMutable(globalId)->properties["name"] = "hero";
   document.FindNodeMutable(globalId)->properties["type"] = "Hero";

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;

   const size_t structPos = output.source.find("typedef struct Hero");
   const size_t globalPos = output.source.find("Hero hero;");
   ASSERT_NE(structPos, std::string::npos);
   ASSERT_NE(globalPos, std::string::npos);
   EXPECT_LT(structPos, globalPos);
}

TEST(CCodegenTest, EmitsMultiArgCallAndStructLiteral)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId litA =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 40.0f);
   document.FindNodeMutable(litA)->properties["value"] = "1";
   const Cgen::NodeId litB =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 100.0f);
   document.FindNodeMutable(litB)->properties["value"] = "2";
   const Cgen::NodeId litC =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 160.0f);
   document.FindNodeMutable(litC)->properties["value"] = "3";
   const Cgen::NodeId callId =
      document.AddNode(Cgen::BlockType::Call, 200.0f, 40.0f);
   document.FindNodeMutable(callId)->properties["function"] = "combine";
   document.FindNodeMutable(callId)->properties["storeTo"] = "sum";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", callId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litA, "Value", callId, "Arg0", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litB, "Value", callId, "Arg1", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litC, "Value", callId, "Arg2", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(callId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput callOutput = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(callOutput.result)) << callOutput.diagnostics;
   EXPECT_NE(callOutput.source.find("sum = combine(1, 2, 3);"), std::string::npos);

   Cgen::GraphDocument literalDoc;
   const Cgen::NodeId structLit =
      literalDoc.AddNode(Cgen::BlockType::StructLiteral, 40.0f, 40.0f);
   literalDoc.FindNodeMutable(structLit)->properties["type"] = "Hero";
   literalDoc.FindNodeMutable(structLit)->properties["init"] =
      ".hp = 30, .atk = 6";
   const Cgen::NodeId decl =
      literalDoc.AddNode(Cgen::BlockType::VariableDecl, 200.0f, 40.0f);
   literalDoc.FindNodeMutable(decl)->properties["name"] = "hero";
   literalDoc.FindNodeMutable(decl)->properties["type"] = "Hero";
   Cgen::SyncNodePortTypes(literalDoc.FindNodeMutable(decl));
   Cgen::SyncNodePortTypes(literalDoc.FindNodeMutable(structLit));
   const Cgen::NodeId litStart = literalDoc.GetNodes().front().id;
   const Cgen::NodeId litEnd =
      literalDoc.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(
      Cgen::IsOk(literalDoc.Connect(litStart, "Next", decl, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(literalDoc.Connect(structLit, "Value", decl, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(literalDoc.Connect(decl, "Next", litEnd, "In", nullptr)));

   const Cgen::CodegenOutput litOutput = Cgen::GenerateCSource(literalDoc);
   EXPECT_TRUE(Cgen::IsOk(litOutput.result)) << litOutput.diagnostics;
   EXPECT_NE(litOutput.source.find("Hero hero = (Hero){ .hp = 30, .atk = 6 };"),
             std::string::npos);
}

TEST(CCodegenTest, EmitsMallocIntoTypedPointerDecl)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId sizeLit =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 120.0f);
   document.FindNodeMutable(sizeLit)->properties["value"] = "16";

   const Cgen::NodeId mallocId =
      document.AddNode(Cgen::BlockType::Malloc, 200.0f, 40.0f);
   Cgen::Port* pSize =
      Cgen::FindPortMutable(document.FindNodeMutable(mallocId), "Size");
   ASSERT_NE(pSize, nullptr);
   pSize->dataType.base = Cgen::PrimitiveType::Int32;
   pSize->dataType.isPointer = false;

   const Cgen::NodeId decl =
      document.AddNode(Cgen::BlockType::VariableDecl, 360.0f, 40.0f);
   document.FindNodeMutable(decl)->properties["name"] = "pBuf";
   document.FindNodeMutable(decl)->properties["type"] = "uint8_t*";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(decl));
   const Cgen::NodeId freeId =
      document.AddNode(Cgen::BlockType::Free, 520.0f, 40.0f);
   const Cgen::NodeId ref =
      document.AddNode(Cgen::BlockType::VariableRef, 520.0f, 120.0f);
   document.FindNodeMutable(ref)->properties["name"] = "pBuf";
   document.FindNodeMutable(ref)->properties["type"] = "uint8_t*";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(ref));
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 700.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", decl, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(sizeLit, "Value", mallocId, "Size", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(mallocId, "Ptr", decl, "Init", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(decl, "Next", freeId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(ref, "Value", freeId, "Ptr", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(freeId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find(
                "uint8_t* pBuf = (uint8_t*)(malloc((size_t)(16)));"),
             std::string::npos);
   EXPECT_NE(output.source.find("free(pBuf);"), std::string::npos);
}

TEST(CCodegenTest, EmitsAddressOfIntoCallArg)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId decl =
      document.AddNode(Cgen::BlockType::VariableDecl, 40.0f, 40.0f);
   document.FindNodeMutable(decl)->properties["name"] = "hero";
   document.FindNodeMutable(decl)->properties["type"] = "Hero";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(decl));
   const Cgen::NodeId addr =
      document.AddNode(Cgen::BlockType::AddressOf, 200.0f, 120.0f);
   document.FindNodeMutable(addr)->properties["name"] = "hero";
   document.FindNodeMutable(addr)->properties["type"] = "Hero*";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(addr));
   const Cgen::NodeId callId =
      document.AddNode(Cgen::BlockType::Call, 200.0f, 40.0f);
   document.FindNodeMutable(callId)->properties["function"] = "do_combat";
   document.FindNodeMutable(callId)->properties["storeTo"] = "";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", decl, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(decl, "Next", callId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(addr, "Value", callId, "Arg0", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(callId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      EXPECT_NE(report.issues[index].severity, Cgen::ValidationSeverity::Error)
         << report.issues[index].message;
   }

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("do_combat(&hero);"), std::string::npos);
}

TEST(CCodegenTest, EmitsNestedFieldPath)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId store =
      document.AddNode(Cgen::BlockType::FieldStore, 200.0f, 40.0f);
   document.FindNodeMutable(store)->properties["object"] = "hero";
   document.FindNodeMutable(store)->properties["field"] = "stats.hp";
   document.FindNodeMutable(store)->properties["access"] = ".";
   const Cgen::NodeId lit =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 120.0f);
   document.FindNodeMutable(lit)->properties["value"] = "10";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", store, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(lit, "Value", store, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(store, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("hero.stats.hp = 10;"), std::string::npos);
}

TEST(CCodegenTest, EmitsEnumAndTypedefBeforeGlobals)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId enumId =
      document.AddNode(Cgen::BlockType::EnumDecl, 40.0f, -80.0f);
   document.FindNodeMutable(enumId)->properties["name"] = "Color";
   document.FindNodeMutable(enumId)->properties["enumerators"] = "Red, Green, Blue";

   const Cgen::NodeId typedefId =
      document.AddNode(Cgen::BlockType::TypedefDecl, 40.0f, -40.0f);
   document.FindNodeMutable(typedefId)->properties["name"] = "Byte";
   document.FindNodeMutable(typedefId)->properties["type"] = "uint8_t";

   const Cgen::NodeId globalColor =
      document.AddNode(Cgen::BlockType::GlobalDecl, 40.0f, 40.0f);
   document.FindNodeMutable(globalColor)->properties["name"] = "tint";
   document.FindNodeMutable(globalColor)->properties["type"] = "Color";

   const Cgen::NodeId globalByte =
      document.AddNode(Cgen::BlockType::GlobalDecl, 40.0f, 80.0f);
   document.FindNodeMutable(globalByte)->properties["name"] = "flag";
   document.FindNodeMutable(globalByte)->properties["type"] = "Byte";

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;

   const size_t enumPos = output.source.find("typedef enum Color");
   const size_t typedefPos = output.source.find("typedef uint8_t Byte;");
   const size_t tintPos = output.source.find("Color tint;");
   const size_t flagPos = output.source.find("Byte flag;");
   ASSERT_NE(enumPos, std::string::npos);
   ASSERT_NE(typedefPos, std::string::npos);
   ASSERT_NE(tintPos, std::string::npos);
   ASSERT_NE(flagPos, std::string::npos);
   EXPECT_LT(enumPos, tintPos);
   EXPECT_LT(typedefPos, flagPos);
   EXPECT_NE(output.source.find("Red"), std::string::npos);
   EXPECT_NE(output.source.find("Green"), std::string::npos);
   EXPECT_NE(output.source.find("Blue"), std::string::npos);
}

TEST(CCodegenTest, EmitsDerefLoadAndStore)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 40.0f, 40.0f);
   document.FindNodeMutable(declId)->properties["name"] = "value";
   document.FindNodeMutable(declId)->properties["type"] = "int32_t";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(declId));

   const Cgen::NodeId addrStoreId =
      document.AddNode(Cgen::BlockType::AddressOf, 40.0f, 120.0f);
   document.FindNodeMutable(addrStoreId)->properties["name"] = "value";
   document.FindNodeMutable(addrStoreId)->properties["type"] = "int32_t*";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(addrStoreId));

   const Cgen::NodeId addrLoadId =
      document.AddNode(Cgen::BlockType::AddressOf, 40.0f, 200.0f);
   document.FindNodeMutable(addrLoadId)->properties["name"] = "value";
   document.FindNodeMutable(addrLoadId)->properties["type"] = "int32_t*";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(addrLoadId));

   const Cgen::NodeId litId =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 280.0f);
   document.FindNodeMutable(litId)->properties["value"] = "42";
   document.FindNodeMutable(litId)->properties["type"] = "int32_t";

   const Cgen::NodeId storeId =
      document.AddNode(Cgen::BlockType::DerefStore, 200.0f, 40.0f);
   document.FindNodeMutable(storeId)->properties["type"] = "int32_t";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(storeId));

   const Cgen::NodeId loadId =
      document.AddNode(Cgen::BlockType::DerefLoad, 200.0f, 160.0f);
   document.FindNodeMutable(loadId)->properties["type"] = "int32_t";
   Cgen::SyncNodePortTypes(document.FindNodeMutable(loadId));

   const Cgen::NodeId printfId =
      document.AddNode(Cgen::BlockType::Printf, 360.0f, 40.0f);
   document.FindNodeMutable(printfId)->properties["format"] = "%d\\n";

   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 520.0f, 40.0f);

   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", storeId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(addrStoreId, "Value", storeId, "Ptr", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(litId, "Value", storeId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(storeId, "Next", printfId, "In", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(addrLoadId, "Value", loadId, "Ptr", nullptr)));
   ASSERT_TRUE(
      Cgen::IsOk(document.Connect(loadId, "Value", printfId, "Arg0", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", endId, "In", nullptr)));

   const Cgen::CodegenOutput output = Cgen::GenerateCSource(document);
   EXPECT_TRUE(Cgen::IsOk(output.result)) << output.diagnostics;
   EXPECT_NE(output.source.find("(*&value) = 42;"), std::string::npos);
   EXPECT_NE(output.source.find("(int32_t)((*&value))"), std::string::npos);
}
