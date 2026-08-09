/*!
 *\file test_graph_validator.cpp
 *\brief Unit tests for flowchart validation.
 */
#include <gtest/gtest.h>

#include "model/graph_document.h"
#include "model/graph_validator.h"
#include "model/node.h"

TEST(GraphValidatorTest, ReportsMissingEndFromStart)
{
   Cgen::GraphDocument document;
   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if (report.issues[index].message.find("No End block reachable") !=
          std::string::npos)
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, ReportsMissingCondOnIf)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId ifId = document.AddNode(Cgen::BlockType::If, 200.0f, 40.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", ifId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(ifId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == ifId) &&
          (report.issues[index].message.find("Cond") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, ReportsUndeclaredVariableRef)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId refId =
      document.AddNode(Cgen::BlockType::VariableRef, 200.0f, 40.0f);
   document.FindNodeMutable(refId)->properties["name"] = "missingName";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == refId) &&
          (report.issues[index].message.find("undeclared") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, ReportsBreakOutsideLoop)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId breakId =
      document.AddNode(Cgen::BlockType::Break, 200.0f, 40.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", breakId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(breakId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == breakId) &&
          (report.issues[index].message.find("While/For") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, WarnsUnusedDeclaration)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 200.0f, 40.0f);
   document.FindNodeMutable(declId)->properties["name"] = "orphan";
   document.FindNodeMutable(declId)->properties["type"] = "int32_t";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", declId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(declId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == declId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Warning) &&
          (report.issues[index].message.find("Unused declaration") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, ReportsMissingSwitchValue)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId switchId =
      document.AddNode(Cgen::BlockType::Switch, 200.0f, 40.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", switchId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(switchId, "Default", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == switchId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Error) &&
          (report.issues[index].message.find("Value") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, WarnsMissingSwitchDefault)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId switchId =
      document.AddNode(Cgen::BlockType::Switch, 200.0f, 40.0f);
   const Cgen::NodeId litId =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 200.0f);
   document.FindNodeMutable(litId)->properties["value"] = "1";
   document.FindNodeMutable(litId)->properties["type"] = "int32_t";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", switchId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litId, "Value", switchId, "Value", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(switchId, "Cases", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == switchId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Warning) &&
          (report.issues[index].message.find("Default") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, WarnsUnreachableEnd)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   const Cgen::NodeId orphanEndId =
      document.AddNode(Cgen::BlockType::End, 400.0f, 200.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == orphanEndId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Warning) &&
          (report.issues[index].message.find("Unreachable End") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, WarnsUnknownCallFunction)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId callId =
      document.AddNode(Cgen::BlockType::Call, 200.0f, 40.0f);
   document.FindNodeMutable(callId)->properties["function"] = "missing_fn";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", callId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(callId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == callId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Warning) &&
          (report.issues[index].message.find("unknown function") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, ReportsCallArityMismatch)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId functionId =
      document.AddNode(Cgen::BlockType::FunctionDef, 40.0f, 200.0f);
   Cgen::Node* pFunction = document.FindNodeMutable(functionId);
   ASSERT_NE(pFunction, nullptr);
   pFunction->properties["name"] = "add2";
   pFunction->properties["paramCount"] = "2";
   pFunction->properties["param0Name"] = "a";
   pFunction->properties["param0Type"] = "int32_t";
   pFunction->properties["param1Name"] = "b";
   pFunction->properties["param1Type"] = "int32_t";
   Cgen::SyncFunctionDefParams(pFunction);

   const Cgen::NodeId litId =
      document.AddNode(Cgen::BlockType::Literal, 40.0f, 40.0f);
   const Cgen::NodeId callId =
      document.AddNode(Cgen::BlockType::Call, 200.0f, 40.0f);
   document.FindNodeMutable(callId)->properties["function"] = "add2";
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 400.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", callId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(litId, "Value", callId, "Arg0", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(document.Connect(callId, "Next", endId, "In", nullptr)));

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == callId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Error) &&
          (report.issues[index].message.find("expects 2") != std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}

TEST(GraphValidatorTest, WarnsUnusedFunctionParamPort)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId functionId =
      document.AddNode(Cgen::BlockType::FunctionDef, 40.0f, 40.0f);
   Cgen::Node* pFunction = document.FindNodeMutable(functionId);
   ASSERT_NE(pFunction, nullptr);
   pFunction->properties["name"] = "bump";
   pFunction->properties["paramCount"] = "1";
   pFunction->properties["param0Name"] = "x";
   pFunction->properties["param0Type"] = "int32_t";
   Cgen::SyncFunctionDefParams(pFunction);

   const Cgen::ValidationReport report = Cgen::ValidateGraph(document);
   bool found = false;
   for (size_t index = 0; index < report.issues.size(); ++index)
   {
      if ((report.issues[index].nodeId == functionId) &&
          (report.issues[index].severity == Cgen::ValidationSeverity::Warning) &&
          (report.issues[index].message.find("Unused Param port") !=
           std::string::npos))
      {
         found = true;
         break;
      }
   }
   EXPECT_TRUE(found);
}
