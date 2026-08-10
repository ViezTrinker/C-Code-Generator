/*!
 *\file test_hover_tooltip.cpp
 *\brief Unit tests for palette/toolbar hover tooltip text helpers.
 */
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "gui/hover_tooltip_text.h"
#include "model/block_type.h"

TEST(HoverTooltipTest, WrapLeavesShortTextAsOneLine)
{
   std::vector<std::string> lines;
   Cgen::WrapHoverTooltipLines("Short tip.", 280.0f, &lines);
   ASSERT_EQ(lines.size(), 1u);
   EXPECT_EQ(lines[0], "Short tip.");
}

TEST(HoverTooltipTest, WrapSplitsLongTextAcrossLines)
{
   std::vector<std::string> lines;
   const std::string longText =
      "Validate the graph and generate C99 into build_out/ then compile with gcc.";
   Cgen::WrapHoverTooltipLines(longText, 120.0f, &lines);
   ASSERT_GE(lines.size(), 2u);
   std::string joined;
   for (size_t index = 0; index < lines.size(); ++index)
   {
      if (index > 0)
      {
         joined.push_back(' ');
      }
      joined.append(lines[index]);
   }
   EXPECT_NE(joined.find("Validate"), std::string::npos);
   EXPECT_NE(joined.find("gcc"), std::string::npos);
}

TEST(HoverTooltipTest, WrapIgnoresNullOutput)
{
   Cgen::WrapHoverTooltipLines("text", 100.0f, nullptr);
}

TEST(HoverTooltipTest, WrapClearsEmptyInput)
{
   std::vector<std::string> lines;
   lines.push_back("stale");
   Cgen::WrapHoverTooltipLines("", 100.0f, &lines);
   EXPECT_TRUE(lines.empty());
}

TEST(HoverTooltipTest, WrapHonorsExplicitNewlines)
{
   std::vector<std::string> lines;
   Cgen::WrapHoverTooltipLines("First line\nSecond line", 400.0f, &lines);
   ASSERT_EQ(lines.size(), 2u);
   EXPECT_EQ(lines[0], "First line");
   EXPECT_EQ(lines[1], "Second line");
}

TEST(HoverTooltipTest, PaletteBlockTipMatchesPropertiesHelp)
{
   const std::string_view tip = Cgen::PaletteRowHoverTipText(
      Cgen::PaletteRowTipKind::Block, Cgen::BlockType::For);
   EXPECT_EQ(tip, Cgen::BlockTypeHelpText(Cgen::BlockType::For));
   EXPECT_FALSE(tip.empty());
}

TEST(HoverTooltipTest, PaletteGroupTipIsExpandCollapseHint)
{
   const std::string_view tip = Cgen::PaletteRowHoverTipText(
      Cgen::PaletteRowTipKind::GroupHeader, Cgen::BlockType::Start);
   EXPECT_NE(tip.find("expand"), std::string::npos);
   EXPECT_NE(tip.find("collapse"), std::string::npos);
}

TEST(HoverTooltipTest, EveryBlockHasPaletteHoverTip)
{
   constexpr Cgen::BlockType SampleTypes[] = {
      Cgen::BlockType::Start,
      Cgen::BlockType::If,
      Cgen::BlockType::Call,
      Cgen::BlockType::FunctionDef,
      Cgen::BlockType::EnumDecl,
      Cgen::BlockType::DerefLoad,
      Cgen::BlockType::Malloc
   };
   for (size_t index = 0; index < (sizeof(SampleTypes) / sizeof(SampleTypes[0]));
        ++index)
   {
      const std::string_view tip = Cgen::PaletteRowHoverTipText(
         Cgen::PaletteRowTipKind::Block, SampleTypes[index]);
      EXPECT_FALSE(tip.empty()) << Cgen::BlockTypeToString(SampleTypes[index]);
      EXPECT_EQ(tip, Cgen::BlockTypeHelpText(SampleTypes[index]));
   }
}

TEST(HoverTooltipTest, ToolbarActionsHaveNonEmptyTips)
{
   constexpr Cgen::ToolbarAction Actions[] = {
      Cgen::ToolbarAction::NewDocument,
      Cgen::ToolbarAction::Open,
      Cgen::ToolbarAction::Save,
      Cgen::ToolbarAction::Generate,
      Cgen::ToolbarAction::Build,
      Cgen::ToolbarAction::Run,
      Cgen::ToolbarAction::Stop,
      Cgen::ToolbarAction::Tidy,
      Cgen::ToolbarAction::Snap,
      Cgen::ToolbarAction::AlignLeft,
      Cgen::ToolbarAction::AlignTop,
      Cgen::ToolbarAction::FitAll,
      Cgen::ToolbarAction::FitSelection,
      Cgen::ToolbarAction::Help
   };
   for (size_t index = 0; index < (sizeof(Actions) / sizeof(Actions[0])); ++index)
   {
      const std::string_view tip = Cgen::ToolbarActionTooltipText(Actions[index]);
      EXPECT_FALSE(tip.empty());
   }
}

TEST(HoverTooltipTest, ToolbarNoneTipIsEmpty)
{
   EXPECT_TRUE(Cgen::ToolbarActionTooltipText(Cgen::ToolbarAction::None).empty());
}
