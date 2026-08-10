/*!
 *\file test_autosave.cpp
 *\brief Unit tests for crash-recovery autosave helpers.
 */
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "model/graph_document.h"
#include "serialize/autosave.h"

TEST(AutosaveTest, SiblingPathAppendsTmp)
{
   const std::filesystem::path sibling =
      Cgen::SiblingAutosavePath("C:/graphs/dungeon.cgen");
   EXPECT_EQ(sibling.string(), "C:/graphs/dungeon.cgen.tmp");
   EXPECT_TRUE(Cgen::SiblingAutosavePath("").empty());
}

TEST(AutosaveTest, WriteLoadRoundTripRestoresOriginalPath)
{
   Cgen::GraphDocument original;
   const Cgen::NodeId startId = original.GetNodes().front().id;
   const Cgen::NodeId endId = original.AddNode(Cgen::BlockType::End, 220.0f, 40.0f);
   ASSERT_TRUE(Cgen::IsOk(original.Connect(startId, "Next", endId, "In", nullptr)));
   original.SetFilePath("project.cgen");
   original.SetDirty(true);

   const std::filesystem::path autosavePath =
      std::filesystem::temp_directory_path() / "cgen_unit_test_autosave.cgen.tmp";
   ASSERT_TRUE(Cgen::IsOk(
      Cgen::WriteAutosave(original, autosavePath, "project.cgen")));
   EXPECT_TRUE(Cgen::AutosaveExists(autosavePath));

   Cgen::GraphDocument loaded;
   std::string diagnostics;
   ASSERT_TRUE(Cgen::IsOk(Cgen::LoadAutosave(autosavePath, &loaded, &diagnostics)))
      << diagnostics;
   EXPECT_EQ(loaded.GetFilePath(), "project.cgen");
   EXPECT_TRUE(loaded.IsDirty());
   EXPECT_EQ(loaded.GetNodes().size(), original.GetNodes().size());

   ASSERT_TRUE(Cgen::IsOk(Cgen::ClearAutosave(autosavePath)));
   EXPECT_FALSE(Cgen::AutosaveExists(autosavePath));
}
