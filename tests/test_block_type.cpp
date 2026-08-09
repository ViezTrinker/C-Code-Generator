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
   EXPECT_TRUE(Cgen::IsExpressionBlock(Cgen::BlockType::Random));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::Printf));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::If));
   EXPECT_FALSE(Cgen::IsExpressionBlock(Cgen::BlockType::ScanfInt));
}
