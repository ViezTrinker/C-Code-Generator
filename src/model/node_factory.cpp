/*!
 *\file node_factory.cpp
 *\brief Block type strings and node factory.
 */
#include "model/block_type.h"
#include "model/graph_document.h"
#include "model/node.h"

#include <string>
#include <utility>

namespace Cgen
{
   namespace
   {
      struct BlockEntry
      {
         BlockType type;
         const char* pId;
         const char* pLabel;
         const char* pHelp;
         bool isExpression;
      };

      constexpr BlockEntry BlockTable[] = {
         {BlockType::Start, "Start", "Start",
          "Program entry point. Connect Next to begin execution. Cannot be deleted.",
          false},
         {BlockType::End, "End", "End",
          "Stops the current control-flow chain. Use to end a branch or the program path.",
          false},
         {BlockType::If, "If", "If",
          "Branches on Cond (non-zero = true). Runs Then or Else, then continues on Next.",
          false},
         {BlockType::ElseIf, "ElseIf", "Else If",
          "Wire from If/ElseIf Else for a flat else-if chain. Same ports as If.",
          false},
         {BlockType::Switch, "Switch", "Switch",
          "Multi-way branch on Value. Wire Cases to a Case chain; optional Default.",
          false},
         {BlockType::Case, "Case", "Case",
          "One switch arm. Property: value. Wire Body and NextCase. Used only from Switch.",
          false},
         {BlockType::While, "While", "While",
          "Loops while Cond is non-zero. Body runs each iteration; Exit runs afterward.",
          false},
         {BlockType::For, "For", "For",
          "Counted loop. Properties: iterator, start, end, type. Body runs for each value.",
          false},
         {BlockType::Break, "Break", "Break",
          "Exits the innermost loop or switch. Valid only inside a loop/switch body.",
          false},
         {BlockType::Continue, "Continue", "Continue",
          "Skips to the next loop iteration. Valid only inside a loop body.",
          false},
         {BlockType::Literal, "Literal", "Literal",
          "Constant value expression. Set value (e.g. 42, 3.14f, 'X') and type.",
          true},
         {BlockType::VariableDecl, "VariableDecl", "Var Decl",
          "Declares a local variable. Properties: name, type. Optional Init data input.",
          false},
         {BlockType::GlobalDecl, "GlobalDecl", "Global",
          "Declares a file-scope variable. Properties: name, type. Optional Init input.",
          false},
         {BlockType::VariableRef, "VariableRef", "Var Ref",
          "Reads a variable by name as an expression. Property: name.",
          true},
         {BlockType::Assign, "Assign", "Assign",
          "Assigns Value into target. Property: target (variable name).",
          false},
         {BlockType::CompoundAssign, "CompoundAssign", "Op Assign",
          "Compound assign. Properties: target, op (+ - * / %). Wire Value.",
          false},
         {BlockType::Inc, "Inc", "Inc",
          "Increments a variable. Property: target.",
          false},
         {BlockType::Dec, "Dec", "Dec",
          "Decrements a variable. Property: target.",
          false},
         {BlockType::Add, "Add", "Add",
          "Expression: Left + Right.",
          true},
         {BlockType::Sub, "Sub", "Sub",
          "Expression: Left - Right.",
          true},
         {BlockType::Mul, "Mul", "Mul",
          "Expression: Left * Right.",
          true},
         {BlockType::Div, "Div", "Div",
          "Expression: Left / Right (integer division for integer types).",
          true},
         {BlockType::Mod, "Mod", "Mod",
          "Expression: Left % Right (remainder).",
          true},
         {BlockType::Neg, "Neg", "Neg",
          "Expression: unary minus of Value.",
          true},
         {BlockType::Cast, "Cast", "Cast",
          "Casts Value to toType. Property: toType (e.g. int32_t, float, char*).",
          true},
         {BlockType::Equal, "Equal", "Equal",
          "Expression: Left == Right (1 if equal, else 0).",
          true},
         {BlockType::NotEqual, "NotEqual", "Not Equal",
          "Expression: Left != Right (1 if different, else 0).",
          true},
         {BlockType::Less, "Less", "Less",
          "Expression: Left < Right.",
          true},
         {BlockType::LessEqual, "LessEqual", "Less Equal",
          "Expression: Left <= Right.",
          true},
         {BlockType::Greater, "Greater", "Greater",
          "Expression: Left > Right.",
          true},
         {BlockType::GreaterEqual, "GreaterEqual", "Greater Equal",
          "Expression: Left >= Right.",
          true},
         {BlockType::And, "And", "And",
          "Expression: Left && Right (logical and).",
          true},
         {BlockType::Or, "Or", "Or",
          "Expression: Left || Right (logical or).",
          true},
         {BlockType::Not, "Not", "Not",
          "Expression: !Value (logical not).",
          true},
         {BlockType::Printf, "Printf", "Printf",
          "Prints text. Property: format (printf style). Wire Arg0-Arg5 for values.",
          false},
         {BlockType::WaitEnter, "WaitEnter", "Wait Enter",
          "Shows a prompt and waits until the user presses Enter.",
          false},
         {BlockType::ScanfInt, "ScanfInt", "Scanf Int",
          "Reads an integer with scanf. Properties: target variable, prompt text.",
          false},
         {BlockType::ScanfChar, "ScanfChar", "Scanf Char",
          "Reads one character with scanf. Properties: target variable, prompt text.",
          false},
         {BlockType::ScanfFloat, "ScanfFloat", "Scanf Float",
          "Reads a float with scanf %f. Properties: target variable, prompt text.",
          false},
         {BlockType::ScanfLine, "ScanfLine", "Scanf Line",
          "Reads a line with fgets. Properties: target buffer, size, prompt.",
          false},
         {BlockType::ArrayDecl, "ArrayDecl", "Array Decl",
          "Declares a local array. Properties: name, elemType, size.",
          false},
         {BlockType::IndexAssign, "IndexAssign", "Index Set",
          "Writes Value into array[Index]. Properties: array, elemType (cast).",
          false},
         {BlockType::IndexLoad, "IndexLoad", "Index Get",
          "Reads array[Index] as int32. Properties: array, elemType (storage hint).",
          true},
         {BlockType::StrLen, "StrLen", "Str Len",
          "Expression: strlen(buffer) as int32. Property: buffer.",
          true},
         {BlockType::StrCpy, "StrCpy", "Str Cpy",
          "Copies src into dest with strcpy. Properties: dest, src.",
          false},
         {BlockType::StrNCpy, "StrNCpy", "Str NCpy",
          "Copies up to count chars with strncpy and null-terminates. Properties: dest, src, count.",
          false},
         {BlockType::StrCmp, "StrCmp", "Str Cmp",
          "Expression: strcmp(left, right). Properties: left, right.",
          true},
         {BlockType::FileOpen, "FileOpen", "File Open",
          "Opens a file. Properties: handle (FILE*), path, mode (e.g. rb, w).",
          false},
         {BlockType::FileRead, "FileRead", "File Read",
          "fread into buffer. Properties: handle, buffer, size, count.",
          false},
         {BlockType::FileWrite, "FileWrite", "File Write",
          "fwrite from buffer. Properties: handle, buffer, size, count.",
          false},
         {BlockType::FileClose, "FileClose", "File Close",
          "Closes a FILE* handle. Property: handle.",
          false},
         {BlockType::FilePrintf, "FilePrintf", "File Printf",
          "fprintf to a FILE*. Properties: handle, format. Wire Arg0-Arg7.",
          false},
         {BlockType::FileGets, "FileGets", "File Gets",
          "fgets from a FILE*. Properties: handle, target, size, status (0/1).",
          false},
         {BlockType::Assert, "Assert", "Assert",
          "assert(Cond). Wire Cond; aborts if zero/false when NDEBUG is unset.",
          false},
         {BlockType::Comment, "Comment", "Comment",
          "Emits a C block comment. Property: text.",
          false},
         {BlockType::StructDecl, "StructDecl", "Struct",
          "File-scope typedef struct. Properties: name, fields (C field list).",
          false},
         {BlockType::FieldLoad, "FieldLoad", "Field Get",
          "Reads object.field or object->field. Properties: object, field (nested a.b ok), access.",
          true},
         {BlockType::FieldStore, "FieldStore", "Field Set",
          "Writes Value into object.field or object->field. Properties: object, field (nested a.b ok), access.",
          false},
         {BlockType::RandomChar, "RandomChar", "Rand Char",
          "Random character expression. Property: set = lower, upper, digit, or special.",
          true},
         {BlockType::ShuffleArray, "ShuffleArray", "Shuffle",
          "Fisher-Yates shuffle on a char array. Property: array. Wire Length.",
          false},
         {BlockType::Malloc, "Malloc", "Malloc",
          "Allocates memory. Property: elemType. Wire Size. Result is a pointer.",
          true},
         {BlockType::Free, "Free", "Free",
          "Frees a pointer from Malloc. Wire Ptr.",
          false},
         {BlockType::TimeNow, "TimeNow", "Time",
          "Expression: current Unix time as int64 (seconds since epoch).",
          true},
         {BlockType::LocalTime, "LocalTime", "Local Time",
          "Fills named int variables with local date/time fields (year, month, ...).",
          false},
         {BlockType::Sleep, "Sleep", "Sleep",
          "Pauses execution. Property: seconds.",
          false},
         {BlockType::Random, "Random", "Random",
          "Expression: rand() integer. Combine with Mod to limit the range.",
          true},
         {BlockType::FunctionDef, "FunctionDef", "Function",
          "Defines a C function. Properties: name, returnType, params. Wire Body.",
          false},
         {BlockType::Return, "Return", "Return",
          "Returns from a function. Optional Value input for the return expression.",
          false},
         {BlockType::Call, "Call", "Call",
          "Calls a function. Properties: function, returnType, storeTo. Wire Arg0-Arg7; Result for expression use.",
          false},
         {BlockType::StructLiteral, "StructLiteral", "Struct Literal",
          "Expression: designated initializer. Properties: type, init (e.g. .hp = 30, .atk = 6).",
          true}
      };

