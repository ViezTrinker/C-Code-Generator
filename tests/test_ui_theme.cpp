/*!
 *\file test_ui_theme.cpp
 *\brief Unit tests for UI theme id helpers.
 */
#include <gtest/gtest.h>

#include "gui/ui_theme_id.h"

TEST(UiThemeIdTest, CyclesBetweenDarkAndLight)
{
   EXPECT_EQ(Cgen::CycleUiThemeId(Cgen::UiThemeId::Dark), Cgen::UiThemeId::Light);
   EXPECT_EQ(Cgen::CycleUiThemeId(Cgen::UiThemeId::Light), Cgen::UiThemeId::Dark);
}

TEST(UiThemeIdTest, RoundTripsStringIds)
{
   EXPECT_EQ(Cgen::UiThemeIdToString(Cgen::UiThemeId::Dark), "dark");
   EXPECT_EQ(Cgen::UiThemeIdToString(Cgen::UiThemeId::Light), "light");

   Cgen::UiThemeId parsed = Cgen::UiThemeId::Dark;
   ASSERT_TRUE(Cgen::UiThemeIdFromString("light", &parsed));
   EXPECT_EQ(parsed, Cgen::UiThemeId::Light);
   ASSERT_TRUE(Cgen::UiThemeIdFromString("dark", &parsed));
   EXPECT_EQ(parsed, Cgen::UiThemeId::Dark);
}

TEST(UiThemeIdTest, RejectsUnknownAndNull)
{
   Cgen::UiThemeId parsed = Cgen::UiThemeId::Dark;
   EXPECT_FALSE(Cgen::UiThemeIdFromString("purple", &parsed));
   EXPECT_FALSE(Cgen::UiThemeIdFromString("light", nullptr));
}
