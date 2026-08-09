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
    *\brief Supported flowchart block kinds.
    */
   enum class BlockType: uint8_t
   {
      Start = 0,
      End,
      If,
      ElseIf,
      Switch,
      Case,
      While,
      For,
      Break,
      Continue,
      Literal,
      VariableDecl,
      GlobalDecl,
      VariableRef,
      Assign,
      CompoundAssign,
      Inc,
      Dec,
      Add,
      Sub,
      Mul,
      Div,
      Mod,
      Neg,
      Equal,
      NotEqual,
      Less,
      LessEqual,
      Greater,
      GreaterEqual,
      And,
      Or,
      Not,
      Printf,
      WaitEnter,
      ScanfInt,
      ScanfChar,
      ScanfLine,
      ArrayDecl,
      IndexAssign,
      IndexLoad,
      StrLen,
      StrCpy,
      StrNCpy,
      StrCmp,
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
    *\brief Short help text shown in the Properties panel.
    *
    *\param[in] blockType Block type.
    *\return Help description for the block.
    */
   std::string_view BlockTypeHelpText(BlockType blockType);

   /*!
    *\brief Returns true if the block is a pure expression (no control flow).
    *
    *\param[in] blockType Block type.
    *\return true for expression blocks.
    */
   bool IsExpressionBlock(BlockType blockType);
} // namespace Cgen

#endif // BLOCK_TYPE_H
