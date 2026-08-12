/*!
 *\file node_style.h
 *\brief Per-node size and color style helpers (properties + presets).
 */
#ifndef NODE_STYLE_H
#define NODE_STYLE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "model/node.h"

namespace Cgen
{
   inline constexpr float BlockNodeMinWidth = 100.0f;
   inline constexpr float BlockNodeMaxWidth = 480.0f;
   inline constexpr float BlockNodeMinHeightOverride = 64.0f;
   inline constexpr float BlockNodeMaxHeight = 640.0f;

   inline constexpr std::string_view NodeStyleWidthKey = "width";
   inline constexpr std::string_view NodeStyleHeightKey = "height";
   inline constexpr std::string_view NodeStyleFillColorKey = "fillColor";
   inline constexpr std::string_view NodeStyleTextColorKey = "textColor";
   inline constexpr std::string_view NodeStyleFillColorCustomKey = "fillColorCustom";
   inline constexpr std::string_view NodeStyleTextColorCustomKey = "textColorCustom";
   inline constexpr std::string_view NodeStyleColorDefault = "Default";
   inline constexpr std::string_view NodeStyleColorCustom = "Custom";

   /*!
    *\brief 8-bit RGBA color without SFML dependency.
    */
   struct Rgba8
   {
      uint8_t red = 0;
      uint8_t green = 0;
      uint8_t blue = 0;
      uint8_t alpha = 255;
   };

   /*!
    *\brief Parses #RGB or #RRGGBB into an opaque color.
    *
    *\param[in] text Hex string.
    *\param[out] pOutColor Receives the color when parsing succeeds.
    *\return true when parsing succeeds.
    */
   bool ParseHexColor(std::string_view text, Rgba8* pOutColor);

   /*!
    *\brief Returns the number of named color presets (including Default and Custom).
    */
   size_t NodeColorPresetCount(void);

   /*!
    *\brief Returns a preset name by index.
    *
    *\param[in] index Preset index in [0, NodeColorPresetCount()).
    */
   std::string_view NodeColorPresetName(size_t index);

   /*!
    *\brief Resolves a named preset (not Default/Custom) to RGB.
    *
    *\param[in] name Preset name.
    *\param[out] pOutColor Receives the color when known.
    *\return true when name is a concrete preset color.
    */
   bool ResolvePresetColor(std::string_view name, Rgba8* pOutColor);

   /*!
    *\brief Resolves fill or text color from node properties.
    *
    *\param[in] node Node with optional style properties.
    *\param[in] presetKey Property key for the preset name.
    *\param[in] customKey Property key for custom hex.
    *\param[out] pOutColor Receives the override color when present.
    *\return true when an override should replace the theme default.
    */
   bool TryResolveNodeStyleColor(const Node& node,
                                 std::string_view presetKey,
                                 std::string_view customKey,
                                 Rgba8* pOutColor);

   /*!
    *\brief Reads an optional float property with clamp.
    *
    *\param[in] node Node properties.
    *\param[in] key Property key.
    *\param[in] defaultValue Value when missing/invalid.
    *\param[in] minValue Inclusive minimum.
    *\param[in] maxValue Inclusive maximum.
    */
   float ReadClampedFloatProperty(const Node& node,
                                  std::string_view key,
                                  float defaultValue,
                                  float minValue,
                                  float maxValue);

   /*!
    *\brief Returns true when the node stores an explicit float override for key.
    *
    *\param[in] node Node properties.
    *\param[in] key Property key.
    *\param[out] pOutValue Parsed value when present and valid.
    */
   bool TryReadFloatProperty(const Node& node,
                             std::string_view key,
                             float* pOutValue);

   /*!
    *\brief Formats a float for storage in a node property.
    *
    *\param[in] value Numeric value.
    */
   std::string FormatStyleFloat(float value);

   /*!
    *\brief Returns true when key is a visual style property.
    *
    *\param[in] key Property key.
    */
   bool IsNodeStylePropertyKey(std::string_view key);
} // namespace Cgen

#endif // NODE_STYLE_H
