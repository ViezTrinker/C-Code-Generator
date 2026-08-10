/*!
 *\file test_http_url.cpp
 *\brief Unit tests for http(s) URL extraction.
 */
#include "gui/http_url.h"

#include <gtest/gtest.h>

TEST(HttpUrlTest, ExtractsHttpsFromLabeledLine)
{
   std::string url;
   ASSERT_TRUE(Cgen::ExtractHttpUrlFromLine(
      "GitHub profile: https://github.com/ViezTrinker", &url));
   EXPECT_EQ(url, "https://github.com/ViezTrinker");
}

TEST(HttpUrlTest, ExtractsHttpWhenNoHttps)
{
   std::string url;
   ASSERT_TRUE(Cgen::ExtractHttpUrlFromLine(
      "See http://example.com/path", &url));
   EXPECT_EQ(url, "http://example.com/path");
}

TEST(HttpUrlTest, PrefersEarlierSchemeOccurrence)
{
   std::string url;
   ASSERT_TRUE(Cgen::ExtractHttpUrlFromLine(
      "http://a.example https://b.example", &url));
   EXPECT_EQ(url, "http://a.example");
}

TEST(HttpUrlTest, TrimsTrailingPunctuation)
{
   std::string url;
   ASSERT_TRUE(Cgen::ExtractHttpUrlFromLine(
      "Repo: https://github.com/ViezTrinker/Graphical-C-Code-Generator.", &url));
   EXPECT_EQ(url, "https://github.com/ViezTrinker/Graphical-C-Code-Generator");
}

TEST(HttpUrlTest, ReturnsFalseWithoutUrl)
{
   std::string url = "stale";
   EXPECT_FALSE(Cgen::ExtractHttpUrlFromLine("Author: ViezTrinker", &url));
   EXPECT_TRUE(url.empty());
}

TEST(HttpUrlTest, RejectsNullOutput)
{
   EXPECT_FALSE(Cgen::ExtractHttpUrlFromLine("https://example.com", nullptr));
}
