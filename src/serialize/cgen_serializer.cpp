/*!
 *\file cgen_serializer.cpp
 *\brief CGEN1 JSON document serialization.
 */
#include "serialize/cgen_serializer.h"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

namespace Cgen
{
   namespace
   {
      using Json = nlohmann::json;

      Json PortToJson(const Port& port)
      {
         Json json;
         json["name"] = port.name;
         json["kind"] = (port.kind == PortKind::Control) ? "Control" : "Data";
         json["direction"] = (port.direction == PortDirection::In) ? "In" : "Out";
         json["dataType"] = CTypeToString(port.dataType);
         return json;
      }

      bool PortFromJson(const Json& json, Port* pPort, std::string* pDiagnostics)
      {
         if (pPort == nullptr)
         {
            return false;
         }
         if ((!json.contains("name")) || (!json.contains("kind")) ||
             (!json.contains("direction")))
         {
            if (pDiagnostics != nullptr)
            {
               pDiagnostics->append("Port entry missing required fields.\n");
            }
            return false;
         }
         pPort->name = json.at("name").get<std::string>();
         const std::string kind = json.at("kind").get<std::string>();
         const std::string direction = json.at("direction").get<std::string>();
         if (kind == "Control")
         {
            pPort->kind = PortKind::Control;
         }
         else if (kind == "Data")
         {
            pPort->kind = PortKind::Data;
         }
         else
         {
            if (pDiagnostics != nullptr)
            {
               pDiagnostics->append("Unknown port kind.\n");
            }
            return false;
         }
         if (direction == "In")
         {
            pPort->direction = PortDirection::In;
         }
         else if (direction == "Out")
         {
            pPort->direction = PortDirection::Out;
         }
         else
         {
            if (pDiagnostics != nullptr)
            {
               pDiagnostics->append("Unknown port direction.\n");
            }
            return false;
         }
         if (json.contains("dataType"))
         {
            const std::string typeText = json.at("dataType").get<std::string>();
            if (!CTypeFromString(typeText, &pPort->dataType))
            {
               if (pDiagnostics != nullptr)
               {
                  pDiagnostics->append("Invalid port data type.\n");
               }
               return false;
            }
         }
         return true;
      }

      Json NodeToJson(const Node& node)
      {
         Json json;
         json["id"] = node.id;
         json["type"] = std::string(BlockTypeToString(node.type));
         json["posX"] = node.posX;
         json["posY"] = node.posY;
         Json ports = Json::array();
         for (size_t index = 0; index < node.ports.size(); ++index)
         {
            ports.push_back(PortToJson(node.ports[index]));
         }
         json["ports"] = ports;
         Json properties = Json::object();
         for (const auto& entry : node.properties)
         {
            properties[entry.first] = entry.second;
         }
         json["properties"] = properties;
         return json;
      }

      bool NodeFromJson(const Json& json, Node* pNode, std::string* pDiagnostics)
      {
         if (pNode == nullptr)
         {
            return false;
         }
         if ((!json.contains("id")) || (!json.contains("type")))
         {
            if (pDiagnostics != nullptr)
            {
               pDiagnostics->append("Node entry missing id/type.\n");
            }
            return false;
         }
         pNode->id = json.at("id").get<NodeId>();
         const std::string typeText = json.at("type").get<std::string>();
         if (!BlockTypeFromString(typeText, &pNode->type))
         {
            if (pDiagnostics != nullptr)
            {
               pDiagnostics->append("Unknown block type.\n");
            }
            return false;
         }
         pNode->posX = json.value("posX", 0.0f);
         pNode->posY = json.value("posY", 0.0f);
         pNode->ports.clear();
         if (json.contains("ports"))
         {
            for (const Json& portJson : json.at("ports"))
            {
               Port port;
               if (!PortFromJson(portJson, &port, pDiagnostics))
               {
                  return false;
               }
               pNode->ports.push_back(port);
            }
         }
         else
         {
            Node templated = CreateNode(pNode->id, pNode->type, pNode->posX, pNode->posY);
            pNode->ports = templated.ports;
         }
         pNode->properties.clear();
         if (json.contains("properties"))
         {
            for (auto iterator = json.at("properties").begin();
                 iterator != json.at("properties").end();
                 ++iterator)
            {
               pNode->properties[iterator.key()] = iterator.value().get<std::string>();
            }
         }
         return true;
      }

