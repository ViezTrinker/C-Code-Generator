/*!
 *\file test_cgen_serializer.cpp
 *\brief Unit tests for .cgen load/save round-trips.
 */
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "model/graph_document.h"
#include "serialize/cgen_serializer.h"

namespace
{
   std::filesystem::path MakeTempCgenPath(void)
   {
      const auto directory = std::filesystem::temp_directory_path();
      return directory / "cgen_unit_test_roundtrip.cgen";
   }
} // namespace

TEST(CgenSerializerTest, RoundTripsNodesAndEdges)
{
   Cgen::GraphDocument original;
   const Cgen::NodeId startId = original.GetNodes().front().id;
   const Cgen::NodeId printfId = original.AddNode(Cgen::BlockType::Printf, 120.0f, 80.0f);
   Cgen::Node* pPrintf = original.FindNodeMutable(printfId);
   ASSERT_NE(pPrintf, nullptr);
   pPrintf->properties["format"] = "hi\\n";
   const Cgen::NodeId endId = original.AddNode(Cgen::BlockType::End, 280.0f, 80.0f);
   ASSERT_TRUE(Cgen::IsOk(original.Connect(startId, "Next", printfId, "In", nullptr)));
   ASSERT_TRUE(Cgen::IsOk(original.Connect(printfId, "Next", endId, "In", nullptr)));
   original.SetViewport(12.0f, 34.0f, 0.75f);
   original.SetFilePath("demo.cgen");
   original.SetDirty(false);

   const std::filesystem::path path = MakeTempCgenPath();
   ASSERT_TRUE(Cgen::IsOk(Cgen::SaveCgenFile(original, path.string())));

   Cgen::GraphDocument loaded;
   std::string diagnostics;
   ASSERT_TRUE(Cgen::IsOk(Cgen::LoadCgenFile(path.string(), &loaded, &diagnostics)))
      << diagnostics;
   EXPECT_EQ(loaded.GetNodes().size(), original.GetNodes().size());
   EXPECT_EQ(loaded.GetEdges().size(), original.GetEdges().size());
   EXPECT_FLOAT_EQ(loaded.GetViewportX(), 12.0f);
   EXPECT_FLOAT_EQ(loaded.GetViewportY(), 34.0f);
   EXPECT_FLOAT_EQ(loaded.GetViewportZoom(), 0.75f);
   EXPECT_EQ(loaded.GetFilePath(), path.string());
   EXPECT_FALSE(loaded.IsDirty());

   const Cgen::Node* pLoadedPrintf = loaded.FindNode(printfId);
   ASSERT_NE(pLoadedPrintf, nullptr);
   EXPECT_EQ(pLoadedPrintf->properties.at("format"), "hi\\n");

   std::error_code errorCode;
   std::filesystem::remove(path, errorCode);
}

TEST(CgenSerializerTest, RejectsMissingFile)
{
   Cgen::GraphDocument document;
   std::string diagnostics;
   EXPECT_TRUE(Cgen::IsErr(
      Cgen::LoadCgenFile("definitely_missing_cgen_file_12345.cgen",
                         &document,
                         &diagnostics)));
}

TEST(CgenSerializerTest, RejectsInvalidHeader)
{
   const std::filesystem::path path =
      std::filesystem::temp_directory_path() / "cgen_unit_test_bad_header.cgen";
   {
      std::ofstream stream(path.string(), std::ios::binary);
      ASSERT_TRUE(stream.good());
      stream << "NOTCGEN\n{}\n";
   }

   Cgen::GraphDocument document;
   std::string diagnostics;
   EXPECT_TRUE(Cgen::IsErr(Cgen::LoadCgenFile(path.string(), &document, &diagnostics)));

   std::error_code errorCode;
   std::filesystem::remove(path, errorCode);
}

TEST(CgenSerializerTest, LoadsBundledExample)
{
   Cgen::GraphDocument document;
   std::string diagnostics;
   const Cgen::Result result =
      Cgen::LoadCgenFile("examples/add_two_integers.cgen", &document, &diagnostics);
   ASSERT_TRUE(Cgen::IsOk(result)) << diagnostics;
   EXPECT_GE(document.GetNodes().size(), 3u);
   EXPECT_FALSE(document.GetEdges().empty());
}
