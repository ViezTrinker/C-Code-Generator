/*!
 *\file c_type.cpp
 *\brief C type helpers.
 */
#include "model/c_type.h"

#include <cctype>
#include <cstring>

namespace Cgen
{
   namespace
   {
      struct PrimitiveEntry
      {
         PrimitiveType type;
         const char* pId;
         const char* pSpelling;
      };

      constexpr PrimitiveEntry PrimitiveTable[] = {
         {PrimitiveType::Void, "void", "void"},
         {PrimitiveType::Char, "char", "char"},
         {PrimitiveType::Int8, "int8_t", "int8_t"},
         {PrimitiveType::Uint8, "uint8_t", "uint8_t"},
         {PrimitiveType::Int16, "int16_t", "int16_t"},
         {PrimitiveType::Uint16, "uint16_t", "uint16_t"},
         {PrimitiveType::Int32, "int32_t", "int32_t"},
         {PrimitiveType::Uint32, "uint32_t", "uint32_t"},
         {PrimitiveType::Int64, "int64_t", "int64_t"},
         {PrimitiveType::Uint64, "uint64_t", "uint64_t"},
         {PrimitiveType::Float, "float", "float"},
         {PrimitiveType::Double, "double", "double"}
      };

      constexpr size_t PrimitiveTableCount =
         sizeof(PrimitiveTable) / sizeof(PrimitiveTable[0]);

      bool IsIntegerLike(PrimitiveType primitiveType)
      {
         return (primitiveType == PrimitiveType::Char) ||
                (primitiveType == PrimitiveType::Int8) ||
                (primitiveType == PrimitiveType::Uint8) ||
                (primitiveType == PrimitiveType::Int16) ||
                (primitiveType == PrimitiveType::Uint16) ||
                (primitiveType == PrimitiveType::Int32) ||
                (primitiveType == PrimitiveType::Uint32) ||
                (primitiveType == PrimitiveType::Int64) ||
                (primitiveType == PrimitiveType::Uint64);
      }

      bool IsFloatLike(PrimitiveType primitiveType)
      {
         return (primitiveType == PrimitiveType::Float) ||
                (primitiveType == PrimitiveType::Double);
      }

      std::string_view TrimAscii(std::string_view text)
      {
         while ((!text.empty()) &&
                (std::isspace(static_cast<unsigned char>(text.front())) != 0))
         {
            text.remove_prefix(1);
         }
         while ((!text.empty()) &&
                (std::isspace(static_cast<unsigned char>(text.back())) != 0))
         {
            text.remove_suffix(1);
         }
         return text;
      }

      bool IsValidNamedTypeSpelling(std::string_view text)
      {
         if (text.empty())
         {
            return false;
         }
         const unsigned char first = static_cast<unsigned char>(text.front());
         if ((std::isalpha(first) == 0) && (text.front() != '_'))
         {
            return false;
         }
         for (size_t index = 1; index < text.size(); ++index)
         {
            const unsigned char character =
               static_cast<unsigned char>(text[index]);
            if ((std::isalnum(character) == 0) && (text[index] != '_'))
            {
               return false;
            }
         }
         return true;
      }

      bool SameNamedBase(const CType& left, const CType& right)
      {
         if ((left.base != PrimitiveType::Named) ||
             (right.base != PrimitiveType::Named))
         {
            return false;
         }
         return left.namedSpelling == right.namedSpelling;
      }
   } // namespace

   std::string_view PrimitiveTypeToCSpelling(PrimitiveType primitiveType)
   {
      for (size_t index = 0; index < PrimitiveTableCount; ++index)
      {
         if (PrimitiveTable[index].type == primitiveType)
         {
            return PrimitiveTable[index].pSpelling;
         }
      }
      return "int32_t";
   }

   std::string_view PrimitiveTypeToString(PrimitiveType primitiveType)
   {
      for (size_t index = 0; index < PrimitiveTableCount; ++index)
      {
         if (PrimitiveTable[index].type == primitiveType)
         {
            return PrimitiveTable[index].pId;
         }
      }
      if (primitiveType == PrimitiveType::Named)
      {
         return "named";
      }
      return "int32_t";
   }

   bool PrimitiveTypeFromString(std::string_view text, PrimitiveType* pOutType)
   {
      if (pOutType == nullptr)
      {
         return false;
      }
      for (size_t index = 0; index < PrimitiveTableCount; ++index)
      {
         if (text == PrimitiveTable[index].pId)
         {
            *pOutType = PrimitiveTable[index].type;
            return true;
         }
      }
      return false;
   }

   std::string CTypeToString(const CType& cType)
   {
      std::string result;
      if (cType.base == PrimitiveType::Named)
      {
         result = cType.namedSpelling.empty() ? std::string("void")
                                              : cType.namedSpelling;
      }
      else
      {
         result = std::string(PrimitiveTypeToCSpelling(cType.base));
      }
      if (cType.isPointer)
      {
         result.append("*");
      }
      return result;
   }

   bool CTypeFromString(std::string_view text, CType* pOutType)
   {
      if (pOutType == nullptr)
      {
         return false;
      }
      CType parsed {};
      std::string_view core = TrimAscii(text);
      if (core.empty())
      {
         return false;
      }
      if (core.back() == '*')
      {
         parsed.isPointer = true;
         core.remove_suffix(1);
         core = TrimAscii(core);
      }
      if (core.size() >= 7 && (core.substr(0, 7) == "struct "))
      {
         core.remove_prefix(7);
         core = TrimAscii(core);
      }
      if (PrimitiveTypeFromString(core, &parsed.base))
      {
         parsed.namedSpelling.clear();
         *pOutType = parsed;
         return true;
      }
      if (!IsValidNamedTypeSpelling(core))
      {
         return false;
      }
      parsed.base = PrimitiveType::Named;
      parsed.namedSpelling = std::string(core);
      *pOutType = parsed;
      return true;
   }

   bool AreTypesCompatible(const CType& left, const CType& right)
   {
      if (left.isPointer == right.isPointer)
      {
         if ((left.base == PrimitiveType::Named) ||
             (right.base == PrimitiveType::Named))
         {
            if (SameNamedBase(left, right))
            {
               return true;
            }
         }
         else if (left.base == right.base)
         {
            return true;
         }
      }

      if (left.isPointer && right.isPointer &&
          ((left.base == PrimitiveType::Void) || (right.base == PrimitiveType::Void)))
      {
         return true;
      }

      // Untyped Void (non-pointer) is a universal wildcard (Call args, etc.).
      if ((left.base == PrimitiveType::Void) && (!left.isPointer))
      {
         return true;
      }
      if ((right.base == PrimitiveType::Void) && (!right.isPointer))
      {
         return true;
      }

      if ((!left.isPointer) && (!right.isPointer) && IsIntegerLike(left.base) &&
          IsIntegerLike(right.base))
      {
         return true;
      }
      if ((!left.isPointer) && (!right.isPointer) && IsFloatLike(left.base) &&
          IsFloatLike(right.base))
      {
         return true;
      }
      return false;
   }
} // namespace Cgen
