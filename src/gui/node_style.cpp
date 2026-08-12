/*!
 *\file node_style.cpp
 *\brief Per-node size and color style helpers.
 */
#include "gui/node_style.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace Cgen
{
   namespace
   {
      struct NamedPreset
      {
         const char* pName = nullptr;
         uint8_t red = 0;
         uint8_t green = 0;
         uint8_t blue = 0;
      };

      constexpr NamedPreset ConcretePresets[] = {
         {"Red", 198, 40, 40},
         {"Orange", 230, 126, 34},
         {"Yellow", 241, 196, 15},
         {"Green", 39, 174, 96},
         {"Teal", 26, 188, 156},
         {"Blue", 52, 152, 219},
         {"Purple", 155, 89, 182},
         {"Pink", 233, 30, 99},
         {"Gray", 127, 140, 141},
         {"White", 245, 245, 245},
         {"Black", 30, 30, 30},
      };

      constexpr size_t ConcretePresetCount =
         sizeof(ConcretePresets) / sizeof(ConcretePresets[0]);

      int HexDigitValue(char character)
      {
         if ((character >= '0') && (character <= '9'))
         {
            return character - '0';
         }
         if ((character >= 'a') && (character <= 'f'))
         {
            return 10 + (character - 'a');
         }
         if ((character >= 'A') && (character <= 'F'))
         {
            return 10 + (character - 'A');
         }
         return -1;
      }

      const std::string* FindProperty(const Node& node, std::string_view key)
      {
         const auto found = node.properties.find(std::string(key));
         if (found == node.properties.end())
         {
            return nullptr;
         }
         return &found->second;
      }

      bool EqualsIgnoreCase(std::string_view left, std::string_view right)
      {
         if (left.size() != right.size())
         {
            return false;
         }
         for (size_t index = 0; index < left.size(); ++index)
         {
            const auto leftChar =
               static_cast<unsigned char>(left[index]);
            const auto rightChar =
               static_cast<unsigned char>(right[index]);
            if (std::tolower(leftChar) != std::tolower(rightChar))
            {
               return false;
            }
         }
         return true;
      }
   } // namespace

   bool ParseHexColor(std::string_view text, Rgba8* pOutColor)
   {
      if (pOutColor == nullptr)
      {
         return false;
      }
      if (text.empty())
      {
         return false;
      }
      size_t start = 0;
      if (text[0] == '#')
      {
         start = 1;
      }
      const size_t length = text.size() - start;
      if ((length != 3) && (length != 6))
      {
         return false;
      }

      uint8_t red = 0;
      uint8_t green = 0;
      uint8_t blue = 0;
      if (length == 3)
      {
         const int digit0 = HexDigitValue(text[start]);
         const int digit1 = HexDigitValue(text[start + 1]);
         const int digit2 = HexDigitValue(text[start + 2]);
         if ((digit0 < 0) || (digit1 < 0) || (digit2 < 0))
         {
            return false;
         }
         red = static_cast<uint8_t>((digit0 << 4) | digit0);
         green = static_cast<uint8_t>((digit1 << 4) | digit1);
         blue = static_cast<uint8_t>((digit2 << 4) | digit2);
      }
      else
      {
         const int digit0 = HexDigitValue(text[start]);
         const int digit1 = HexDigitValue(text[start + 1]);
         const int digit2 = HexDigitValue(text[start + 2]);
         const int digit3 = HexDigitValue(text[start + 3]);
         const int digit4 = HexDigitValue(text[start + 4]);
         const int digit5 = HexDigitValue(text[start + 5]);
         if ((digit0 < 0) || (digit1 < 0) || (digit2 < 0) || (digit3 < 0) ||
             (digit4 < 0) || (digit5 < 0))
         {
            return false;
         }
         red = static_cast<uint8_t>((digit0 << 4) | digit1);
         green = static_cast<uint8_t>((digit2 << 4) | digit3);
         blue = static_cast<uint8_t>((digit4 << 4) | digit5);
      }

      pOutColor->red = red;
      pOutColor->green = green;
      pOutColor->blue = blue;
      pOutColor->alpha = 255;
      return true;
   }

   size_t NodeColorPresetCount(void)
   {
      return ConcretePresetCount + 2;
   }

   std::string_view NodeColorPresetName(size_t index)
   {
      if (index == 0)
      {
         return NodeStyleColorDefault;
      }
      if (index == (ConcretePresetCount + 1))
      {
         return NodeStyleColorCustom;
      }
      if (index <= ConcretePresetCount)
      {
         return ConcretePresets[index - 1].pName;
      }
      return NodeStyleColorDefault;
   }

   bool ResolvePresetColor(std::string_view name, Rgba8* pOutColor)
   {
      if (pOutColor == nullptr)
      {
         return false;
      }
      if (name.empty() || EqualsIgnoreCase(name, NodeStyleColorDefault) ||
          EqualsIgnoreCase(name, NodeStyleColorCustom))
      {
         return false;
      }
      for (size_t index = 0; index < ConcretePresetCount; ++index)
      {
         if (EqualsIgnoreCase(name, ConcretePresets[index].pName))
         {
            pOutColor->red = ConcretePresets[index].red;
            pOutColor->green = ConcretePresets[index].green;
            pOutColor->blue = ConcretePresets[index].blue;
            pOutColor->alpha = 255;
            return true;
         }
      }
      return false;
   }

   bool TryResolveNodeStyleColor(const Node& node,
                                 std::string_view presetKey,
                                 std::string_view customKey,
                                 Rgba8* pOutColor)
   {
      if (pOutColor == nullptr)
      {
         return false;
      }
      const std::string* pPreset = FindProperty(node, presetKey);
      if (pPreset == nullptr)
      {
         return false;
      }
      if (pPreset->empty() || EqualsIgnoreCase(*pPreset, NodeStyleColorDefault))
      {
         return false;
      }
      if (EqualsIgnoreCase(*pPreset, NodeStyleColorCustom))
      {
         const std::string* pCustom = FindProperty(node, customKey);
         if ((pCustom == nullptr) || pCustom->empty())
         {
            return false;
         }
         return ParseHexColor(*pCustom, pOutColor);
      }
      return ResolvePresetColor(*pPreset, pOutColor);
   }

   bool TryReadFloatProperty(const Node& node,
                             std::string_view key,
                             float* pOutValue)
   {
      if (pOutValue == nullptr)
      {
         return false;
      }
      const std::string* pValue = FindProperty(node, key);
      if ((pValue == nullptr) || pValue->empty())
      {
         return false;
      }
      char* pEnd = nullptr;
      const float parsed = std::strtof(pValue->c_str(), &pEnd);
      if ((pEnd == pValue->c_str()) || (!std::isfinite(parsed)))
      {
         return false;
      }
      *pOutValue = parsed;
      return true;
   }

   float ReadClampedFloatProperty(const Node& node,
                                  std::string_view key,
                                  float defaultValue,
                                  float minValue,
                                  float maxValue)
   {
      float value = defaultValue;
      if (!TryReadFloatProperty(node, key, &value))
      {
         return defaultValue;
      }
      if (value < minValue)
      {
         return minValue;
      }
      if (value > maxValue)
      {
         return maxValue;
      }
      return value;
   }

   std::string FormatStyleFloat(float value)
   {
      char buffer[64] = {};
      std::snprintf(buffer, sizeof(buffer), "%.1f", static_cast<double>(value));
      return std::string(buffer);
   }

   bool IsNodeStylePropertyKey(std::string_view key)
   {
      return (key == NodeStyleWidthKey) || (key == NodeStyleHeightKey) ||
             (key == NodeStyleFillColorKey) || (key == NodeStyleTextColorKey) ||
             (key == NodeStyleFillColorCustomKey) ||
             (key == NodeStyleTextColorCustomKey);
   }
} // namespace Cgen