      Json DocumentToJson(const GraphDocument& document)
      {
         Json json;
         json["nextNodeId"] = document.GetNextNodeId();
         json["nextEdgeId"] = document.GetNextEdgeId();
         json["viewport"] = {
            {"x", document.GetViewportX()},
            {"y", document.GetViewportY()},
            {"zoom", document.GetViewportZoom()}
         };
         Json nodes = Json::array();
         const std::vector<Node>& nodeList = document.GetNodes();
         for (size_t index = 0; index < nodeList.size(); ++index)
         {
            nodes.push_back(NodeToJson(nodeList[index]));
         }
         json["nodes"] = nodes;
         Json edges = Json::array();
         const std::vector<Edge>& edgeList = document.GetEdges();
         for (size_t index = 0; index < edgeList.size(); ++index)
         {
            const Edge& edge = edgeList[index];
            edges.push_back({
               {"id", edge.id},
               {"fromNodeId", edge.fromNodeId},
               {"fromPort", edge.fromPort},
               {"toNodeId", edge.toNodeId},
               {"toPort", edge.toPort}
            });
         }
         json["edges"] = edges;
         json["types"] = Json::array();
         return json;
      }
   } // namespace

   Result SaveCgenFile(const GraphDocument& document, std::string_view filePath)
   {
      std::ofstream output(std::string(filePath), std::ios::binary);
      if (!output.is_open())
      {
         return Result::IoError;
      }
      output << "CGEN 1\n";
      output << DocumentToJson(document).dump(2);
      output << "\n";
      if (!output.good())
      {
         return Result::IoError;
      }
      return Result::Ok;
   }

   Result LoadCgenFile(std::string_view filePath,
                       GraphDocument* pDocument,
                       std::string* pDiagnostics)
   {
      if (pDocument == nullptr)
      {
         return Result::InvalidArgument;
      }
      std::ifstream input(std::string(filePath), std::ios::binary);
      if (!input.is_open())
      {
         return Result::IoError;
      }
      std::string header;
      std::getline(input, header);
      while ((!header.empty()) &&
             ((header.back() == '\r') || (header.back() == '\n') ||
              (header.back() == ' ') || (header.back() == '\t')))
      {
         header.pop_back();
      }
      if (header != "CGEN 1")
      {
         if (pDiagnostics != nullptr)
         {
            pDiagnostics->append("Unsupported or missing CGEN header.\n");
         }
         return Result::ParseError;
      }
      std::ostringstream bodyStream;
      bodyStream << input.rdbuf();
      Json json;
      try
      {
         json = Json::parse(bodyStream.str());
      }
      catch (const std::exception& exception)
      {
         if (pDiagnostics != nullptr)
         {
            pDiagnostics->append(exception.what());
            pDiagnostics->append("\n");
         }
         return Result::ParseError;
      }

      pDocument->GetNodesMutable().clear();
      pDocument->GetEdgesMutable().clear();

      if (json.contains("nodes"))
      {
         for (const Json& nodeJson : json.at("nodes"))
         {
            Node node;
            if (!NodeFromJson(nodeJson, &node, pDiagnostics))
            {
               return Result::ParseError;
            }
            pDocument->GetNodesMutable().push_back(node);
         }
      }

      if (json.contains("edges"))
      {
         for (const Json& edgeJson : json.at("edges"))
         {
            Edge edge;
            edge.id = edgeJson.at("id").get<EdgeId>();
            edge.fromNodeId = edgeJson.at("fromNodeId").get<NodeId>();
            edge.fromPort = edgeJson.at("fromPort").get<std::string>();
            edge.toNodeId = edgeJson.at("toNodeId").get<NodeId>();
            edge.toPort = edgeJson.at("toPort").get<std::string>();
            pDocument->GetEdgesMutable().push_back(edge);
         }
      }

      pDocument->SetNextNodeId(json.value("nextNodeId", static_cast<NodeId>(1)));
      pDocument->SetNextEdgeId(json.value("nextEdgeId", static_cast<EdgeId>(1)));
      if (json.contains("viewport"))
      {
         const Json& viewport = json.at("viewport");
         pDocument->SetViewport(viewport.value("x", 0.0f),
                                viewport.value("y", 0.0f),
                                viewport.value("zoom", 1.0f));
      }
      pDocument->SetFilePath(filePath);
      pDocument->SetDirty(false);
      return Result::Ok;
   }
} // namespace Cgen
