/*!
 *\file test_graph_layout.cpp
 *\brief Unit tests for auto-layout and clipboard paste.
 */
#include <gtest/gtest.h>

#include "model/graph_clipboard.h"
#include "model/graph_document.h"
#include "model/graph_layout.h"

TEST(GraphLayoutTest, PlacesEndToTheRightOfStart)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId startId = document.GetNodes().front().id;
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 10.0f, 10.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(startId, "Next", endId, "In", nullptr)));

   Cgen::ApplyAutoLayout(&document);
   const Cgen::Node* pStart = document.FindNode(startId);
   const Cgen::Node* pEnd = document.FindNode(endId);
   ASSERT_NE(pStart, nullptr);
   ASSERT_NE(pEnd, nullptr);
   EXPECT_GT(pEnd->posX, pStart->posX);
}

TEST(GraphClipboardTest, PastesRemappedInternalEdge)
{
   Cgen::GraphDocument document;
   const Cgen::NodeId printfId =
      document.AddNode(Cgen::BlockType::Printf, 100.0f, 100.0f);
   const Cgen::NodeId endId = document.AddNode(Cgen::BlockType::End, 300.0f, 100.0f);
   ASSERT_TRUE(Cgen::IsOk(document.Connect(printfId, "Next", endId, "In", nullptr)));

   std::vector<Cgen::NodeId> selected;
   selected.push_back(printfId);
   selected.push_back(endId);

   Cgen::GraphClipboard clipboard;
   Cgen::CopySelectionToClipboard(document, selected, &clipboard);
   ASSERT_TRUE(clipboard.hasContent);
   EXPECT_EQ(clipboard.nodes.size(), 2u);
   EXPECT_EQ(clipboard.edges.size(), 1u);

   std::vector<Cgen::NodeId> pasted;
   ASSERT_TRUE(Cgen::PasteClipboardIntoDocument(
      &document, clipboard, 40.0f, 40.0f, &pasted));
   EXPECT_EQ(pasted.size(), 2u);

   const Cgen::Edge* pEdge =
      document.FindOutgoingEdge(pasted[0], "Next");
   ASSERT_NE(pEdge, nullptr);
   EXPECT_EQ(pEdge->toNodeId, pasted[1]);
   EXPECT_EQ(document.FindNode(pasted[0])->type, Cgen::BlockType::Printf);
   EXPECT_EQ(document.FindNode(pasted[1])->type, Cgen::BlockType::End);
}
