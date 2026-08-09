/*!
 *\file node_factory.cpp
 *\brief Block type strings and node factory.
 */
#include "model/block_type.h"
#include "model/node.h"

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
         {BlockType::While, "While", "While",
          "Loops while Cond is non-zero. Body runs each iteration; Exit runs afterward.",
          false},
         {BlockType::For, "For", "For",
          "Counted loop. Properties: iterator, start, end, type. Body runs for each value.",
          false},
         {BlockType::Literal, "Literal", "Literal",
          "Constant value expression. Set value (e.g. 42, 'X', or 0).",
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
         {BlockType::ArrayDecl, "ArrayDecl", "Array Decl",
          "Declares a local array. Properties: name, elemType, size.",
          false},
         {BlockType::IndexAssign, "IndexAssign", "Index Set",
          "Writes Value into array[Index] as char. Property: array name.",
          false},
         {BlockType::IndexLoad, "IndexLoad", "Index Get",
          "Reads array[Index] as int. Property: array name.",
          true},
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
          "Calls a function. Properties: function, storeTo. Wire Arg0.",
          false}
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
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeDataIn("Cond", PrimitiveType::Int32, false));
            node.ports.push_back(MakeControlOut("Then"));
            node.ports.push_back(MakeControlOut("Else"));
            node.ports.push_back(MakeControlOut("Next"));
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
            break;
         case BlockType::Assign:
            node.ports.push_back(MakeControlIn("In"));
            node.ports.push_back(MakeControlOut("Next"));
            node.ports.push_back(MakeDataIn("Value", PrimitiveType::Int32, false));
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
            AddBinaryExpressionPorts(&node);
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
            break;
         case BlockType::IndexLoad:
            node.ports.push_back(MakeDataIn("Index", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataOut("Value", PrimitiveType::Int32, false));
            node.properties["array"] = "buffer";
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
            node.ports.push_back(MakeDataIn("Size", PrimitiveType::Uint64, false));
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
            node.ports.push_back(MakeDataIn("Arg0", PrimitiveType::Int32, false));
            node.ports.push_back(MakeDataOut("Result", PrimitiveType::Int32, false));
            node.properties["function"] = "helper";
            node.properties["storeTo"] = "";
            break;
      }

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
} // namespace Cgen
