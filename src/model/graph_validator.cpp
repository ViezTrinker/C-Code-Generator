/*!
 *\file graph_validator.cpp
 *\brief Flowchart validation: reachability, names, Break scope, ports.
 */
#include "model/graph_validator.h"

#include <cctype>
#include <queue>
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "model/block_type.h"
#include "model/c_type.h"
#include "model/edge.h"
#include "model/node.h"
#include "model/port.h"

namespace Cgen
{
   namespace
   {
      std::string GetProperty(const Node& node, std::string_view key, std::string_view fallback)
      {
         const auto found = node.properties.find(std::string(key));
         if (found == node.properties.end())
         {
            return std::string(fallback);
         }
         return found->second;
      }

      bool IsRootishType(BlockType blockType)
      {
         return (blockType == BlockType::Start) || (blockType == BlockType::FunctionDef) ||
                (blockType == BlockType::StructDecl) || (blockType == BlockType::EnumDecl) ||
                (blockType == BlockType::TypedefDecl) ||
                (blockType == BlockType::GlobalDecl) || (blockType == BlockType::Comment);
      }

      bool IsStatementLike(BlockType blockType)
      {
         if (IsExpressionBlock(blockType))
         {
            return false;
         }
         if (IsRootishType(blockType))
         {
            return false;
         }
         return true;
      }

      void AddIssue(ValidationReport* pReport,
                   ValidationSeverity severity,
                   NodeId nodeId,
                   std::string_view message)
      {
         if (pReport == nullptr)
         {
            return;
         }
         ValidationIssue issue;
         issue.severity = severity;
         issue.nodeId = nodeId;
         issue.message = std::string(message);
         pReport->issues.push_back(issue);
      }

      void CollectControlReachable(const GraphDocument& document,
                                  NodeId rootId,
                                  std::unordered_set<NodeId>* pVisited)
      {
         if ((pVisited == nullptr) || (rootId == 0))
         {
            return;
         }
         std::queue<NodeId> pending;
         pending.push(rootId);
         pVisited->insert(rootId);
         while (!pending.empty())
         {
            const NodeId currentId = pending.front();
            pending.pop();
            const Node* pNode = document.FindNode(currentId);
            if (pNode == nullptr)
            {
               continue;
            }
            for (const Port& port : pNode->ports)
            {
               if ((port.kind != PortKind::Control) ||
                   (port.direction != PortDirection::Out))
               {
                  continue;
               }
               const Edge* pEdge =
                  document.FindOutgoingEdge(currentId, port.name);
               if (pEdge == nullptr)
               {
                  continue;
               }
               if (pVisited->find(pEdge->toNodeId) != pVisited->end())
               {
                  continue;
               }
               pVisited->insert(pEdge->toNodeId);
               pending.push(pEdge->toNodeId);
            }
         }
      }

