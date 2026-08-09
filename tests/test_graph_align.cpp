/*!
 *\file test_graph_align.cpp
 *\brief Unit tests for snap/align helpers.
 */
#include <gtest/gtest.h>

#include "model/c_type.h"
#include "model/graph_align.h"
#include "model/graph_document.h"
#include "model/node.h"

TEST(GraphAlignTest, SnapCoordinateRoundsToGrid)
{
   EXPECT_FLOAT_EQ(Cgen::SnapCoordinateToGrid(11.0f, 20.0f), 20.0f);
   EXPECT_FLOAT_EQ(Cgen::SnapCoordinateToGrid(9.0f, 20.0f), 0.0f);
   EXPECT_FLOAT_EQ(Cgen::SnapCoordinateToGrid(30.0f, 20.0f), 40.0f);
}

TEST(GraphAlignTest, AlignLeftUsesMinimumX)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId leftId =
      document.AddNode(Cgen::BlockType::Printf, 40.0f, 80.0f);
   const Cgen::NodeId rightId =
      document.AddNode(Cgen::BlockType::Printf, 200.0f, 120.0f);
   std::vector<Cgen::NodeId> ids;
   ids.push_back(leftId);
   ids.push_back(rightId);
   Cgen::AlignNodes(&document.GetNodesMutable(), ids, Cgen::AlignSelection::Left);
   EXPECT_FLOAT_EQ(document.FindNode(leftId)->posX, 40.0f);
   EXPECT_FLOAT_EQ(document.FindNode(rightId)->posX, 40.0f);
}

TEST(FunctionParamTest, ParsesLegacyParamsIntoPorts)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId functionId =
      document.AddNode(Cgen::BlockType::FunctionDef, 40.0f, 40.0f);
   Cgen::Node* pFunction = document.FindNodeMutable(functionId);
   ASSERT_NE(pFunction, nullptr);
   pFunction->properties.erase("paramCount");
   pFunction->properties.erase("param0Name");
   pFunction->properties.erase("param0Type");
   pFunction->properties["params"] = "int32_t a, Hero* pHero";
   Cgen::SyncFunctionDefParams(pFunction);
   EXPECT_EQ(Cgen::GetFunctionParamCount(*pFunction), 2u);
   std::string name;
   std::string typeText;
   ASSERT_TRUE(Cgen::GetFunctionParam(*pFunction, 0, &name, &typeText));
   EXPECT_EQ(name, "a");
   EXPECT_EQ(typeText, "int32_t");
   ASSERT_TRUE(Cgen::GetFunctionParam(*pFunction, 1, &name, &typeText));
   EXPECT_EQ(name, "pHero");
   EXPECT_EQ(typeText, "Hero*");
   EXPECT_EQ(Cgen::FormatFunctionParamList(*pFunction), "int32_t a, Hero* pHero");
}

TEST(FunctionParamTest, CallArgsMatchFunctionDefTypes)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId functionId =
      document.AddNode(Cgen::BlockType::FunctionDef, 40.0f, 40.0f);
   Cgen::Node* pFunction = document.FindNodeMutable(functionId);
   ASSERT_NE(pFunction, nullptr);
   pFunction->properties["name"] = "combine";
   pFunction->properties["paramCount"] = "2";
   pFunction->properties["param0Name"] = "left";
   pFunction->properties["param0Type"] = "int32_t";
   pFunction->properties["param1Name"] = "right";
   pFunction->properties["param1Type"] = "float";
   pFunction->properties["returnType"] = "float";
   Cgen::SyncFunctionDefParams(pFunction);

   const Cgen::NodeId callId =
      document.AddNode(Cgen::BlockType::Call, 240.0f, 40.0f);
   Cgen::Node* pCall = document.FindNodeMutable(callId);
   ASSERT_NE(pCall, nullptr);
   pCall->properties["function"] = "combine";
   Cgen::SyncCallArgPorts(pCall, &document);

   const Cgen::Port* pArg0 = Cgen::FindPort(*pCall, "Arg0");
   const Cgen::Port* pArg1 = Cgen::FindPort(*pCall, "Arg1");
   const Cgen::Port* pArg2 = Cgen::FindPort(*pCall, "Arg2");
   const Cgen::Port* pResult = Cgen::FindPort(*pCall, "Result");
   ASSERT_NE(pArg0, nullptr);
   ASSERT_NE(pArg1, nullptr);
   ASSERT_NE(pArg2, nullptr);
   ASSERT_NE(pResult, nullptr);
   EXPECT_TRUE(pArg0->visible);
   EXPECT_TRUE(pArg1->visible);
   EXPECT_FALSE(pArg2->visible);
   EXPECT_EQ(Cgen::CTypeToString(pArg0->dataType), "int32_t");
   EXPECT_EQ(Cgen::CTypeToString(pArg1->dataType), "float");
   EXPECT_EQ(Cgen::CTypeToString(pResult->dataType), "float");
}
