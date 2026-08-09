/*!
 *\file test_block_placement.cpp
 *\brief Unit tests for non-overlapping block placement.
 */
#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "gui/block_placement.h"
#include "model/node.h"

namespace
{
   Cgen::Node MakeNodeAt(float posX, float posY)
   {
      Cgen::Node node;
      node.id = 1;
      node.posX = posX;
      node.posY = posY;
      return node;
   }

   bool NearlyEqual(float left, float right)
   {
      return std::fabs(left - right) < 0.001f;
   }
} // namespace

TEST(BlockPlacementTest, PrefersRequestedPositionWhenEmpty)
{
   const std::vector<Cgen::Node> nodes;
   Cgen::WorldPosition preferred;
   preferred.x = 100.0f;
   preferred.y = 200.0f;

   const Cgen::WorldPosition free =
      Cgen::FindFreeBlockWorldPosition(preferred, nodes);
   EXPECT_TRUE(NearlyEqual(free.x, preferred.x));
   EXPECT_TRUE(NearlyEqual(free.y, preferred.y));
}

TEST(BlockPlacementTest, DetectsOverlapAtSamePosition)
{
   std::vector<Cgen::Node> nodes;
   nodes.push_back(MakeNodeAt(50.0f, 50.0f));

   Cgen::WorldPosition sameSpot;
   sameSpot.x = 50.0f;
   sameSpot.y = 50.0f;
   EXPECT_TRUE(Cgen::BlockPlacementOverlapsExisting(sameSpot, nodes));
}

TEST(BlockPlacementTest, DoesNotTreatFarPositionAsOverlap)
{
   std::vector<Cgen::Node> nodes;
   nodes.push_back(MakeNodeAt(50.0f, 50.0f));

   Cgen::WorldPosition farAway;
   farAway.x = 50.0f + Cgen::BlockNodeWidth + Cgen::BlockPlacementGap;
   farAway.y = 50.0f;
   EXPECT_FALSE(Cgen::BlockPlacementOverlapsExisting(farAway, nodes));
}

TEST(BlockPlacementTest, ShiftsRightWhenPreferredIsOccupied)
{
   std::vector<Cgen::Node> nodes;
   nodes.push_back(MakeNodeAt(100.0f, 100.0f));

   Cgen::WorldPosition preferred;
   preferred.x = 100.0f;
   preferred.y = 100.0f;

   const Cgen::WorldPosition free =
      Cgen::FindFreeBlockWorldPosition(preferred, nodes);
   EXPECT_FALSE(Cgen::BlockPlacementOverlapsExisting(free, nodes));
   EXPECT_FALSE(NearlyEqual(free.x, preferred.x) && NearlyEqual(free.y, preferred.y));
   EXPECT_TRUE(NearlyEqual(free.y, preferred.y));
   EXPECT_TRUE(NearlyEqual(free.x, preferred.x + Cgen::BlockNodeWidth + Cgen::BlockPlacementGap));
}

TEST(BlockPlacementTest, RepeatedSamePreferredYieldsDistinctSlots)
{
   std::vector<Cgen::Node> nodes;
   Cgen::WorldPosition preferred;
   preferred.x = 80.0f;
   preferred.y = 80.0f;

   constexpr uint32_t PlaceCount = 5;
   for (uint32_t index = 0; index < PlaceCount; ++index)
   {
      const Cgen::WorldPosition free =
         Cgen::FindFreeBlockWorldPosition(preferred, nodes);
      EXPECT_FALSE(Cgen::BlockPlacementOverlapsExisting(free, nodes));
      nodes.push_back(MakeNodeAt(free.x, free.y));
   }

   for (size_t left = 0; left < nodes.size(); ++left)
   {
      for (size_t right = left + 1; right < nodes.size(); ++right)
      {
         const bool sameX = NearlyEqual(nodes[left].posX, nodes[right].posX);
         const bool sameY = NearlyEqual(nodes[left].posY, nodes[right].posY);
         EXPECT_FALSE(sameX && sameY);
      }
   }
}

TEST(BlockPlacementTest, PrintfHeightFitsAllInputPorts)
{
   const Cgen::Node printfNode = Cgen::CreateNode(1, Cgen::BlockType::Printf, 0.0f, 0.0f);
   size_t inCount = 0;
   for (size_t index = 0; index < printfNode.ports.size(); ++index)
   {
      if (printfNode.ports[index].direction == Cgen::PortDirection::In)
      {
         ++inCount;
      }
   }
   ASSERT_GE(inCount, 7u);

   const float height = Cgen::ComputeBlockNodeHeight(printfNode);
   const float lastPortY =
      Cgen::BlockPortTopOffset +
      (static_cast<float>(inCount - 1) * Cgen::BlockPortSpacing);
   EXPECT_GT(height, lastPortY);
   EXPECT_GE(height, Cgen::BlockNodeHeight);
}
