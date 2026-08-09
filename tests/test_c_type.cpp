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

TEST(CTypeTest, ParsesNamedStructAndFileTypes)
{
   Cgen::CType hero {};
   ASSERT_TRUE(Cgen::CTypeFromString("Hero", &hero));
   EXPECT_EQ(hero.base, Cgen::PrimitiveType::Named);
   EXPECT_FALSE(hero.isPointer);
   EXPECT_EQ(hero.namedSpelling, "Hero");
   EXPECT_EQ(Cgen::CTypeToString(hero), "Hero");

   Cgen::CType filePtr {};
   ASSERT_TRUE(Cgen::CTypeFromString("FILE*", &filePtr));
   EXPECT_EQ(filePtr.base, Cgen::PrimitiveType::Named);
   EXPECT_TRUE(filePtr.isPointer);
   EXPECT_EQ(filePtr.namedSpelling, "FILE");
   EXPECT_EQ(Cgen::CTypeToString(filePtr), "FILE*");

   Cgen::CType structHero {};
   ASSERT_TRUE(Cgen::CTypeFromString("struct Hero", &structHero));
   EXPECT_EQ(structHero.namedSpelling, "Hero");
}

TEST(CTypeTest, NamedPointersMatchAndVoidPointerCompatible)
{
   Cgen::CType heroPtr {};
   ASSERT_TRUE(Cgen::CTypeFromString("Hero*", &heroPtr));
   Cgen::CType otherHeroPtr {};
   ASSERT_TRUE(Cgen::CTypeFromString("Hero*", &otherHeroPtr));
   EXPECT_TRUE(Cgen::AreTypesCompatible(heroPtr, otherHeroPtr));

   Cgen::CType pointPtr {};
   ASSERT_TRUE(Cgen::CTypeFromString("Point*", &pointPtr));
   EXPECT_FALSE(Cgen::AreTypesCompatible(heroPtr, pointPtr));

   Cgen::CType voidPtr {};
   ASSERT_TRUE(Cgen::CTypeFromString("void*", &voidPtr));
   EXPECT_TRUE(Cgen::AreTypesCompatible(voidPtr, heroPtr));
}

TEST(CTypeTest, CompatibilityRequiresSameShape)
{
   Cgen::CType left {};
   Cgen::CType right {};
   left.base = Cgen::PrimitiveType::Int32;
   right.base = Cgen::PrimitiveType::Int32;
   EXPECT_TRUE(Cgen::AreTypesCompatible(left, right));

   right.base = Cgen::PrimitiveType::Int64;
   EXPECT_TRUE(Cgen::AreTypesCompatible(left, right));

   right.base = Cgen::PrimitiveType::Float;
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

TEST(CTypeTest, ParsesBoolAsNamedType)
{
   Cgen::CType parsed {};
   ASSERT_TRUE(Cgen::CTypeFromString("bool", &parsed));
   EXPECT_EQ(parsed.base, Cgen::PrimitiveType::Named);
   EXPECT_FALSE(parsed.isPointer);
   EXPECT_EQ(parsed.namedSpelling, "bool");
   EXPECT_EQ(Cgen::CTypeToString(parsed), "bool");

   Cgen::CType pointer {};
   ASSERT_TRUE(Cgen::CTypeFromString("bool*", &pointer));
   EXPECT_TRUE(pointer.isPointer);
   EXPECT_EQ(Cgen::CTypeToString(pointer), "bool*");
}