      constexpr size_t BlockTableCount = sizeof(BlockTable) / sizeof(BlockTable[0]);

      Port MakeControlIn(std::string_view name)
      {
         Port port;
         port.name = std::string(name);
         port.kind = PortKind::Control;
         port.direction = PortDirection::In;
         return port;
      }

      Port MakeControlOut(std::string_view name)
      {
         Port port;
         port.name = std::string(name);
         port.kind = PortKind::Control;
         port.direction = PortDirection::Out;
         return port;
      }

      Port MakeDataIn(std::string_view name, PrimitiveType base, bool isPointer)
      {
         Port port;
         port.name = std::string(name);
         port.kind = PortKind::Data;
         port.direction = PortDirection::In;
         port.dataType.base = base;
         port.dataType.isPointer = isPointer;
         return port;
      }

      Port MakeDataOut(std::string_view name, PrimitiveType base, bool isPointer)
      {
         Port port;
         port.name = std::string(name);
         port.kind = PortKind::Data;
         port.direction = PortDirection::Out;
         port.dataType.base = base;
         port.dataType.isPointer = isPointer;
         return port;
      }

      void AddBinaryExpressionPorts(Node* pNode)
      {
         pNode->ports.push_back(MakeDataIn("Left", PrimitiveType::Int32, false));
         pNode->ports.push_back(MakeDataIn("Right", PrimitiveType::Int32, false));
         pNode->ports.push_back(MakeDataOut("Result", PrimitiveType::Int32, false));
      }

