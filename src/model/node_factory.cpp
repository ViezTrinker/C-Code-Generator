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
         bool isExpression;
      };

      constexpr BlockEntry BlockTable[] = {
         {BlockType::Start, "Start", "Start", false},
         {BlockType::End, "End", "End", false},
         {BlockType::If, "If", "If", false},
         {BlockType::While, "While", "While", false},
         {BlockType::For, "For", "For", false},
         {BlockType::Literal, "Literal", "Literal", true},
         {BlockType::VariableDecl, "VariableDecl", "Var Decl", false},
         {BlockType::GlobalDecl, "GlobalDecl", "Global", false},
         {BlockType::VariableRef, "VariableRef", "Var Ref", true},
         {BlockType::Assign, "Assign", "Assign", false},
         {BlockType::Add, "Add", "Add", true},
         {BlockType::Sub, "Sub", "Sub", true},
         {BlockType::Mul, "Mul", "Mul", true},
         {BlockType::Div, "Div", "Div", true},
         {BlockType::Equal, "Equal", "Equal", true},
         {BlockType::NotEqual, "NotEqual", "Not Equal", true},
         {BlockType::Less, "Less", "Less", true},
         {BlockType::LessEqual, "LessEqual", "Less Equal", true},
         {BlockType::Greater, "Greater", "Greater", true},
         {BlockType::GreaterEqual, "GreaterEqual", "Greater Equal", true},
         {BlockType::Printf, "Printf", "Printf", false},
         {BlockType::Malloc, "Malloc", "Malloc", true},
         {BlockType::Free, "Free", "Free", false},
         {BlockType::TimeNow, "TimeNow", "Time", true},
         {BlockType::Random, "Random", "Random", true},
         {BlockType::FunctionDef, "FunctionDef", "Function", false},
         {BlockType::Return, "Return", "Return", false},
         {BlockType::Call, "Call", "Call", false}
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
            node.properties["format"] = "value=%d\\n";
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
