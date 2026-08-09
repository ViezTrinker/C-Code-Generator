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
   ASSERT_TRUE(Cgen::BlockTypeFromString("Cast", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::Cast);
   ASSERT_TRUE(Cgen::BlockTypeFromString("StructDecl", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::StructDecl);
   ASSERT_TRUE(Cgen::BlockTypeFromString("EnumDecl", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::EnumDecl);
   ASSERT_TRUE(Cgen::BlockTypeFromString("TypedefDecl", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::TypedefDecl);
   ASSERT_TRUE(Cgen::BlockTypeFromString("StructLiteral", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::StructLiteral);
   ASSERT_TRUE(Cgen::BlockTypeFromString("AddressOf", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::AddressOf);
   ASSERT_TRUE(Cgen::BlockTypeFromString("DerefLoad", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::DerefLoad);
   ASSERT_TRUE(Cgen::BlockTypeFromString("DerefStore", &parsed));
   EXPECT_EQ(parsed, Cgen::BlockType::DerefStore);
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
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Cast));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::FieldLoad));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::StructLiteral));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::AddressOf));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::DerefLoad));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::StrLen));
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Random));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Printf));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::If));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::ElseIf));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Break));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::ScanfInt));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::StructDecl));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::EnumDecl));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::TypedefDecl));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::DerefStore));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Assert));
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
      Cgen::BlockType::AddressOf,
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
      Cgen::BlockType::Cast,
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
      Cgen::BlockType::ScanfFloat,
      Cgen::BlockType::ScanfLine,
      Cgen::BlockType::ArrayDecl,
      Cgen::BlockType::IndexAssign,
      Cgen::BlockType::IndexLoad,
      Cgen::BlockType::StrLen,
      Cgen::BlockType::StrCpy,
      Cgen::BlockType::StrNCpy,
      Cgen::BlockType::StrCmp,
      Cgen::BlockType::FileOpen,
      Cgen::BlockType::FileRead,
      Cgen::BlockType::FileWrite,
      Cgen::BlockType::FileClose,
      Cgen::BlockType::FilePrintf,
      Cgen::BlockType::FileGets,
      Cgen::BlockType::Assert,
      Cgen::BlockType::Comment,
      Cgen::BlockType::StructDecl,
      Cgen::BlockType::EnumDecl,
      Cgen::BlockType::TypedefDecl,
      Cgen::BlockType::FieldLoad,
      Cgen::BlockType::FieldStore,
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
      Cgen::BlockType::Call,
      Cgen::BlockType::StructLiteral,
      Cgen::BlockType::DerefLoad,
      Cgen::BlockType::DerefStore
   };

   for (size_t index = 0; index < (sizeof(AllTypes) / sizeof(AllTypes[0])); ++index)
   {
      const std::string_view help = Cgen::BlockTypeHelpText(AllTypes[index]);
      EXPECT_FALSE(help.empty()) << Cgen::BlockTypeToString(AllTypes[index]);
   }
}
