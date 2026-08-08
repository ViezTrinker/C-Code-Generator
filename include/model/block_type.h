/*!
 *\file block_type.h
 *\brief Block type enumeration for flowchart nodes.
 */
#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

#include <cstdint>
#include <string>
#include <string_view>

namespace Cgen
{
   /*!
    *\brief Supported MVP flowchart block kinds.
    */
   enum class BlockType: uint8_t
   {
      Start = 0,
      End,
      If,
      While,
      For,
      Literal,
      VariableDecl,
      GlobalDecl,
      VariableRef,
      Assign,
      Add,
      Sub,
      Mul,
      Div,
      Mod,
      Equal,
      NotEqual,
      Less,
      LessEqual,
      Greater,
      GreaterEqual,
      Printf,
      WaitEnter,
      ScanfInt,
      ArrayDecl,
      IndexAssign,
      IndexLoad,
      RandomChar,
      ShuffleArray,
      Malloc,
      Free,
      TimeNow,
      LocalTime,
      Sleep,
      Random,
      FunctionDef,
      Return,
      Call
   };

   /*!
    *\brief Converts a block type to a stable string id.
    *
    *\param[in] blockType Block type.
    *\return String identifier used in .cgen files.
    */
   std::string_view BlockTypeToString(BlockType blockType);

   /*!
    *\brief Parses a block type from a string id.
    *
    *\param[in] text String identifier.
    *\param[out] pOutBlockType Parsed type on success.
    *\return true if parsing succeeded.
    */
   bool BlockTypeFromString(std::string_view text, BlockType* pOutBlockType);

   /*!
    *\brief Human-readable label for the palette and canvas.
    *
    *\param[in] blockType Block type.
    *\return Display label.
    */
   std::string_view BlockTypeLabel(BlockType blockType);

   /*!
    *\brief Returns true if the block is a pure expression (no control flow).
    *
    *\param[in] blockType Block type.
    *\return true for expression blocks.
    */
   bool IsExpressionBlock(BlockType blockType);
} // namespace Cgen

#endif // BLOCK_TYPE_H
