/*!
 *\file test_c_type.cpp
 *\brief Unit tests for C type helpers.
 */
#include <gtest/gtest.h>

#include "model/c_type.h"

TEST(CTypeTest, PrimitiveRoundTrip)
{
   Cgen::PrimitiveType parsed = Cgen::PrimitiveType::Void;
   ASSERT_TRUE(Cgen::PrimitiveTypeFromString("int32_t", &parsed));
   EXPECT_EQ(parsed, Cgen::PrimitiveType::Int32);
   EXPECT_EQ(Cgen::PrimitiveTypeToString(parsed), "int32_t");
   EXPECT_EQ(Cgen::PrimitiveTypeToCSpelling(parsed), "int32_t");
}

TEST(CTypeTest, RejectsUnknownPrimitive)
{
   Cgen::PrimitiveType parsed = Cgen::PrimitiveType::Void;
   EXPECT_FALSE(Cgen::PrimitiveTypeFromString("not_a_type", &parsed));
}

TEST(CTypeTest, ParsesPointerTypes)
{
   Cgen::CType parsed {};
   ASSERT_TRUE(Cgen::CTypeFromString("uint8_t*", &parsed));
   EXPECT_EQ(parsed.base, Cgen::PrimitiveType::Uint8);
   EXPECT_TRUE(parsed.isPointer);
   EXPECT_EQ(Cgen::CTypeToString(parsed), "uint8_t*");
}

TEST(CTypeTest, CompatibilityRequiresSameShape)
{
   Cgen::CType left {};
   Cgen::CType right {};
   left.base = Cgen::PrimitiveType::Int32;
   right.base = Cgen::PrimitiveType::Int32;
   EXPECT_TRUE(Cgen::AreTypesCompatible(left, right));

   right.base = Cgen::PrimitiveType::Int64;
   EXPECT_FALSE(Cgen::AreTypesCompatible(left, right));

   right.base = Cgen::PrimitiveType::Int32;
   right.isPointer = true;
   EXPECT_FALSE(Cgen::AreTypesCompatible(left, right));
}

TEST(CTypeTest, VoidPointerCompatibleWithTypedPointer)
{
   Cgen::CType voidPtr {};
   voidPtr.base = Cgen::PrimitiveType::Void;
   voidPtr.isPointer = true;
   Cgen::CType uint8Ptr {};
   uint8Ptr.base = Cgen::PrimitiveType::Uint8;
   uint8Ptr.isPointer = true;
   EXPECT_TRUE(Cgen::AreTypesCompatible(voidPtr, uint8Ptr));
   EXPECT_TRUE(Cgen::AreTypesCompatible(uint8Ptr, voidPtr));
}

TEST(CTypeTest, VoidNonPointerActsAsUniversalNonPointer)
{
   Cgen::CType anyValue {};
   anyValue.base = Cgen::PrimitiveType::Void;
   anyValue.isPointer = false;
   Cgen::CType intValue {};
   intValue.base = Cgen::PrimitiveType::Int32;
   EXPECT_TRUE(Cgen::AreTypesCompatible(anyValue, intValue));
}
