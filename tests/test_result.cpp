/*!
 *\file test_result.cpp
 *\brief Unit tests for Result helpers.
 */
#include <gtest/gtest.h>

#include "model/result.h"

TEST(ResultTest, IsOkRecognizesSuccess)
{
   EXPECT_TRUE(Cgen::IsOk(Cgen::Result::Ok));
   EXPECT_FALSE(Cgen::IsErr(Cgen::Result::Ok));
}

TEST(ResultTest, IsErrRecognizesFailures)
{
   EXPECT_TRUE(Cgen::IsErr(Cgen::Result::Error));
   EXPECT_TRUE(Cgen::IsErr(Cgen::Result::ParseError));
   EXPECT_TRUE(Cgen::IsErr(Cgen::Result::IoError));
   EXPECT_TRUE(Cgen::IsErr(Cgen::Result::NotFound));
   EXPECT_TRUE(Cgen::IsErr(Cgen::Result::InvalidArgument));
   EXPECT_FALSE(Cgen::IsOk(Cgen::Result::Error));
}
