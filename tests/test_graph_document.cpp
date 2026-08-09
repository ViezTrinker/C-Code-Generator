/*!
 *\file test_graph_document.cpp
 *\brief Unit tests for GraphDocument mutations and wiring rules.
 */
#include <gtest/gtest.h>

#include "model/graph_document.h"

TEST(GraphDocumentTest, StartsWithStartNode)
{
   Cgen::GraphDocument document;
   ASSERT_FALSE(document.GetNodes().empty());
   EXPECT_EQ(document.GetNodes().front().type, Cgen::BlockType::Start);
   EXPECT_FALSE(document.IsDirty());
}

TEST(GraphDocumentTest, AddAndRemoveNode)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId literalId = document.AddNode(Cgen::BlockType::Literal, 10.0f, 20.0f);
   EXPECT_TRUE(document.IsDirty());
   ASSERT_NE(document.FindNode(literalId), nullptr);
   EXPECT_EQ(document.FindNode(literalId)->type, Cgen::BlockType::Literal);

   EXPECT_TRUE(Cgen::IsOk(document.RemoveNode(literalId)));
   EXPECT_EQ(document.FindNode(literalId), nullptr);
}

TEST(GraphDocumentTest, CannotRemoveStartNode)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   EXPECT_EQ(document.RemoveNode(startId), Cgen::Result::InvalidArgument);
   EXPECT_NE(document.FindNode(startId), nullptr);
}

TEST(GraphDocumentTest, ConnectControlFlow)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 200.0f, 80.0f);
   Cgen::EdgeId edgeId = 0;
   EXPECT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", &edgeId)));
   EXPECT_NE(edgeId, 0u);
   ASSERT_NE(document.FindOutgoingEdge(startId, "Next"), nullptr);
   EXPECT_EQ(document.FindOutgoingEdge(startId, "Next")->toNodeId, endId);
}

TEST(GraphDocumentTest, RejectsDuplicateOutgoingControl)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId endA = document.AddNode(Cgen::BlockType::End, 200.0f, 80.0f);
   const Cgen::NodeId endB = document.AddNode(Cgen::BlockType::End, 200.0f, 160.0f);
   EXPECT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endA, "In", nullptr)));
   EXPECT_EQ(document.Connect(startId, "Next", endB, "In", nullptr),
             Cgen::Result::InvalidArgument);
}

TEST(GraphDocumentTest, ConnectDataPorts)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId literalId = document.AddNode(Cgen::BlockType::Literal, 40.0f, 40.0f);
   const Cgen::NodeId declId =
      document.AddNode(Cgen::BlockType::VariableDecl, 200.0f, 40.0f);
   EXPECT_TRUE(Cgen::IsOk(document.Connect(literalId, "Value", declId, "Init", nullptr)));
}

TEST(GraphDocumentTest, RejectsSelfConnect)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   EXPECT_EQ(document.Connect(startId, "Next", startId, "In", nullptr),
             Cgen::Result::InvalidArgument);
}

TEST(GraphDocumentTest, RemoveNodeAlsoRemovesIncidentEdges)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId printfId = document.AddNode(Cgen::BlockType::Printf, 200.0f, 40.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 360.0f, 40.0f);
   EXPECT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", printfId, "In", nullptr)));
   EXPECT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", endId, "In", nullptr)));
   EXPECT_TRUE(Cgen::IsOk(document.RemoveNode(printfId)));
   EXPECT_EQ(document.FindOutgoingEdge(startId, "Next"), nullptr);
   EXPECT_TRUE(document.GetEdges().empty());
}

TEST(GraphDocumentTest, CaptureAndRestoreGraph)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 100.0f, 100.0f);
   const Cgen::GraphSnapshot snapshot = document.CaptureGraph();
   document.AddNode(Cgen::BlockType::Printf, 200.0f, 200.0f);
   EXPECT_EQ(document.GetNodes().size(), 3u);
   document.RestoreGraph(snapshot);
   EXPECT_EQ(document.GetNodes().size(), 2u);
   EXPECT_NE(document.FindNode(endId), nullptr);
}