      void CollectDeclaredNames(const GraphDocument& document, std::set<std::string>* pNames)
      {
         if (pNames == nullptr)
         {
            return;
         }
         for (const Node& node : document.GetNodes())
         {
            if ((node.type == BlockType::VariableDecl) ||
                (node.type == BlockType::GlobalDecl) ||
                (node.type == BlockType::ArrayDecl))
            {
               const std::string name = GetProperty(node, "name", "");
               if (!name.empty())
               {
                  pNames->insert(name);
               }
            }
            if (node.type == BlockType::For)
            {
               const std::string iteratorName = GetProperty(node, "iterator", "");
               if (!iteratorName.empty())
               {
                  pNames->insert(iteratorName);
               }
            }
            if (node.type == BlockType::FunctionDef)
            {
               const uint32_t paramCount = GetFunctionParamCount(node);
               for (uint32_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
               {
                  std::string paramName;
                  std::string paramType;
                  if (GetFunctionParam(node, paramIndex, &paramName, &paramType) &&
                      (!paramName.empty()))
                  {
                     pNames->insert(paramName);
                  }
               }
            }
         }
      }

      void InsertNamedProperty(const Node& node,
                               std::string_view key,
                               std::set<std::string>* pNames)
      {
         if (pNames == nullptr)
         {
            return;
         }
         const std::string value = GetProperty(node, key, "");
         if (!value.empty())
         {
            pNames->insert(value);
         }
      }

      void CollectUsedNames(const GraphDocument& document, std::set<std::string>* pNames)
      {
         if (pNames == nullptr)
         {
            return;
         }

         for (const Node& node : document.GetNodes())
         {
            if (node.type == BlockType::VariableRef)
            {
               InsertNamedProperty(node, "name", pNames);
            }
            if (node.type == BlockType::AddressOf)
            {
               InsertNamedProperty(node, "name", pNames);
            }
            if ((node.type == BlockType::Assign) || (node.type == BlockType::Inc) ||
                (node.type == BlockType::Dec) || (node.type == BlockType::CompoundAssign))
            {
               InsertNamedProperty(node, "target", pNames);
            }
            if ((node.type == BlockType::ScanfInt) || (node.type == BlockType::ScanfChar) ||
                (node.type == BlockType::ScanfFloat) || (node.type == BlockType::ScanfLine))
            {
               InsertNamedProperty(node, "target", pNames);
            }
            if ((node.type == BlockType::FieldLoad) || (node.type == BlockType::FieldStore))
            {
               InsertNamedProperty(node, "object", pNames);
            }
            if ((node.type == BlockType::IndexAssign) || (node.type == BlockType::IndexLoad) ||
                (node.type == BlockType::ShuffleArray))
            {
               InsertNamedProperty(node, "array", pNames);
            }
            if (node.type == BlockType::StrLen)
            {
               InsertNamedProperty(node, "buffer", pNames);
            }
            if ((node.type == BlockType::StrCpy) || (node.type == BlockType::StrNCpy) ||
                (node.type == BlockType::StrCmp))
            {
               InsertNamedProperty(node, "dest", pNames);
               InsertNamedProperty(node, "src", pNames);
               InsertNamedProperty(node, "left", pNames);
               InsertNamedProperty(node, "right", pNames);
            }
            if ((node.type == BlockType::FileOpen) || (node.type == BlockType::FileClose) ||
                (node.type == BlockType::FileRead) || (node.type == BlockType::FileWrite) ||
                (node.type == BlockType::FilePrintf) || (node.type == BlockType::FileGets))
            {
               InsertNamedProperty(node, "handle", pNames);
               InsertNamedProperty(node, "buffer", pNames);
               InsertNamedProperty(node, "target", pNames);
            }
            if (node.type == BlockType::Call)
            {
               InsertNamedProperty(node, "storeTo", pNames);
            }
            if (node.type == BlockType::LocalTime)
            {
               InsertNamedProperty(node, "year", pNames);
               InsertNamedProperty(node, "month", pNames);
               InsertNamedProperty(node, "day", pNames);
               InsertNamedProperty(node, "hour", pNames);
               InsertNamedProperty(node, "minute", pNames);
               InsertNamedProperty(node, "second", pNames);
            }
         }
      }

      bool PortRequiresData(BlockType blockType, std::string_view portName)
      {
         if (portName == "Cond")
         {
            return (blockType == BlockType::If) || (blockType == BlockType::ElseIf) ||
                   (blockType == BlockType::While) || (blockType == BlockType::Assert);
         }
         if (portName == "Value")
         {
            return (blockType == BlockType::Assign) ||
                   (blockType == BlockType::CompoundAssign) ||
                   (blockType == BlockType::FieldStore) ||
                   (blockType == BlockType::IndexAssign) ||
                   (blockType == BlockType::DerefStore) ||
                   (blockType == BlockType::Switch);
         }
         if (portName == "Ptr")
         {
            return (blockType == BlockType::DerefLoad) ||
                   (blockType == BlockType::DerefStore) ||
                   (blockType == BlockType::Free);
         }
         if ((portName == "Left") || (portName == "Right"))
         {
            return (blockType == BlockType::Add) || (blockType == BlockType::Sub) ||
                   (blockType == BlockType::Mul) || (blockType == BlockType::Div) ||
                   (blockType == BlockType::Mod) || (blockType == BlockType::Equal) ||
                   (blockType == BlockType::NotEqual) || (blockType == BlockType::Less) ||
                   (blockType == BlockType::LessEqual) ||
                   (blockType == BlockType::Greater) ||
                   (blockType == BlockType::GreaterEqual) || (blockType == BlockType::And) ||
                   (blockType == BlockType::Or);
         }
         if (portName == "Index")
         {
            return (blockType == BlockType::IndexAssign) ||
                   (blockType == BlockType::IndexLoad);
         }
         return false;
      }

      bool IsInsideLoop(const GraphDocument& document, NodeId nodeId)
      {
         std::unordered_set<NodeId> visited;
         std::queue<NodeId> pending;
         pending.push(nodeId);
         visited.insert(nodeId);
         while (!pending.empty())
         {
            const NodeId currentId = pending.front();
            pending.pop();
            for (const Edge& edge : document.GetEdges())
            {
               if (edge.toNodeId != currentId)
               {
                  continue;
               }
               const Node* pFrom = document.FindNode(edge.fromNodeId);
               if (pFrom == nullptr)
               {
                  continue;
               }
               const Port* pPort = FindPort(*pFrom, edge.fromPort);
               if ((pPort == nullptr) || (pPort->kind != PortKind::Control))
               {
                  continue;
               }
               if ((pFrom->type == BlockType::While) || (pFrom->type == BlockType::For))
               {
                  if ((edge.fromPort == "Body") || (edge.fromPort == "Next"))
                  {
                     return true;
                  }
               }
               if (visited.find(edge.fromNodeId) == visited.end())
               {
                  visited.insert(edge.fromNodeId);
                  pending.push(edge.fromNodeId);
               }
            }
         }
         return false;
      }

   } // namespace

