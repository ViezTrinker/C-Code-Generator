/*!
 *\file toolbar_action.h
 *\brief Toolbar action identifiers shared by UI and tooltip text.
 */
#ifndef TOOLBAR_ACTION_H
#define TOOLBAR_ACTION_H

#include <cstdint>

namespace Cgen
{
   /*!
    *\brief Toolbar action identifiers.
    */
   enum class ToolbarAction: uint8_t
   {
      None = 0,
      NewDocument,
      Open,
      Save,
      Generate,
      Build,
      Run,
      Stop,
      Tidy,
      Theme,
      About,
      Help,
      FitAll,
      FitSelection,
      Snap,
      AlignLeft,
      AlignTop,
      OrthogonalWires
   };
} // namespace Cgen

#endif // TOOLBAR_ACTION_H