      void AddUnaryExpressionPorts(Node* pNode)
      {
         pNode->ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
         pNode->ports.push_back(MakeDataOut("Result", PrimitiveType::Int32, false));
      }

      void AddIfLikePorts(Node* pNode)
      {
         pNode->ports.push_back(MakeControlIn("In"));
         pNode->ports.push_back(MakeDataIn("Cond", PrimitiveType::Int32, false));
         pNode->ports.push_back(MakeControlOut("Then"));
         pNode->ports.push_back(MakeControlOut("Else"));
         pNode->ports.push_back(MakeControlOut("Next"));
      }
   } // namespace

   std::string_view BlockTypeToString(BlockType blockType)
   {
      for (size_t index = 0; index < BlockTableCount; ++index)
      {
         if (BlockTable[index].type == blockType)
         {
            return BlockTable[index].pId;
         }
      }
      return "Start";
   }

   bool BlockTypeFromString(std::string_view text, BlockType* pOutBlockType)
   {
      if (pOutBlockType == nullptr)
      {
         return false;
      }
      for (size_t index = 0; index < BlockTableCount; ++index)
      {
         if (text == BlockTable[index].pId)
         {
            *pOutBlockType = BlockTable[index].type;
            return true;
         }
      }
      return false;
   }

   std::string_view BlockTypeLabel(BlockType blockType)
   {
      for (size_t index = 0; index < BlockTableCount; ++index)
      {
         if (BlockTable[index].type == blockType)
         {
            return BlockTable[index].pLabel;
         }
      }
      return "Block";
   }

