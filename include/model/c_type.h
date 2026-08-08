/*!
 *\file c_type.h
 *\brief C type representation for ports and declarations.
 */
#ifndef C_TYPE_H
#define C_TYPE_H

#include <cstdint>
#include <string>
#include <string_view>

namespace Cgen
{
   /*!
    *\brief Primitive C types supported in the MVP.
    */
   enum class PrimitiveType: uint8_t
   {
      Void = 0,
      Char,
      Int8,
      Uint8,
      Int16,
      Uint16,
      Int32,
      Uint32,
      Int64,
      Uint64,
      Float,
      Double
   };

   /*!
    *\brief C type with optional single-level pointer.
    */
   struct CType
   {
      PrimitiveType base = PrimitiveType::Int32;
      bool isPointer = false;
   };

   /*!
    *\brief Converts a primitive type to a C spelling.
    *
    *\param[in] primitiveType Primitive type.
    *\return C type spelling without pointer star.
    */
   std::string_view PrimitiveTypeToCSpelling(PrimitiveType primitiveType);

   /*!
    *\brief Converts a primitive type to a stable string id.
    *
    *\param[in] primitiveType Primitive type.
    *\return String identifier.
    */
   std::string_view PrimitiveTypeToString(PrimitiveType primitiveType);

   /*!
    *\brief Parses a primitive type from a string id.
    *
    *\param[in] text String identifier.
    *\param[out] pOutType Parsed type on success.
    *\return true if parsing succeeded.
    */
   bool PrimitiveTypeFromString(std::string_view text, PrimitiveType* pOutType);

   /*!
    *\brief Renders a full C type including pointer star.
    *
    *\param[in] cType Type to render.
    *\return C type string.
    */
   std::string CTypeToString(const CType& cType);

   /*!
    *\brief Parses a CType from "uint32_t" or "uint32_t*".
    *
    *\param[in] text Type text.
    *\param[out] pOutType Parsed type on success.
    *\return true if parsing succeeded.
    */
   bool CTypeFromString(std::string_view text, CType* pOutType);

   /*!
    *\brief Returns true if two types are compatible for wiring.
    *
    *\param[in] left Left type.
    *\param[in] right Right type.
    *\return true if types match.
    */
   bool AreTypesCompatible(const CType& left, const CType& right);
} // namespace Cgen

#endif // C_TYPE_H