   ValidationReport ValidateGraph(const GraphDocument& document)
   {
      ValidationReport report;

      for (const Edge& edge : document.GetEdges())
      {
         if (document.FindNode(edge.fromNodeId) == nullptr)
         {
            AddIssue(&report,
                     ValidationSeverity::Error,
                     0,
                     "Dangling edge: missing from-node.");
         }
         if (document.FindNode(edge.toNodeId) == nullptr)
         {
            AddIssue(&report,
                     ValidationSeverity::Error,
                     0,
                     "Dangling edge: missing to-node.");
         }
      }

      const Node* pStart = nullptr;
      for (const Node& node : document.GetNodes())
      {
         if (node.type == BlockType::Start)
         {
            pStart = &node;
            break;
         }
      }
      if (pStart == nullptr)
      {
         AddIssue(&report, ValidationSeverity::Error, 0, "Document has no Start block.");
         return report;
      }

      std::unordered_set<NodeId> fromStart;
      CollectControlReachable(document, pStart->id, &fromStart);
      bool foundEnd = false;
      for (const NodeId nodeId : fromStart)
      {
         const Node* pNode = document.FindNode(nodeId);
         if ((pNode != nullptr) && (pNode->type == BlockType::End))
         {
            foundEnd = true;
            break;
         }
      }
      if (!foundEnd)
      {
         AddIssue(&report,
                  ValidationSeverity::Error,
                  pStart->id,
                  "No End block reachable from Start along control flow.");
      }

      for (const Node& node : document.GetNodes())
      {
         if (node.type != BlockType::End)
         {
            continue;
         }
         if (fromStart.find(node.id) == fromStart.end())
         {
            AddIssue(&report,
                     ValidationSeverity::Warning,
                     node.id,
                     "Unreachable End (not on Start control-flow path).");
         }
      }

      for (const Node& node : document.GetNodes())
      {
         if (node.type != BlockType::Switch)
         {
            continue;
         }
         if (document.FindOutgoingEdge(node.id, "Default") == nullptr)
         {
            AddIssue(&report,
                     ValidationSeverity::Warning,
                     node.id,
                     "Switch has no Default arm.");
         }
      }

      std::unordered_set<NodeId> reachable;
      CollectControlReachable(document, pStart->id, &reachable);
      for (const Node& node : document.GetNodes())
      {
         if (node.type == BlockType::FunctionDef)
         {
            CollectControlReachable(document, node.id, &reachable);
         }
      }

      for (const Node& node : document.GetNodes())
      {
         if (!IsStatementLike(node.type))
         {
            continue;
         }
         if (reachable.find(node.id) == reachable.end())
         {
            std::ostringstream stream;
            stream << "Unreachable " << BlockTypeLabel(node.type)
                   << " block (not connected from Start or a Function).";
            AddIssue(&report, ValidationSeverity::Warning, node.id, stream.str());
         }
      }

      for (const Node& node : document.GetNodes())
      {
         for (const Port& port : node.ports)
         {
            if ((port.kind != PortKind::Data) || (port.direction != PortDirection::In))
            {
               continue;
            }
            if (!PortRequiresData(node.type, port.name))
            {
               continue;
            }
            if (document.FindIncomingEdge(node.id, port.name) == nullptr)
            {
               std::ostringstream stream;
               stream << BlockTypeLabel(node.type) << " is missing required input '"
                      << port.name << "'.";
               AddIssue(&report, ValidationSeverity::Error, node.id, stream.str());
            }
         }
      }

      std::set<std::string> declaredNames;
      CollectDeclaredNames(document, &declaredNames);

      for (const Node& node : document.GetNodes())
      {
         if (node.type == BlockType::VariableRef)
         {
            const std::string name = GetProperty(node, "name", "");
            if ((!name.empty()) && (declaredNames.find(name) == declaredNames.end()))
            {
               AddIssue(&report,
                        ValidationSeverity::Error,
                        node.id,
                        "VariableRef uses undeclared name '" + name + "'.");
            }
         }

         if ((node.type == BlockType::Assign) || (node.type == BlockType::Inc) ||
             (node.type == BlockType::Dec) || (node.type == BlockType::CompoundAssign))
         {
            const std::string target = GetProperty(node, "target", "");
            if ((!target.empty()) && (declaredNames.find(target) == declaredNames.end()))
            {
               AddIssue(&report,
                        ValidationSeverity::Error,
                        node.id,
                        std::string(BlockTypeLabel(node.type)) + " target '" + target +
                           "' is undeclared.");
            }
         }

         if ((node.type == BlockType::ScanfInt) || (node.type == BlockType::ScanfChar) ||
             (node.type == BlockType::ScanfFloat) || (node.type == BlockType::ScanfLine))
         {
            const std::string target = GetProperty(node, "target", "");
            if ((!target.empty()) && (declaredNames.find(target) == declaredNames.end()))
            {
               AddIssue(&report,
                        ValidationSeverity::Error,
                        node.id,
                        "Scanf target '" + target + "' is undeclared.");
            }
         }

         if ((node.type == BlockType::FileOpen) || (node.type == BlockType::FileClose) ||
             (node.type == BlockType::FileRead) || (node.type == BlockType::FileWrite) ||
             (node.type == BlockType::FilePrintf) || (node.type == BlockType::FileGets))
         {
            const std::string handle = GetProperty(node, "handle", "");
            if ((!handle.empty()) && (declaredNames.find(handle) == declaredNames.end()))
            {
               AddIssue(&report,
                        ValidationSeverity::Warning,
                        node.id,
                        "File handle '" + handle + "' has no matching VariableDecl.");
            }
         }

         if ((node.type == BlockType::Break) || (node.type == BlockType::Continue))
         {
            if (!IsInsideLoop(document, node.id))
            {
               AddIssue(&report,
                        ValidationSeverity::Error,
                        node.id,
                        std::string(BlockTypeLabel(node.type)) +
                           " is not inside a While/For Body.");
            }
         }

         if (node.type == BlockType::Call)
         {
            const std::string functionName = GetProperty(node, "function", "");
            if (functionName.empty())
            {
               AddIssue(&report,
                        ValidationSeverity::Error,
                        node.id,
                        "Call is missing property 'function'.");
            }
            else
            {
               const Node* pFunction = nullptr;
               for (const Node& candidate : document.GetNodes())
               {
                  if (candidate.type != BlockType::FunctionDef)
                  {
                     continue;
                  }
                  if (GetProperty(candidate, "name", "") == functionName)
                  {
                     pFunction = &candidate;
                     break;
                  }
               }
               if (pFunction == nullptr)
               {
                  AddIssue(&report,
                           ValidationSeverity::Warning,
                           node.id,
                           "Call targets unknown function '" + functionName +
                              "' (no FunctionDef).");
               }
               else
               {
                  const uint32_t expectedArgs = GetFunctionParamCount(*pFunction);
                  uint32_t wiredArgs = 0;
                  for (uint32_t argIndex = 0; argIndex < MaxFunctionParams; ++argIndex)
                  {
                     const std::string argName = "Arg" + std::to_string(argIndex);
                     if (document.FindIncomingEdge(node.id, argName) != nullptr)
                     {
                        ++wiredArgs;
                     }
                  }
                  if (wiredArgs != expectedArgs)
                  {
                     std::ostringstream stream;
                     stream << "Call to '" << functionName << "' has " << wiredArgs
                            << " args but FunctionDef expects " << expectedArgs << ".";
                     AddIssue(&report,
                              ValidationSeverity::Error,
                              node.id,
                              stream.str());
                  }
               }
            }
         }

         if (node.type == BlockType::FunctionDef)
         {
            const uint32_t paramCount = GetFunctionParamCount(node);
            const std::string functionName = GetProperty(node, "name", "helper");
            for (uint32_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
            {
               const std::string portName = "Param" + std::to_string(paramIndex);
               if (document.FindOutgoingEdge(node.id, portName) != nullptr)
               {
                  continue;
               }
               std::string paramName;
               std::string paramType;
               if (!GetFunctionParam(node, paramIndex, &paramName, &paramType))
               {
                  paramName = portName;
               }
               AddIssue(&report,
                        ValidationSeverity::Warning,
                        node.id,
                        "Unused Param port " + portName + " ('" + paramName +
                           "') on FunctionDef '" + functionName + "'.");
            }
         }
      }

      for (const Edge& edge : document.GetEdges())
      {
         const Node* pFrom = document.FindNode(edge.fromNodeId);
         const Node* pTo = document.FindNode(edge.toNodeId);
         if ((pFrom == nullptr) || (pTo == nullptr))
         {
            continue;
         }
         const Port* pFromPort = FindPort(*pFrom, edge.fromPort);
         const Port* pToPort = FindPort(*pTo, edge.toPort);
         if ((pFromPort == nullptr) || (pToPort == nullptr))
         {
            continue;
         }
         if ((pFromPort->kind != PortKind::Data) || (pToPort->kind != PortKind::Data))
         {
            continue;
         }
         if (!AreTypesCompatible(pFromPort->dataType, pToPort->dataType))
         {
            std::ostringstream stream;
            stream << "Type mismatch on wire " << edge.fromPort << " -> " << edge.toPort
                   << ".";
            AddIssue(&report, ValidationSeverity::Error, edge.toNodeId, stream.str());
         }

         if (pFrom->type == BlockType::Literal)
         {
            CType literalCType;
            const std::string typeText = GetProperty(*pFrom, "type", "int32_t");
            if (CTypeFromString(typeText, &literalCType))
            {
               if (!AreTypesCompatible(literalCType, pToPort->dataType))
               {
                  AddIssue(&report,
                           ValidationSeverity::Warning,
                           pFrom->id,
                           "Literal type property disagrees with destination port type.");
               }
            }
         }
      }

      std::set<std::string> usedNames;
      CollectUsedNames(document, &usedNames);
      for (const Node& node : document.GetNodes())
      {
         if ((node.type != BlockType::VariableDecl) &&
             (node.type != BlockType::GlobalDecl) &&
             (node.type != BlockType::ArrayDecl))
         {
            continue;
         }
         const std::string name = GetProperty(node, "name", "");
         if (name.empty())
         {
            continue;
         }
         if (usedNames.find(name) == usedNames.end())
         {
            AddIssue(&report,
                     ValidationSeverity::Warning,
                     node.id,
                     "Unused declaration '" + name + "'.");
         }
      }

      return report;
   }

   void BuildNodeSeverityMap(
      const ValidationReport& report,
      std::unordered_map<NodeId, ValidationSeverity>* pOutMap)
   {
      if (pOutMap == nullptr)
      {
         return;
      }
      pOutMap->clear();
      for (size_t index = 0; index < report.issues.size(); ++index)
      {
         const ValidationIssue& issue = report.issues[index];
         if (issue.nodeId == 0)
         {
            continue;
         }
         const auto existing = pOutMap->find(issue.nodeId);
         if (existing == pOutMap->end())
         {
            (*pOutMap)[issue.nodeId] = issue.severity;
            continue;
         }
         if (static_cast<int8_t>(issue.severity) >
             static_cast<int8_t>(existing->second))
         {
            existing->second = issue.severity;
         }
      }
   }
} // namespace Cgen
