/*!
 *\file c_codegen.cpp
 *\brief C99 code generation from the flowchart IR.
 */
#include "codegen/c_codegen.h"

#include <array>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace Cgen
{
   namespace
   {
      using TempMap = std::unordered_map<NodeId, std::string>;

      struct EmitContext
      {
         const GraphDocument* pDocument = nullptr;
         std::ostringstream* pOut = nullptr;
         std::string* pDiagnostics = nullptr;
         TempMap temps;
         uint32_t tempCounter = 0;
         int32_t indentLevel = 0;
         bool hadError = false;
      };

      void AppendDiag(EmitContext* pContext, std::string_view message)
      {
         if ((pContext == nullptr) || (pContext->pDiagnostics == nullptr))
         {
            return;
         }
         pContext->pDiagnostics->append(message);
         pContext->pDiagnostics->append("\n");
         pContext->hadError = true;
      }

      void WriteIndent(EmitContext* pContext)
      {
         for (int32_t index = 0; index < pContext->indentLevel; ++index)
         {
            (*pContext->pOut) << "   ";
         }
      }

      std::string MakeTempName(EmitContext* pContext)
      {
         std::ostringstream name;
         name << "tmp_" << pContext->tempCounter;
         ++pContext->tempCounter;
         return name.str();
      }

      const Node* GetNode(const EmitContext& context, NodeId nodeId)
      {
         return context.pDocument->FindNode(nodeId);
      }

      std::string GetProperty(const Node& node, std::string_view key, std::string_view fallback)
      {
         const auto iterator = node.properties.find(std::string(key));
         if (iterator == node.properties.end())
         {
            return std::string(fallback);
         }
         return iterator->second;
      }

      std::string UnescapeFormat(std::string_view text)
      {
         std::string result;
         result.reserve(text.size());
         for (size_t index = 0; index < text.size(); ++index)
         {
            const char current = text[index];
            if ((current == '\\') && ((index + 1) < text.size()))
            {
               const char next = text[index + 1];
               if (next == 'n')
               {
                  result.push_back('\n');
                  ++index;
                  continue;
               }
               if (next == 't')
               {
                  result.push_back('\t');
                  ++index;
                  continue;
               }
               if (next == '\\')
               {
                  result.push_back('\\');
                  ++index;
                  continue;
               }
            }
            result.push_back(current);
         }
         return result;
      }

      std::string EscapeCString(std::string_view text)
      {
         std::string result;
         for (size_t index = 0; index < text.size(); ++index)
         {
            const char current = text[index];
            if (current == '\\')
            {
               result.append("\\\\");
            }
            else if (current == '"')
            {
               result.append("\\\"");
            }
            else if (current == '\n')
            {
               result.append("\\n");
            }
            else if (current == '\t')
            {
               result.append("\\t");
            }
            else
            {
               result.push_back(current);
            }
         }
         return result;
      }

      std::string EmitExpression(EmitContext* pContext, NodeId nodeId);

      std::string EmitInputExpression(EmitContext* pContext,
                                      NodeId nodeId,
                                      std::string_view portName)
      {
         const Edge* pEdge =
            pContext->pDocument->FindIncomingEdge(nodeId, portName);
         if (pEdge == nullptr)
         {
            return std::string();
         }
         return EmitExpression(pContext, pEdge->fromNodeId);
      }

      std::string BinaryOperator(BlockType blockType)
      {
         switch (blockType)
         {
            case BlockType::Add:
               return "+";
            case BlockType::Sub:
               return "-";
            case BlockType::Mul:
               return "*";
            case BlockType::Div:
               return "/";
            case BlockType::Mod:
               return "%";
            case BlockType::Equal:
               return "==";
            case BlockType::NotEqual:
               return "!=";
            case BlockType::Less:
               return "<";
            case BlockType::LessEqual:
               return "<=";
            case BlockType::Greater:
               return ">";
            case BlockType::GreaterEqual:
               return ">=";
            case BlockType::And:
               return "&&";
            case BlockType::Or:
               return "||";
            default:
               return "+";
         }
      }

      bool IsValidCompoundOp(std::string_view opText)
      {
         return (opText == "+") || (opText == "-") || (opText == "*") ||
                (opText == "/") || (opText == "%");
      }

      std::string EmitExpression(EmitContext* pContext, NodeId nodeId)
      {
         const auto tempIterator = pContext->temps.find(nodeId);
         if (tempIterator != pContext->temps.end())
         {
            return tempIterator->second;
         }

         const Node* pNode = GetNode(*pContext, nodeId);
         if (pNode == nullptr)
         {
            AppendDiag(pContext, "Missing expression node.");
            return "0";
         }

         std::string expression;
         switch (pNode->type)
         {
            case BlockType::Literal:
            {
               const std::string value = GetProperty(*pNode, "value", "0");
               expression = value;
               break;
            }
            case BlockType::VariableRef:
            {
               expression = GetProperty(*pNode, "name", "value");
               break;
            }
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
            {
               const std::string left = EmitInputExpression(pContext, nodeId, "Left");
               const std::string right = EmitInputExpression(pContext, nodeId, "Right");
               expression = "(" + left + " " + BinaryOperator(pNode->type) + " " + right + ")";
               break;
            }
            case BlockType::Not:
            {
               const std::string valueExpr =
                  EmitInputExpression(pContext, nodeId, "Value");
               expression = "(!" + valueExpr + ")";
               break;
            }
            case BlockType::Neg:
            {
               const std::string valueExpr =
                  EmitInputExpression(pContext, nodeId, "Value");
               expression = "(-" + valueExpr + ")";
               break;
            }
            case BlockType::StrLen:
            {
               const std::string bufferName = GetProperty(*pNode, "buffer", "buffer");
               expression = "((int32_t)strlen(" + bufferName + "))";
               break;
            }
            case BlockType::StrCmp:
            {
               const std::string leftName = GetProperty(*pNode, "left", "left");
               const std::string rightName = GetProperty(*pNode, "right", "right");
               expression = "strcmp(" + leftName + ", " + rightName + ")";
               break;
            }
            case BlockType::TimeNow:
               expression = "((int64_t)time(NULL))";
               break;
            case BlockType::Random:
               expression = "rand()";
               break;
            case BlockType::IndexLoad:
            {
               const std::string arrayName = GetProperty(*pNode, "array", "buffer");
               const std::string indexExpr =
                  EmitInputExpression(pContext, nodeId, "Index");
               expression = "((int32_t)(" + arrayName + "[" + indexExpr + "]))";
               break;
            }
            case BlockType::RandomChar:
            {
               const std::string setName = GetProperty(*pNode, "set", "lower");
               if (setName == "upper")
               {
                  expression = "((int32_t)('A' + (rand() % 26)))";
               }
               else if (setName == "digit")
               {
                  expression = "((int32_t)('0' + (rand() % 10)))";
               }
               else if (setName == "special")
               {
                  expression =
                     "((int32_t)(\"!@#$%&*?\"[rand() % 8]))";
               }
               else
               {
                  expression = "((int32_t)('a' + (rand() % 26)))";
               }
               break;
            }
            case BlockType::Malloc:
            {
               const std::string sizeExpr = EmitInputExpression(pContext, nodeId, "Size");
               const std::string elemType = GetProperty(*pNode, "elemType", "uint8_t");
               expression = "((" + elemType + "*)malloc((size_t)(" + sizeExpr + ")))";
               break;
            }
            case BlockType::Call:
            {
               const std::string functionName = GetProperty(*pNode, "function", "helper");
               const std::string arg0 = EmitInputExpression(pContext, nodeId, "Arg0");
               expression = functionName + "(" + arg0 + ")";
               break;
            }
            default:
               AppendDiag(pContext,
                          std::string("Node is not an expression: ") +
                             std::string(BlockTypeToString(pNode->type)));
               expression = "0";
               break;
         }

         pContext->temps[nodeId] = expression;
         return expression;
      }

      void EmitStatementChain(EmitContext* pContext, NodeId startNodeId);
      void EmitIfElseChain(EmitContext* pContext, const Node& node, bool asElseIf);

      void EmitSwitchStatement(EmitContext* pContext, const Node& node)
      {
         const std::string valueExpr = EmitInputExpression(pContext, node.id, "Value");
         WriteIndent(pContext);
         (*pContext->pOut) << "switch (" << valueExpr << ")\n";
         WriteIndent(pContext);
         (*pContext->pOut) << "{\n";
         ++pContext->indentLevel;

         const Edge* pCases =
            pContext->pDocument->FindOutgoingEdge(node.id, "Cases");
         NodeId caseId = 0;
         if (pCases != nullptr)
         {
            caseId = pCases->toNodeId;
         }
         std::unordered_set<NodeId> visitedCases;
         while (caseId != 0)
         {
            if (visitedCases.find(caseId) != visitedCases.end())
            {
               AppendDiag(pContext, "Cycle detected in Switch Case chain.");
               break;
            }
            visitedCases.insert(caseId);

            const Node* pCaseNode = GetNode(*pContext, caseId);
            if (pCaseNode == nullptr)
            {
               AppendDiag(pContext, "Missing Case node in Switch chain.");
               break;
            }
            if (pCaseNode->type != BlockType::Case)
            {
               AppendDiag(pContext, "Switch Cases must point to a Case block.");
               break;
            }

            const std::string matchValue = GetProperty(*pCaseNode, "value", "0");
            WriteIndent(pContext);
            (*pContext->pOut) << "case " << matchValue << ":\n";
            WriteIndent(pContext);
            (*pContext->pOut) << "{\n";
            ++pContext->indentLevel;
            const Edge* pBody =
               pContext->pDocument->FindOutgoingEdge(caseId, "Body");
            if (pBody != nullptr)
            {
               EmitStatementChain(pContext, pBody->toNodeId);
            }
            WriteIndent(pContext);
            (*pContext->pOut) << "break;\n";
            --pContext->indentLevel;
            WriteIndent(pContext);
            (*pContext->pOut) << "}\n";

            const Edge* pNextCase =
               pContext->pDocument->FindOutgoingEdge(caseId, "NextCase");
            if (pNextCase == nullptr)
            {
               caseId = 0;
            }
            else
            {
               caseId = pNextCase->toNodeId;
            }
         }

         const Edge* pDefault =
            pContext->pDocument->FindOutgoingEdge(node.id, "Default");
         if (pDefault != nullptr)
         {
            WriteIndent(pContext);
            (*pContext->pOut) << "default:\n";
            WriteIndent(pContext);
            (*pContext->pOut) << "{\n";
            ++pContext->indentLevel;
            EmitStatementChain(pContext, pDefault->toNodeId);
            WriteIndent(pContext);
            (*pContext->pOut) << "break;\n";
            --pContext->indentLevel;
            WriteIndent(pContext);
            (*pContext->pOut) << "}\n";
         }

         --pContext->indentLevel;
         WriteIndent(pContext);
         (*pContext->pOut) << "}\n";
      }

      void EmitIfElseChain(EmitContext* pContext, const Node& node, bool asElseIf)
      {
         const std::string condExpr = EmitInputExpression(pContext, node.id, "Cond");
         WriteIndent(pContext);
         if (asElseIf)
         {
            (*pContext->pOut) << "else if (" << condExpr << ")\n";
         }
         else
         {
            (*pContext->pOut) << "if (" << condExpr << ")\n";
         }
         WriteIndent(pContext);
         (*pContext->pOut) << "{\n";
         ++pContext->indentLevel;
         const Edge* pThen =
            pContext->pDocument->FindOutgoingEdge(node.id, "Then");
         if (pThen != nullptr)
         {
            EmitStatementChain(pContext, pThen->toNodeId);
         }
         --pContext->indentLevel;
         WriteIndent(pContext);
         (*pContext->pOut) << "}\n";

         const Edge* pElse =
            pContext->pDocument->FindOutgoingEdge(node.id, "Else");
         if (pElse == nullptr)
         {
            return;
         }

         const Node* pElseNode = GetNode(*pContext, pElse->toNodeId);
         if ((pElseNode != nullptr) && (pElseNode->type == BlockType::ElseIf))
         {
            EmitIfElseChain(pContext, *pElseNode, true);
            return;
         }

         WriteIndent(pContext);
         (*pContext->pOut) << "else\n";
         WriteIndent(pContext);
         (*pContext->pOut) << "{\n";
         ++pContext->indentLevel;
         EmitStatementChain(pContext, pElse->toNodeId);
         --pContext->indentLevel;
         WriteIndent(pContext);
         (*pContext->pOut) << "}\n";
      }

      void EmitSingleStatement(EmitContext* pContext, const Node& node)
      {
         switch (node.type)
         {
            case BlockType::VariableDecl:
            {
               const std::string name = GetProperty(node, "name", "value");
               const std::string typeName = GetProperty(node, "type", "int32_t");
               const std::string initExpr = EmitInputExpression(pContext, node.id, "Init");
               WriteIndent(pContext);
               (*pContext->pOut) << typeName << " " << name;
               if (!initExpr.empty())
               {
                  (*pContext->pOut) << " = " << initExpr;
               }
               (*pContext->pOut) << ";\n";
               break;
            }
            case BlockType::Assign:
            {
               const std::string target = GetProperty(node, "target", "value");
               const std::string valueExpr =
                  EmitInputExpression(pContext, node.id, "Value");
               WriteIndent(pContext);
               (*pContext->pOut) << target << " = " << valueExpr << ";\n";
               break;
            }
            case BlockType::CompoundAssign:
            {
               const std::string target = GetProperty(node, "target", "value");
               const std::string opText = GetProperty(node, "op", "+");
               const std::string valueExpr =
                  EmitInputExpression(pContext, node.id, "Value");
               WriteIndent(pContext);
               if (IsValidCompoundOp(opText))
               {
                  (*pContext->pOut) << target << " " << opText << "= " << valueExpr
                                    << ";\n";
               }
               else
               {
                  AppendDiag(pContext,
                             "CompoundAssign op must be one of + - * / %; using =.");
                  (*pContext->pOut) << target << " = " << valueExpr << ";\n";
               }
               break;
            }
            case BlockType::Inc:
            {
               const std::string target = GetProperty(node, "target", "value");
               WriteIndent(pContext);
               (*pContext->pOut) << "++" << target << ";\n";
               break;
            }
            case BlockType::Dec:
            {
               const std::string target = GetProperty(node, "target", "value");
               WriteIndent(pContext);
               (*pContext->pOut) << "--" << target << ";\n";
               break;
            }
            case BlockType::Printf:
            {
               const std::string formatRaw = GetProperty(node, "format", "value=%d\\n");
               const std::string formatText = EscapeCString(UnescapeFormat(formatRaw));
               WriteIndent(pContext);
               (*pContext->pOut) << "printf(\"" << formatText << "\"";
               for (int32_t argIndex = 0; argIndex < 6; ++argIndex)
               {
                  std::ostringstream portName;
                  portName << "Arg" << argIndex;
                  const std::string argExpr =
                     EmitInputExpression(pContext, node.id, portName.str());
                  if (!argExpr.empty())
                  {
                     (*pContext->pOut) << ", " << argExpr;
                  }
               }
               (*pContext->pOut) << ");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "fflush(stdout);\n";
               break;
            }
            case BlockType::WaitEnter:
            {
               const std::string promptRaw =
                  GetProperty(node, "prompt", "Press Enter to exit...\\n");
               const std::string promptText = EscapeCString(UnescapeFormat(promptRaw));
               WriteIndent(pContext);
               (*pContext->pOut) << "printf(\"" << promptText << "\");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "fflush(stdout);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "(void)getchar();\n";
               break;
            }
            case BlockType::ScanfInt:
            {
               const std::string target = GetProperty(node, "target", "value");
               const std::string promptRaw = GetProperty(node, "prompt", "Enter value: ");
               const std::string promptText = EscapeCString(UnescapeFormat(promptRaw));
               WriteIndent(pContext);
               (*pContext->pOut) << "printf(\"" << promptText << "\");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "fflush(stdout);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "if (scanf(\"%d\", &" << target << ") != 1)\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << target << " = 0;\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::ScanfChar:
            {
               const std::string target = GetProperty(node, "target", "ch");
               const std::string promptRaw =
                  GetProperty(node, "prompt", "Enter character: ");
               const std::string promptText = EscapeCString(UnescapeFormat(promptRaw));
               WriteIndent(pContext);
               (*pContext->pOut) << "printf(\"" << promptText << "\");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "fflush(stdout);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "if (scanf(\" %c\", &" << target << ") != 1)\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << target << " = 0;\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::ScanfLine:
            {
               const std::string target = GetProperty(node, "target", "buffer");
               const std::string sizeText = GetProperty(node, "size", "256");
               const std::string promptRaw =
                  GetProperty(node, "prompt", "Enter line: ");
               const std::string promptText = EscapeCString(UnescapeFormat(promptRaw));
               WriteIndent(pContext);
               (*pContext->pOut) << "printf(\"" << promptText << "\");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "fflush(stdout);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "if (fgets(" << target << ", " << sizeText
                                 << ", stdin) == NULL)\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << target << "[0] = '\\0';\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::ArrayDecl:
            {
               const std::string name = GetProperty(node, "name", "buffer");
               const std::string elemType = GetProperty(node, "elemType", "char");
               const std::string sizeText = GetProperty(node, "size", "256");
               WriteIndent(pContext);
               (*pContext->pOut) << elemType << " " << name << "[" << sizeText << "];\n";
               break;
            }
            case BlockType::IndexAssign:
            {
               const std::string arrayName = GetProperty(node, "array", "buffer");
               const std::string elemType = GetProperty(node, "elemType", "char");
               const std::string indexExpr =
                  EmitInputExpression(pContext, node.id, "Index");
               const std::string valueExpr =
                  EmitInputExpression(pContext, node.id, "Value");
               WriteIndent(pContext);
               (*pContext->pOut) << arrayName << "[" << indexExpr << "] = (" << elemType
                                 << ")(" << valueExpr << ");\n";
               break;
            }
            case BlockType::StrCpy:
            {
               const std::string destName = GetProperty(node, "dest", "dest");
               const std::string srcName = GetProperty(node, "src", "src");
               WriteIndent(pContext);
               (*pContext->pOut) << "strcpy(" << destName << ", " << srcName << ");\n";
               break;
            }
            case BlockType::StrNCpy:
            {
               const std::string destName = GetProperty(node, "dest", "dest");
               const std::string srcName = GetProperty(node, "src", "src");
               const std::string countText = GetProperty(node, "count", "256");
               WriteIndent(pContext);
               (*pContext->pOut) << "strncpy(" << destName << ", " << srcName << ", "
                                 << countText << ");\n";
               WriteIndent(pContext);
               (*pContext->pOut) << destName << "[(" << countText << ") - 1] = '\\0';\n";
               break;
            }
            case BlockType::ShuffleArray:
            {
               const std::string arrayName = GetProperty(node, "array", "buffer");
               const std::string lengthExpr =
                  EmitInputExpression(pContext, node.id, "Length");
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "int32_t cgenShuffleLen = (int32_t)(" << lengthExpr
                                 << ");\n";
               WriteIndent(pContext);
               (*pContext->pOut)
                  << "for (int32_t cgenShuffleIndex = cgenShuffleLen - 1; "
                     "cgenShuffleIndex > 0; --cgenShuffleIndex)\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut)
                  << "int32_t cgenShuffleSwap = rand() % (cgenShuffleIndex + 1);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "char cgenShuffleTemp = " << arrayName
                                 << "[cgenShuffleIndex];\n";
               WriteIndent(pContext);
               (*pContext->pOut) << arrayName << "[cgenShuffleIndex] = " << arrayName
                                 << "[cgenShuffleSwap];\n";
               WriteIndent(pContext);
               (*pContext->pOut) << arrayName << "[cgenShuffleSwap] = cgenShuffleTemp;\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::LocalTime:
            {
               const std::string yearName = GetProperty(node, "year", "year");
               const std::string monthName = GetProperty(node, "month", "month");
               const std::string dayName = GetProperty(node, "day", "day");
               const std::string hourName = GetProperty(node, "hour", "hour");
               const std::string minuteName = GetProperty(node, "minute", "minute");
               const std::string secondName = GetProperty(node, "second", "second");
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "time_t cgenTimeNow = time(NULL);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "struct tm* pCgenTm = localtime(&cgenTimeNow);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "if (pCgenTm != NULL)\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << yearName
                                 << " = (int32_t)(pCgenTm->tm_year + 1900);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << monthName
                                 << " = (int32_t)(pCgenTm->tm_mon + 1);\n";
               WriteIndent(pContext);
               (*pContext->pOut) << dayName << " = (int32_t)pCgenTm->tm_mday;\n";
               WriteIndent(pContext);
               (*pContext->pOut) << hourName << " = (int32_t)pCgenTm->tm_hour;\n";
               WriteIndent(pContext);
               (*pContext->pOut) << minuteName << " = (int32_t)pCgenTm->tm_min;\n";
               WriteIndent(pContext);
               (*pContext->pOut) << secondName << " = (int32_t)pCgenTm->tm_sec;\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::Sleep:
            {
               const std::string seconds = GetProperty(node, "seconds", "1");
               (*pContext->pOut) << "#ifdef _WIN32\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "Sleep((DWORD)(((" << seconds
                                 << ") * 1000)));\n";
               (*pContext->pOut) << "#else\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "sleep((unsigned int)(" << seconds << "));\n";
               (*pContext->pOut) << "#endif\n";
               break;
            }
            case BlockType::Free:
            {
               const std::string ptrExpr = EmitInputExpression(pContext, node.id, "Ptr");
               WriteIndent(pContext);
               (*pContext->pOut) << "free(" << ptrExpr << ");\n";
               break;
            }
            case BlockType::Return:
            {
               const std::string valueExpr =
                  EmitInputExpression(pContext, node.id, "Value");
               WriteIndent(pContext);
               if (valueExpr.empty())
               {
                  (*pContext->pOut) << "return;\n";
               }
               else
               {
                  (*pContext->pOut) << "return " << valueExpr << ";\n";
               }
               break;
            }
            case BlockType::Call:
            {
               const std::string storeTo = GetProperty(node, "storeTo", "");
               const std::string callExpr = EmitExpression(pContext, node.id);
               WriteIndent(pContext);
               if (!storeTo.empty())
               {
                  (*pContext->pOut) << storeTo << " = " << callExpr << ";\n";
               }
               else
               {
                  (*pContext->pOut) << callExpr << ";\n";
               }
               break;
            }
            case BlockType::If:
            case BlockType::ElseIf:
               EmitIfElseChain(pContext, node, false);
               break;
            case BlockType::Switch:
               EmitSwitchStatement(pContext, node);
               break;
            case BlockType::Case:
               AppendDiag(pContext,
                          "Case block must be reached via Switch Cases, not a linear chain.");
               break;
            case BlockType::Break:
               WriteIndent(pContext);
               (*pContext->pOut) << "break;\n";
               break;
            case BlockType::Continue:
               WriteIndent(pContext);
               (*pContext->pOut) << "continue;\n";
               break;
            case BlockType::While:
            {
               const std::string condExpr = EmitInputExpression(pContext, node.id, "Cond");
               WriteIndent(pContext);
               (*pContext->pOut) << "while (" << condExpr << ")\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               const Edge* pBody =
                  pContext->pDocument->FindOutgoingEdge(node.id, "Body");
               if (pBody != nullptr)
               {
                  EmitStatementChain(pContext, pBody->toNodeId);
               }
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::For:
            {
               const std::string iterator = GetProperty(node, "iterator", "i");
               const std::string start = GetProperty(node, "start", "0");
               const std::string end = GetProperty(node, "end", "10");
               const std::string typeName = GetProperty(node, "type", "int32_t");
               WriteIndent(pContext);
               (*pContext->pOut) << "for (" << typeName << " " << iterator << " = " << start
                                 << "; " << iterator << " < " << end << "; ++" << iterator
                                 << ")\n";
               WriteIndent(pContext);
               (*pContext->pOut) << "{\n";
               ++pContext->indentLevel;
               const Edge* pBody =
                  pContext->pDocument->FindOutgoingEdge(node.id, "Body");
               if (pBody != nullptr)
               {
                  EmitStatementChain(pContext, pBody->toNodeId);
               }
               --pContext->indentLevel;
               WriteIndent(pContext);
               (*pContext->pOut) << "}\n";
               break;
            }
            case BlockType::End:
               break;
            default:
               if (!IsExpressionBlock(node.type))
               {
                  AppendDiag(pContext,
                             std::string("Unsupported statement block: ") +
                                std::string(BlockTypeToString(node.type)));
               }
               break;
         }
      }
      NodeId NextLinearNode(const EmitContext& context, const Node& node)
      {
         const char* pPortName = "Next";
         if ((node.type == BlockType::While) || (node.type == BlockType::For))
         {
            pPortName = "Exit";
         }

         const Edge* pEdge =
            context.pDocument->FindOutgoingEdge(node.id, pPortName);
         if (pEdge == nullptr)
         {
            return 0;
         }
         return pEdge->toNodeId;
      }

      void EmitStatementChain(EmitContext* pContext, NodeId startNodeId)
      {
         NodeId currentId = startNodeId;
         std::unordered_set<NodeId> visited;
         while (currentId != 0)
         {
            if (visited.find(currentId) != visited.end())
            {
               AppendDiag(pContext, "Control-flow cycle detected outside loop body.");
               return;
            }
            visited.insert(currentId);

            const Node* pNode = GetNode(*pContext, currentId);
            if (pNode == nullptr)
            {
               AppendDiag(pContext, "Missing statement node in chain.");
               return;
            }
            if (pNode->type == BlockType::End)
            {
               return;
            }
            if (IsExpressionBlock(pNode->type))
            {
               AppendDiag(pContext, "Expression block used in control-flow chain.");
               return;
            }

            EmitSingleStatement(pContext, *pNode);

            if ((pNode->type == BlockType::Return) ||
                (pNode->type == BlockType::End) ||
                (pNode->type == BlockType::Break) ||
                (pNode->type == BlockType::Continue))
            {
               return;
            }

            currentId = NextLinearNode(*pContext, *pNode);
         }
      }

      void EmitGlobals(EmitContext* pContext)
      {
         const std::vector<Node>& nodes = pContext->pDocument->GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            const Node& node = nodes[index];
            if (node.type != BlockType::GlobalDecl)
            {
               continue;
            }
            const std::string name = GetProperty(node, "name", "gValue");
            const std::string typeName = GetProperty(node, "type", "int32_t");
            const std::string initExpr = EmitInputExpression(pContext, node.id, "Init");
            (*pContext->pOut) << typeName << " " << name;
            if (!initExpr.empty())
            {
               (*pContext->pOut) << " = " << initExpr;
            }
            (*pContext->pOut) << ";\n";
         }
      }

      void EmitFunctions(EmitContext* pContext)
      {
         const std::vector<Node>& nodes = pContext->pDocument->GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            const Node& node = nodes[index];
            if (node.type != BlockType::FunctionDef)
            {
               continue;
            }
            const std::string name = GetProperty(node, "name", "helper");
            const std::string returnType = GetProperty(node, "returnType", "int32_t");
            const std::string params = GetProperty(node, "params", "int32_t x");
            (*pContext->pOut) << returnType << " " << name << "(" << params << ")\n{\n";
            pContext->indentLevel = 1;
            const Edge* pBody =
               pContext->pDocument->FindOutgoingEdge(node.id, "Body");
            if (pBody != nullptr)
            {
               EmitStatementChain(pContext, pBody->toNodeId);
            }
            pContext->indentLevel = 0;
            (*pContext->pOut) << "}\n\n";
         }
      }

      const Node* FindStartNode(const GraphDocument& document)
      {
         const std::vector<Node>& nodes = document.GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            if (nodes[index].type == BlockType::Start)
            {
               return &nodes[index];
            }
         }
         return nullptr;
      }

      bool DocumentContainsBlockType(const GraphDocument& document, BlockType blockType)
      {
         const std::vector<Node>& nodes = document.GetNodes();
         for (size_t index = 0; index < nodes.size(); ++index)
         {
            if (nodes[index].type == blockType)
            {
               return true;
            }
         }
         return false;
      }
   } // namespace

   CodegenOutput GenerateCSource(const GraphDocument& document)
   {
      CodegenOutput output;
      std::ostringstream stream;
      EmitContext context;
      context.pDocument = &document;
      context.pOut = &stream;
      context.pDiagnostics = &output.diagnostics;

      const bool usesRandom =
         DocumentContainsBlockType(document, BlockType::Random) ||
         DocumentContainsBlockType(document, BlockType::RandomChar) ||
         DocumentContainsBlockType(document, BlockType::ShuffleArray);
      const bool usesLocalTime =
         DocumentContainsBlockType(document, BlockType::LocalTime);
      const bool usesSleep = DocumentContainsBlockType(document, BlockType::Sleep);
      const bool usesTimeHeader =
         usesRandom || usesLocalTime ||
         DocumentContainsBlockType(document, BlockType::TimeNow);
      const bool usesStringHeader =
         DocumentContainsBlockType(document, BlockType::StrLen) ||
         DocumentContainsBlockType(document, BlockType::StrCpy) ||
         DocumentContainsBlockType(document, BlockType::StrNCpy) ||
         DocumentContainsBlockType(document, BlockType::StrCmp);

      stream << "/* Generated by c_code_generator */\n";
      stream << "#include <stdint.h>\n";
      stream << "#include <stdio.h>\n";
      stream << "#include <stdlib.h>\n";
      if (usesStringHeader)
      {
         stream << "#include <string.h>\n";
      }
      if (usesTimeHeader)
      {
         stream << "#include <time.h>\n";
      }
      if (usesSleep)
      {
         stream << "#ifdef _WIN32\n";
         stream << "#include <windows.h>\n";
         stream << "#else\n";
         stream << "#include <unistd.h>\n";
         stream << "#endif\n";
      }
      stream << "\n";

      EmitGlobals(&context);
      stream << "\n";
      EmitFunctions(&context);

      const Node* pStart = FindStartNode(document);
      if (pStart == nullptr)
      {
         AppendDiag(&context, "Document has no Start block.");
         output.result = Result::Error;
         output.source = stream.str();
         return output;
      }

      stream << "int main(void)\n{\n";
      context.indentLevel = 1;
      if (usesRandom)
      {
         WriteIndent(&context);
         stream << "srand((unsigned int)time(NULL));\n";
      }

      const Edge* pNext = document.FindOutgoingEdge(pStart->id, "Next");
      if (pNext != nullptr)
      {
         EmitStatementChain(&context, pNext->toNodeId);
      }

      WriteIndent(&context);
      stream << "return 0;\n";
      stream << "}\n";

      output.source = stream.str();
      if (context.hadError)
      {
         output.result = Result::Error;
      }
      else
      {
         output.result = Result::Ok;
      }
      return output;
   }
} // namespace Cgen
