/*!
 *\file test_graph_validator.cpp
 *\brief Unit tests for flowchart validation.
 */
#include <gtest/gtest.h>

#include "model/graph_document.h"
#include "model/graph_validator.h"

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
