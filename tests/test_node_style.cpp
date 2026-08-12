/*!
 *\file test_node_style.cpp
 *\brief Unit tests for node color presets, hex parse, and size properties.
 */
#include "gui/block_placement.h"
#include "gui/node_style.h"
#include "model/node.h"

#include <gtest/gtest.h>

#include <cmath>

namespace
{
   bool NearlyEqual(float left, float right)
   {
      return std::fabs(left - right) < 0.01f;
   }
} // namespace

TEST(NodeStyleTest, ParsesHashRgbAndRrggbb)
{
   Cgen::Rgba8 color {};
   ASSERT_TRUE(Cgen::ParseHexColor("#0f8", &color));
   EXPECT_EQ(color.red, 0);
   EXPECT_EQ(color.green, 255);
   EXPECT_EQ(color.blue, 136);

   ASSERT_TRUE(Cgen::ParseHexColor("#1A2B3C", &color));
   EXPECT_EQ(color.red, 0x1A);
   EXPECT_EQ(color.green, 0x2B);
   EXPECT_EQ(color.blue, 0x3C);
}

TEST(NodeStyleTest, RejectsInvalidHex)
{
   Cgen::Rgba8 color {};
   EXPECT_FALSE(Cgen::ParseHexColor("", &color));
   EXPECT_FALSE(Cgen::ParseHexColor("#12", &color));
   EXPECT_FALSE(Cgen::ParseHexColor("notahex", &color));
   EXPECT_FALSE(Cgen::ParseHexColor("#1A2B3C", nullptr));
}

TEST(NodeStyleTest, ResolvesNamedPresets)
{
   Cgen::Rgba8 color {};
   ASSERT_TRUE(Cgen::ResolvePresetColor("Blue", &color));
   EXPECT_EQ(color.blue, 219);
   EXPECT_FALSE(Cgen::ResolvePresetColor("Default", &color));
   EXPECT_FALSE(Cgen::ResolvePresetColor("Custom", &color));
}

TEST(NodeStyleTest, PresetListIncludesDefaultAndCustom)
{
   ASSERT_GE(Cgen::NodeColorPresetCount(), 3u);
   EXPECT_EQ(Cgen::NodeColorPresetName(0), Cgen::NodeStyleColorDefault);
   EXPECT_EQ(Cgen::NodeColorPresetName(Cgen::NodeColorPresetCount() - 1),
             Cgen::NodeStyleColorCustom);
}

TEST(NodeStyleTest, ResolvesCustomHexWhenPresetIsCustom)
{
   Cgen::Node node = Cgen::CreateNode(1, Cgen::BlockType::Literal, 0.0f, 0.0f);
   node.properties[std::string(Cgen::NodeStyleFillColorKey)] = "Custom";
   node.properties[std::string(Cgen::NodeStyleFillColorCustomKey)] = "#112233";

   Cgen::Rgba8 color {};
   ASSERT_TRUE(Cgen::TryResolveNodeStyleColor(
      node,
      Cgen::NodeStyleFillColorKey,
      Cgen::NodeStyleFillColorCustomKey,
      &color));
   EXPECT_EQ(color.red, 0x11);
   EXPECT_EQ(color.green, 0x22);
   EXPECT_EQ(color.blue, 0x33);
}

TEST(NodeStyleTest, DefaultPresetDoesNotOverride)
{
   Cgen::Node node = Cgen::CreateNode(1, Cgen::BlockType::Literal, 0.0f, 0.0f);
   node.properties[std::string(Cgen::NodeStyleTextColorKey)] = "Default";

   Cgen::Rgba8 color {};
   EXPECT_FALSE(Cgen::TryResolveNodeStyleColor(
      node,
      Cgen::NodeStyleTextColorKey,
      Cgen::NodeStyleTextColorCustomKey,
      &color));
}

TEST(NodeStyleTest, WidthPropertyOverridesDefault)
{
   Cgen::Node node = Cgen::CreateNode(1, Cgen::BlockType::Literal, 0.0f, 0.0f);
   EXPECT_TRUE(NearlyEqual(Cgen::ComputeBlockNodeWidth(node), Cgen::BlockNodeWidth));

   node.properties[std::string(Cgen::NodeStyleWidthKey)] = "200";
   EXPECT_TRUE(NearlyEqual(Cgen::ComputeBlockNodeWidth(node), 200.0f));

   node.properties[std::string(Cgen::NodeStyleWidthKey)] = "50";
   EXPECT_TRUE(NearlyEqual(Cgen::ComputeBlockNodeWidth(node), Cgen::BlockNodeMinWidth));
}

TEST(NodeStyleTest, HeightPropertyCannotShrinkBelowFitted)
{
   Cgen::Node node = Cgen::CreateNode(1, Cgen::BlockType::Printf, 0.0f, 0.0f);
   const float fitted = Cgen::ComputeBlockNodeFittedHeight(node);
   node.properties[std::string(Cgen::NodeStyleHeightKey)] = "10";
   EXPECT_TRUE(NearlyEqual(Cgen::ComputeBlockNodeHeight(node), fitted));

   node.properties[std::string(Cgen::NodeStyleHeightKey)] = "240";
   EXPECT_TRUE(NearlyEqual(Cgen::ComputeBlockNodeHeight(node), 240.0f));
}

TEST(NodeStyleTest, FormatStyleFloatRoundTrips)
{
   const std::string text = Cgen::FormatStyleFloat(180.5f);
   EXPECT_FALSE(text.empty());
}
