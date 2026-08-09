/*!
 *\file graph_document.cpp
 *\brief Graph document IR implementation.
 */
#include "model/graph_document.h"

#include <cstddef>

namespace Cgen
{
   GraphDocument::GraphDocument(void)
   {
      Reset();
   }

   void GraphDocument::Reset(void)
   {
      _nodes.clear();
      _edges.clear();
      _nextNodeId = 1;
      _nextEdgeId = 1;
      _viewportX = 0.0f;
      _viewportY = 0.0f;
      _viewportZoom = 1.0f;
      _dirty = false;
      _filePath.clear();
      _fileDescription.clear();
      _clangFormatOnGenerate = ClangFormatOnGenerate::No;
      AddNode(BlockType::Start, 80.0f, 80.0f);
      _dirty = false;
   }

   NodeId GraphDocument::AddNode(BlockType blockType, float posX, float posY)
   {
      const NodeId id = _nextNodeId;
      ++_nextNodeId;
      _nodes.push_back(CreateNode(id, blockType, posX, posY));
      _dirty = true;
      return id;
   }

   Result GraphDocument::RemoveNode(NodeId nodeId)
   {
      const Node* pNode = FindNode(nodeId);
      if (pNode == nullptr)
      {
         return Result::NotFound;
      }
      if (pNode->type == BlockType::Start)
      {
         return Result::InvalidArgument;
      }

      size_t edgeIndex = 0;
      while (edgeIndex < _edges.size())
      {
         if ((_edges[edgeIndex].fromNodeId == nodeId) ||
             (_edges[edgeIndex].toNodeId == nodeId))
         {
            _edges.erase(_edges.begin() + static_cast<std::ptrdiff_t>(edgeIndex));
         }
         else
         {
            ++edgeIndex;
         }
      }

      for (size_t nodeIndex = 0; nodeIndex < _nodes.size(); ++nodeIndex)
      {
         if (_nodes[nodeIndex].id == nodeId)
         {
            _nodes.erase(_nodes.begin() + static_cast<std::ptrdiff_t>(nodeIndex));
            break;
         }
      }
      _dirty = true;
      return Result::Ok;
   }

   Result GraphDocument::Connect(NodeId fromNodeId,
                                 std::string_view fromPort,
                                 NodeId toNodeId,
                                 std::string_view toPort,
                                 EdgeId* pOutEdgeId)
   {
      if (fromNodeId == toNodeId)
      {
         return Result::InvalidArgument;
      }

      Node* pFromNode = FindNodeMutable(fromNodeId);
      Node* pToNode = FindNodeMutable(toNodeId);
      if ((pFromNode == nullptr) || (pToNode == nullptr))
      {
         return Result::NotFound;
      }

      Port* pFrom = FindPortMutable(pFromNode, fromPort);
      Port* pTo = FindPortMutable(pToNode, toPort);
      if ((pFrom == nullptr) || (pTo == nullptr))
      {
         return Result::NotFound;
      }
      if (pFrom->direction != PortDirection::Out)
      {
         return Result::InvalidArgument;
      }
      if (pTo->direction != PortDirection::In)
      {
         return Result::InvalidArgument;
      }
      if (pFrom->kind != pTo->kind)
      {
         return Result::InvalidArgument;
      }
      if ((pFrom->kind == PortKind::Data) &&
          (!AreTypesCompatible(pFrom->dataType, pTo->dataType)))
      {
         return Result::InvalidArgument;
      }
      if (FindOutgoingEdge(fromNodeId, fromPort) != nullptr)
      {
         return Result::InvalidArgument;
      }
      if (FindIncomingEdge(toNodeId, toPort) != nullptr)
      {
         return Result::InvalidArgument;
      }

      Edge edge;
      edge.id = _nextEdgeId;
      ++_nextEdgeId;
      edge.fromNodeId = fromNodeId;
      edge.fromPort = std::string(fromPort);
      edge.toNodeId = toNodeId;
      edge.toPort = std::string(toPort);
      _edges.push_back(edge);
      if (pOutEdgeId != nullptr)
      {
         *pOutEdgeId = edge.id;
      }
      _dirty = true;
      return Result::Ok;
   }

   Result GraphDocument::RemoveEdge(EdgeId edgeId)
   {
      for (size_t index = 0; index < _edges.size(); ++index)
      {
         if (_edges[index].id == edgeId)
         {
            _edges.erase(_edges.begin() + static_cast<std::ptrdiff_t>(index));
            _dirty = true;
            return Result::Ok;
         }
      }
      return Result::NotFound;
   }

