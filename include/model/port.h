/*!
 *\file port.h
 *\brief Port definitions for flowchart nodes.
 */
#ifndef PORT_H
#define PORT_H

#include <cstdint>
#include <string>

#include "model/c_type.h"

namespace Cgen
{
   /*!
    *\brief Whether a port carries control flow or data.
    */
   enum class PortKind: uint8_t
   {
      Control = 0,
      Data
   };

   /*!
    *\brief Port direction relative to its owning node.
    */
   enum class PortDirection: uint8_t
   {
      In = 0,
      Out
   };

   /*!
    *\brief A named connection point on a node.
    */
   struct Port
   {
      std::string name;
      PortKind kind = PortKind::Control;
      PortDirection direction = PortDirection::Out;
      CType dataType {};
      bool visible = true;
   };
} // namespace Cgen

#endif // PORT_H
