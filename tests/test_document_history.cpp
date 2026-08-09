/*!
 *\file test_document_history.cpp
 *\brief Unit tests for undo/redo history.
 */
#include <gtest/gtest.h>

#include "gui/document_history.h"
#include "model/graph_document.h"

TEST(DocumentHistoryTest, UndoRestoresPreviousGraph)
{
   Cgen::GraphDocument document;
   Cgen::DocumentHistory history;
   EXPECT_FALSE(history.CanUndo());
   EXPECT_FALSE(history.CanRedo());

   history.PushCheckpoint(document);
   const Cgen::NodeId addedId = document.AddNode(Cgen::BlockType::End, 50.0f, 50.0f);
   EXPECT_EQ(document.GetNodes().size(), 2u);

   ASSERT_TRUE(history.Undo(&document));
   EXPECT_EQ(document.FindNode(addedId), nullptr);
   EXPECT_EQ(document.GetNodes().size(), 1u);
   EXPECT_TRUE(history.CanRedo());
}

TEST(DocumentHistoryTest, RedoReappliesUndoneEdit)
{
   Cgen::GraphDocument document;
   Cgen::DocumentHistory history;
   history.PushCheckpoint(document);
   const Cgen::NodeId addedId = document.AddNode(Cgen::BlockType::End, 50.0f, 50.0f);
   ASSERT_TRUE(history.Undo(&document));
   ASSERT_TRUE(history.Redo(&document));
   EXPECT_NE(document.FindNode(addedId), nullptr);
   EXPECT_FALSE(history.CanRedo());
}

TEST(DocumentHistoryTest, PushCheckpointClearsRedoStack)
{
   Cgen::GraphDocument document;
   Cgen::DocumentHistory history;
   history.PushCheckpoint(document);
   document.AddNode(Cgen::BlockType::End, 10.0f, 10.0f);
   ASSERT_TRUE(history.Undo(&document));
   EXPECT_TRUE(history.CanRedo());

   history.PushCheckpoint(document);
   document.AddNode(Cgen::BlockType::Printf, 20.0f, 20.0f);
   EXPECT_FALSE(history.CanRedo());
}

TEST(DocumentHistoryTest, ClearEmptiesStacks)
{
   Cgen::GraphDocument document;
   Cgen::DocumentHistory history;
   history.PushCheckpoint(document);
   document.AddNode(Cgen::BlockType::End, 10.0f, 10.0f);
   history.Undo(&document);
   history.Clear();
   EXPECT_FALSE(history.CanUndo());
   EXPECT_FALSE(history.CanRedo());
}