   std::string_view BlockTypeHelpText(BlockType blockType)
   {
      for (size_t index = 0; index < BlockTableCount; ++index)
      {
         if (BlockTable[index].type == blockType)
         {
            return BlockTable[index].pHelp;
         }
      }
      return "No help available for this block.";
   }

   bool IsExpressionBlock(BlockType blockType)
   {
      for (size_t index = 0; index < BlockTableCount; ++index)
      {
         if (BlockTable[index].type == blockType)
         {
            return BlockTable[index].isExpression;
         }
      }
      return false;
   }

   Node CreateNode(NodeId id, BlockType blockType, float posX, float posY)
   {
      Node node;
      node.id = id;
      node.type = blockType;
      node.posX = posX;
      node.posY = posY;

      switch (blockType)
      {
         case BlockType::Start:
            node.ports.push_back(MakeControlOut("Next"));
            break;
         case BlockType::End:
            node.ports.push_back(MakeControlIn("In"));
            break;
         case BlockType::If:
         case BlockType::ElseIf:
            AddIfLikePorts(&node);
            break;
         case BlockType::Switch:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            node.ports.push_back(MakeControlOut("Cases"));
            node.ports.push_back(MakeControlOut("Default"));
            node.ports.push_back(MakeControlOut("Next"));
            break;
         case BlockType::Case:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Body"));
            node.ports.push_back(MakeControlOut("NextCase"));
            node.properties["value"] = "0";
            break;
         case BlockType::While:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeDataIn("Cond", PrimitiveType::Int32, false));
            node.ports.push_back(MakeControlOut("Body"));
            node.ports.push_back(MakeControlOut("Exit"));
            break;
         case BlockType::For:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Body"));
            node.ports.push_back(MakeControlOut("Exit"));
            node.properties["iterator"] = "i";
            node.properties["start"] = "0";
            node.properties["end"] = "10";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::Break:
         case BlockType::Continue:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            break;
         case BlockType::Literal:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["value"] = "0";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::VariableDecl:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Init", PrimitiveType::Int32, false));
            node.properties["name"] = "value";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::GlobalDecl:
            node.ports.push_back(MakeDataIn("Init", PrimitiveType::Int32, false));
            node.properties["name"] = "gValue";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::VariableRef:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["name"] = "value";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::Assign:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            node.properties["target"] = "value";
            node.properties["type"] = "int32_t";
            break;
         case BlockType::CompoundAssign:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            node.properties["target"] = "value";
            node.properties["op"] = "+";
            break;
         case BlockType::Inc:
         case BlockType::Dec:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["target"] = "value";
            break;
         case BlockType::Add:
         case BlockType::Sub:
         case BlockType::Mul:
         case BlockType::Div:
         case BlockType::Mod:
         case BlockType::Equal:
         case BlockType::NotEqual:
         case BlockType::Less:
         case BlockType::LessEqual:
         case BlockType::Greater:
         case BlockType::GreaterEqual:
         case BlockType::And:
         case BlockType::Or:
            AddBinaryExpressionPorts(&node);
            break;
         case BlockType::Neg:
         case BlockType::Not:
            AddUnaryExpressionPorts(&node);
            break;
         case BlockType::Cast:
            AddUnaryExpressionPorts(&node);
            node.properties["toType"] = "int32_t";
            break;
         case BlockType::Printf:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Arg0", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg1", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg2", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg3", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg4", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg5", PrimitiveType::Int32, false));
            node.properties["format"] = "value=%d\\n";
            break;
         case BlockType::WaitEnter:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["prompt"] = "Press Enter to exit...\\n";
            break;
         case BlockType::ScanfInt:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["target"] = "value";
            node.properties["prompt"] = "Enter value: ";
            break;
         case BlockType::ScanfChar:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["target"] = "ch";
            node.properties["prompt"] = "Enter character: ";
            break;
         case BlockType::ScanfFloat:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["target"] = "value";
            node.properties["prompt"] = "Enter float: ";
            break;
         case BlockType::ScanfLine:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["target"] = "buffer";
            node.properties["size"] = "256";
            node.properties["prompt"] = "Enter line: ";
            break;
         case BlockType::ArrayDecl:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["name"] = "buffer";
            node.properties["elemType"] = "char";
            node.properties["size"] = "256";
            break;
         case BlockType::IndexAssign:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Index", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            node.properties["array"] = "buffer";
            node.properties["elemType"] = "char";
            break;
         case BlockType::IndexLoad:
            node.ports.push_back(MakeDataIn("Index", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["array"] = "buffer";
            node.properties["elemType"] = "char";
            break;
         case BlockType::StrLen:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["buffer"] = "buffer";
            break;
         case BlockType::StrCpy:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["dest"] = "dest";
            node.properties["src"] = "src";
            break;
         case BlockType::StrNCpy:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["dest"] = "dest";
            node.properties["src"] = "src";
            node.properties["count"] = "256";
            break;
         case BlockType::StrCmp:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["left"] = "left";
            node.properties["right"] = "right";
            break;
         case BlockType::FileOpen:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["handle"] = "fp";
            node.properties["path"] = "data.bin";
            node.properties["mode"] = "rb";
            break;
         case BlockType::FileRead:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["handle"] = "fp";
            node.properties["buffer"] = "buffer";
            node.properties["size"] = "1";
            node.properties["count"] = "256";
            break;
         case BlockType::FileWrite:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["handle"] = "fp";
            node.properties["buffer"] = "buffer";
            node.properties["size"] = "1";
            node.properties["count"] = "256";
            break;
         case BlockType::FileClose:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["handle"] = "fp";
            break;
         case BlockType::FilePrintf:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Arg0", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg1", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg2", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg3", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg4", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg5", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg6", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataIn("Arg7", PrimitiveType::Int32, false));
            node.properties["handle"] = "fp";
            node.properties["format"] = "%d\\n";
            break;
         case BlockType::FileGets:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["handle"] = "fp";
            node.properties["target"] = "buffer";
            node.properties["size"] = "256";
            node.properties["status"] = "ok";
            break;
         case BlockType::Assert:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Cond", PrimitiveType::Int32, false));
            break;
         case BlockType::Comment:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["text"] = "TODO";
            break;
         case BlockType::StructDecl:
            node.properties["name"] = "Point";
            node.properties["fields"] = "int32_t x; int32_t y";
            break;
         case BlockType::FieldLoad:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["object"] = "point";
            node.properties["field"] = "x";
            node.properties["access"] = ".";
            break;
         case BlockType::FieldStore:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            node.properties["object"] = "point";
            node.properties["field"] = "x";
            node.properties["access"] = ".";
            break;
         case BlockType::RandomChar:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["set"] = "lower";
            break;
         case BlockType::ShuffleArray:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Length", PrimitiveType::Int32, false));
            node.properties["array"] = "buffer";
            break;
         case BlockType::Malloc:
            node.ports.push_back(MakeDataIn("Size", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataOut("Ptr", PrimitiveType::Void, true));
            node.properties["elemType"] = "uint8_t";
            break;
         case BlockType::Free:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Ptr", PrimitiveType::Void, true));
            break;
         case BlockType::TimeNow:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int64, false));
            break;
         case BlockType::LocalTime:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["year"] = "year";
            node.properties["month"] = "month";
            node.properties["day"] = "day";
            node.properties["hour"] = "hour";
            node.properties["minute"] = "minute";
            node.properties["second"] = "second";
            break;
         case BlockType::Sleep:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.properties["seconds"] = "1";
            break;
         case BlockType::Random:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            break;
         case BlockType::FunctionDef:
            node.ports.push_back(MakeControlOut("Body"));
            node.properties["name"] = "helper";
            node.properties["returnType"] = "int32_t";
            node.properties["params"] = "int32_t x";
            break;
         case BlockType::Return:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
            break;
         case BlockType::Call:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Arg0", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg1", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg2", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg3", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg4", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg5", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg6", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataIn("Arg7", PrimitiveType::Void, false));
            node.ports.push_back(MakeDataOut("Result", PrimitiveType::Int32, false));
            node.properties["function"] = "helper";
            node.properties["returnType"] = "int32_t";
            node.properties["storeTo"] = "";
            break;
         case BlockType::StructLiteral:
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Void, false));
            node.properties["type"] = "Point";
            node.properties["init"] = ".x = 0, .y = 0";
            break;
      }

      SyncNodePortTypes(&node);
      SyncPrintfArgVisibility(&node, nullptr);
      return node;
   }

   const Port* FindPort(const Node& node, std::string_view portName)
   {
      for (size_t index = 0; index < node.ports.size(); ++index)
      {
         if (node.ports[index].name == portName)
         {
            return &node.ports[index];
         }
      }
      return nullptr;
   }

   Port* FindPortMutable(Node* pNode, std::string_view portName)
   {
      if (pNode == nullptr)
      {
         return nullptr;
      }
      for (size_t index = 0; index < pNode->ports.size(); ++index)
      {
         if (pNode->ports[index].name == portName)
         {
            return &pNode->ports[index];
         }
      }
      return nullptr;
   }

   namespace
   {
      void ApplyTypeToPort(Port* pPort, std::string_view typeText)
      {
         if (pPort == nullptr)
         {
            return;
         }
         CType parsed {};
         if (CTypeFromString(typeText, &parsed))
         {
            pPort->dataType = parsed;
            return;
         }
         parsed.base = PrimitiveType::Void;
         parsed.isPointer = false;
         pPort->dataType = parsed;
      }

      uint32_t CountPrintfConversions(std::string_view formatText)
      {
         uint32_t count = 0;
         for (size_t index = 0; index < formatText.size(); ++index)
         {
            if (formatText[index] != '%')
            {
               continue;
            }
            if (((index + 1) < formatText.size()) && (formatText[index + 1] == '%'))
            {
               ++index;
               continue;
            }
            ++count;
         }
         return count;
      }
   } // namespace

   void SyncNodePortTypes(Node* pNode)
   {
      if (pNode == nullptr)
      {
         return;
      }

      if (pNode->type == BlockType::Literal)
      {
         const auto typeIterator = pNode->properties.find("type");
         const std::string_view typeText =
            (typeIterator != pNode->properties.end()) ? typeIterator->second
                                                      : std::string_view("int32_t");
         ApplyTypeToPort(FindPortMutable(pNode, "Value"), typeText);
         return;
      }
      if ((pNode->type == BlockType::VariableDecl) ||
          (pNode->type == BlockType::GlobalDecl))
      {
         const auto typeIterator = pNode->properties.find("type");
         const std::string_view typeText =
            (typeIterator != pNode->properties.end()) ? typeIterator->second
                                                      : std::string_view("int32_t");
         ApplyTypeToPort(FindPortMutable(pNode, "Init"), typeText);
         return;
      }
      if (pNode->type == BlockType::VariableRef)
      {
         const auto typeIterator = pNode->properties.find("type");
         const std::string_view typeText =
            (typeIterator != pNode->properties.end()) ? typeIterator->second
                                                      : std::string_view("int32_t");
         ApplyTypeToPort(FindPortMutable(pNode, "Value"), typeText);
         return;
      }
      if (pNode->type == BlockType::Assign)
      {
         const auto typeIterator = pNode->properties.find("type");
         const std::string_view typeText =
            (typeIterator != pNode->properties.end()) ? typeIterator->second
                                                      : std::string_view("int32_t");
         ApplyTypeToPort(FindPortMutable(pNode, "Value"), typeText);
         return;
      }
      if (pNode->type == BlockType::Call)
      {
         for (uint32_t argIndex = 0; argIndex < 8; ++argIndex)
         {
            std::string argName = "Arg";
            argName.append(std::to_string(argIndex));
            Port* pArg = FindPortMutable(pNode, argName);
            if (pArg == nullptr)
            {
               Port created;
               created.name = argName;
               created.kind = PortKind::Data;
               created.direction = PortDirection::In;
               created.dataType.base = PrimitiveType::Void;
               created.dataType.isPointer = false;
               created.visible = true;
               pNode->ports.push_back(created);
               pArg = FindPortMutable(pNode, argName);
            }
            if (pArg != nullptr)
            {
               pArg->dataType.base = PrimitiveType::Void;
               pArg->dataType.isPointer = false;
            }
         }
         const auto returnIterator = pNode->properties.find("returnType");
         const std::string_view returnText =
            (returnIterator != pNode->properties.end()) ? returnIterator->second
                                                        : std::string_view("int32_t");
         ApplyTypeToPort(FindPortMutable(pNode, "Result"), returnText);
         return;
      }
      if (pNode->type == BlockType::StructLiteral)
      {
         Port* pValue = FindPortMutable(pNode, "Value");
         if (pValue != nullptr)
         {
            pValue->dataType.base = PrimitiveType::Void;
            pValue->dataType.isPointer = false;
         }
      }
   }

   void SyncPrintfArgVisibility(Node* pNode, const GraphDocument* pDocument)
   {
      if (pNode == nullptr)
      {
         return;
      }
      if ((pNode->type != BlockType::Printf) && (pNode->type != BlockType::FilePrintf))
      {
         return;
      }

      const auto formatIterator = pNode->properties.find("format");
      const std::string_view formatText =
         (formatIterator != pNode->properties.end()) ? formatIterator->second
                                                     : std::string_view("");
      const uint32_t conversionCount = CountPrintfConversions(formatText);
      const uint32_t maxArgs = (pNode->type == BlockType::FilePrintf) ? 8u : 6u;

      for (uint32_t argIndex = 0; argIndex < maxArgs; ++argIndex)
      {
         std::string argName = "Arg";
         argName.append(std::to_string(argIndex));
         Port* pArg = FindPortMutable(pNode, argName);
         if (pArg == nullptr)
         {
            continue;
         }
         bool wired = false;
         if (pDocument != nullptr)
         {
            wired = (pDocument->FindIncomingEdge(pNode->id, argName) != nullptr);
         }
         pArg->visible = (argIndex < conversionCount) || wired;
      }
   }

   void SyncAllNodePorts(GraphDocument* pDocument)
   {
      if (pDocument == nullptr)
      {
         return;
      }
      std::vector<Node>& nodes = pDocument->GetNodesMutable();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         SyncNodePortTypes(&nodes[index]);
         SyncPrintfArgVisibility(&nodes[index], pDocument);
      }
   }
} // namespace Cgen
