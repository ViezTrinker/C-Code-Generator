/*!
 *\file result.h
 *\brief Common result codes for the C code generator.
 */
#ifndef RESULT_H
#define RESULT_H

#include <cstdint>

namespace Cgen
{
   /*!
    *\brief Generic operation result.
    */
   enum class Result: int8_t
   {
      InvalidArgument = -5,
      NotFound = -4,
      IoError = -3,
      ParseError = -2,
      Error = -1,
      Ok = 0
   };

   /*!
    *\brief Returns true when the result encodes failure.
    *
    *\param[in] result Result value to test.
    *\return true if result is an error code.
    */
   inline bool IsErr(Result result)
   {
      return static_cast<int8_t>(result) < 0;
   }

   /*!
    *\brief Returns true when the result encodes success.
    *
    *\param[in] result Result value to test.
    *\return true if result is Ok.
    */
   inline bool IsOk(Result result)
   {
      return result == Result::Ok;
   }
} // namespace Cgen

#endif // RESULT_H
