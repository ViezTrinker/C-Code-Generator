/*!
 *\file test_block_type.cpp
 *\brief Unit tests for block type string mapping.
 */
#include <gtest/gtest.h>

#include "model/block_type.h"

TEST(BlockTypeTest, RoundTripsKnownIds)
{
   Cgen::BlockType parsed = Cgen::BlockType::Start;
   ASSERT_TRUE(Cgen::BlockTypeFromString("ScanfChar", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::ScanfChar);
   EXPECT_EQ(Cgen::BlockTypeToString(parsed), "ScanfChar");
   EXPECT_FALSE(Cgen::BlockTypeLabel(parsed).empty());

   ASSERT_TRUE(Cgen::BlockTypeFromString("ElseIf", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::ElseIf);
   ASSERT_TRUE(Cgen::BlockTypeFromString("CompoundAssign", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::CompoundAssign);
}

TEST(BlockTypeTest, RejectsUnknownId)
{
   Cgen::BlockType parsed = Cgen::BlockType::Start;
   EXPECT_FALSE(Cgen::BlockTypeFromString("NotABlock", &parsed));
}

TEST(BlockTypeTest, ExpressionClassification)
{
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Literal));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Add));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::And));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Not));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Neg));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::StrLen));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Random));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Printf));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::If));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::ElseIf));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Break));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::ScanfInt));
}

TEST(BlockTypeTest, EveryBlockHasHelpText)
{
   constexpr Cgen::BlockType AllTypes[] = {
      Cgen::BlockType::Start,
      Cgen::BlockType::End,
      Cgen::BlockType::If,
      Cgen::BlockType::ElseIf,
      Cgen::BlockType::Switch,
      Cgen::BlockType::Case,
      Cgen::BlockType::While,
      Cgen::BlockType::For,
      Cgen::BlockType::Break,
      Cgen::BlockType::Continue,
      Cgen::BlockType::Literal,
      Cgen::BlockType::VariableDecl,
      Cgen::BlockType::GlobalDecl,
      Cgen::BlockType::VariableRef,
      Cgen::BlockType::Assign,
      Cgen::BlockType::CompoundAssign,
      Cgen::BlockType::Inc,
      Cgen::BlockType::Dec,
      Cgen::BlockType::Add,
      Cgen::BlockType::Sub,
      Cgen::BlockType::Mul,
      Cgen::BlockType::Div,
      Cgen::BlockType::Mod,
      Cgen::BlockType::Neg,
      Cgen::BlockType::Equal,
      Cgen::BlockType::NotEqual,
      Cgen::BlockType::Less,
      Cgen::BlockType::LessEqual,
      Cgen::BlockType::Greater,
      Cgen::BlockType::GreaterEqual,
      Cgen::BlockType::And,
      Cgen::BlockType::Or,
      Cgen::BlockType::Not,
      Cgen::BlockType::Printf,
      Cgen::BlockType::WaitEnter,
      Cgen::BlockType::ScanfInt,
      Cgen::BlockType::ScanfChar,
      Cgen::BlockType::ScanfLine,
      Cgen::BlockType::ArrayDecl,
      Cgen::BlockType::IndexAssign,
      Cgen::BlockType::IndexLoad,
      Cgen::BlockType::StrLen,
      Cgen::BlockType::StrCpy,
      Cgen::BlockType::StrNCpy,
      Cgen::BlockType::StrCmp,
      Cgen::BlockType::RandomChar,
      Cgen::BlockType::ShuffleArray,
      Cgen::BlockType::Malloc,
      Cgen::BlockType::Free,
      Cgen::BlockType::TimeNow,
      Cgen::BlockType::LocalTime,
      Cgen::BlockType::Sleep,
      Cgen::BlockType::Random,
      Cgen::BlockType::FunctionDef,
      Cgen::BlockType::Return,
      Cgen::BlockType::Call
   };

   for (size_t index = 0; index < (sizeof(AllTypes) / sizeof(AllTypes[0])); ++index)
   {
      const std::string_view help = Cgen::BlockTypeHelpText(AllTypes[index]);
      EXPECT_FALSE(help.empty()) << Cgen::BlockTypeToString(AllTypes[index]);
   }
}
