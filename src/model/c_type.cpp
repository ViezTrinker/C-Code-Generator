/*!
 *\file c_type.cpp
 *\brief C type helpers.
 */
#include "model/c_type.h"

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
      std::string result(PrimitiveTypeToCSpelling(cType.base));
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
      std::string_view core = text;
      if (!text.empty() && text.back() == '*')
      {
         parsed.isPointer = true;
         core = text.substr(0, text.size() - 1);
      }
      if (!PrimitiveTypeFromString(core, &parsed.base))
      {
         return false;
      }
      *pOutType = parsed;
      return true;
   }

   bool AreTypesCompatible(const CType& left, const CType& right)
   {
      if ((left.base == right.base) && (left.isPointer == right.isPointer))
      {
         return true;
      }
      if (left.isPointer && right.isPointer &&
          ((left.base == PrimitiveType::Void) || (right.base == PrimitiveType::Void)))
      {
         return true;
      }
      if ((!left.isPointer) && (!right.isPointer) &&
          ((left.base == PrimitiveType::Void) || (right.base == PrimitiveType::Void)))
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