   const Node* GraphDocument::FindNode(NodeId nodeId) const
   {
      for (size_t index = 0; index < _nodes.size(); ++index)
      {
         if (_nodes[index].id == nodeId)
         {
            return &_nodes[index];
         }
      }
      return nullptr;
   }

   Node* GraphDocument::FindNodeMutable(NodeId nodeId)
   {
      for (size_t index = 0; index < _nodes.size(); ++index)
      {
         if (_nodes[index].id == nodeId)
         {
            return &_nodes[index];
         }
      }
      return nullptr;
   }

   const Edge* GraphDocument::FindIncomingEdge(NodeId nodeId,
                                               std::string_view portName) const
   {
      for (size_t index = 0; index < _edges.size(); ++index)
      {
         if ((_edges[index].toNodeId == nodeId) &&
             (_edges[index].toPort == portName))
         {
            return &_edges[index];
         }
      }
      return nullptr;
   }

   const Edge* GraphDocument::FindOutgoingEdge(NodeId nodeId,
                                               std::string_view portName) const
   {
      for (size_t index = 0; index < _edges.size(); ++index)
      {
         if ((_edges[index].fromNodeId == nodeId) &&
             (_edges[index].fromPort == portName))
         {
            return &_edges[index];
         }
      }
      return nullptr;
   }

   const std::vector<Node>& GraphDocument::GetNodes(void) const
   {
      return _nodes;
   }

   std::vector<Node>& GraphDocument::GetNodesMutable(void)
   {
      return _nodes;
   }

   const std::vector<Edge>& GraphDocument::GetEdges(void) const
   {
      return _edges;
   }

   std::vector<Edge>& GraphDocument::GetEdgesMutable(void)
   {
      return _edges;
   }

   float GraphDocument::GetViewportX(void) const
   {
      return _viewportX;
   }

   float GraphDocument::GetViewportY(void) const
   {
      return _viewportY;
   }

   float GraphDocument::GetViewportZoom(void) const
   {
      return _viewportZoom;
   }

   void GraphDocument::SetViewport(float viewportX,
                                   float viewportY,
                                   float viewportZoom)
   {
      _viewportX = viewportX;
      _viewportY = viewportY;
      _viewportZoom = viewportZoom;
   }

   bool GraphDocument::IsDirty(void) const
   {
      return _dirty;
   }

   void GraphDocument::SetDirty(bool dirty)
   {
      _dirty = dirty;
   }

   const std::string& GraphDocument::GetFilePath(void) const
   {
      return _filePath;
   }

   void GraphDocument::SetFilePath(std::string_view filePath)
   {
      _filePath = std::string(filePath);
   }

   NodeId GraphDocument::GetNextNodeId(void) const
   {
      return _nextNodeId;
   }

   void GraphDocument::SetNextNodeId(NodeId nextNodeId)
   {
      _nextNodeId = nextNodeId;
   }

   EdgeId GraphDocument::GetNextEdgeId(void) const
   {
      return _nextEdgeId;
   }

   void GraphDocument::SetNextEdgeId(EdgeId nextEdgeId)
   {
      _nextEdgeId = nextEdgeId;
   }

   const std::string& GraphDocument::GetFileDescription(void) const
   {
      return _fileDescription;
   }

   void GraphDocument::SetFileDescription(std::string_view description)
   {
      _fileDescription = std::string(description);
   }

   GraphDocument::ClangFormatOnGenerate GraphDocument::GetClangFormatOnGenerate(void) const
   {
      return _clangFormatOnGenerate;
   }

   void GraphDocument::SetClangFormatOnGenerate(ClangFormatOnGenerate value)
   {
      _clangFormatOnGenerate = value;
   }

   GraphSnapshot GraphDocument::CaptureGraph(void) const
   {
      GraphSnapshot snapshot;
      snapshot.nodes = _nodes;
      snapshot.edges = _edges;
      snapshot.nextNodeId = _nextNodeId;
      snapshot.nextEdgeId = _nextEdgeId;
      snapshot.dirty = _dirty;
      snapshot.fileDescription = _fileDescription;
      snapshot.clangFormatOnGenerate =
         (_clangFormatOnGenerate == ClangFormatOnGenerate::Yes);
      return snapshot;
   }

   void GraphDocument::RestoreGraph(const GraphSnapshot& snapshot)
   {
      _nodes = snapshot.nodes;
      _edges = snapshot.edges;
      _nextNodeId = snapshot.nextNodeId;
      _nextEdgeId = snapshot.nextEdgeId;
      _dirty = snapshot.dirty;
      _fileDescription = snapshot.fileDescription;
      _clangFormatOnGenerate =
         snapshot.clangFormatOnGenerate ? ClangFormatOnGenerate::Yes
                                        : ClangFormatOnGenerate::No;
   }
} // namespace Cgen
