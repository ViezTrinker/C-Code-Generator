/*!
 *\file graph_document.h
 *\brief In-memory flowchart document (graph IR).
 */
#ifndef GRAPH_DOCUMENT_H
#define GRAPH_DOCUMENT_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "model/edge.h"
#include "model/node.h"
#include "model/result.h"

namespace Cgen
{
   /*!
    *\brief Complete editable flowchart document.
    */
   class GraphDocument
   {
   public:
      /*!
       *\brief Constructs an empty document with a Start node.
       */
      GraphDocument(void);

      /*!
       *\brief Clears the document and places a fresh Start node.
       */
      void Reset(void);

      /*!
       *\brief Adds a node of the given type.
       *
       *\param[in] blockType Block kind.
       *\param[in] posX Canvas X.
       *\param[in] posY Canvas Y.
       *\return New node id.
       */
      NodeId AddNode(BlockType blockType, float posX, float posY);

      /*!
       *\brief Removes a node and incident edges.
       *
       *\param[in] nodeId Node to remove.
       *\return Result::Ok or Result::NotFound.
       */
      Result RemoveNode(NodeId nodeId);

      /*!
       *\brief Connects two ports if the connection is valid.
       *
       *\param[in] fromNodeId Source node.
       *\param[in] fromPort Source port name.
       *\param[in] toNodeId Destination node.
       *\param[in] toPort Destination port name.
       *\param[out] pOutEdgeId Optional new edge id.
       *\return Result code.
       */
      Result Connect(NodeId fromNodeId,
                     std::string_view fromPort,
                     NodeId toNodeId,
                     std::string_view toPort,
                     EdgeId* pOutEdgeId);

      /*!
       *\brief Removes an edge by id.
       *
       *\param[in] edgeId Edge to remove.
       *\return Result::Ok or Result::NotFound.
       */
      Result RemoveEdge(EdgeId edgeId);

      /*!
       *\brief Finds a node by id.
       *
       *\param[in] nodeId Node id.
       *\return Pointer or nullptr.
       */
      const Node* FindNode(NodeId nodeId) const;

      /*!
       *\brief Finds a mutable node by id.
       *
       *\param[in] nodeId Node id.
       *\return Pointer or nullptr.
       */
      Node* FindNodeMutable(NodeId nodeId);

      /*!
       *\brief Finds the edge feeding a given input port.
       *
       *\param[in] nodeId Destination node.
       *\param[in] portName Destination port.
       *\return Pointer or nullptr.
       */
      const Edge* FindIncomingEdge(NodeId nodeId, std::string_view portName) const;

      /*!
       *\brief Finds the edge leaving a given output port.
       *
       *\param[in] nodeId Source node.
       *\param[in] portName Source port.
       *\return Pointer or nullptr.
       */
      const Edge* FindOutgoingEdge(NodeId nodeId, std::string_view portName) const;

      /*!
       *\brief Accesses all nodes.
       */
      const std::vector<Node>& GetNodes(void) const;

      /*!
       *\brief Accesses mutable nodes.
       */
      std::vector<Node>& GetNodesMutable(void);

      /*!
       *\brief Accesses all edges.
       */
      const std::vector<Edge>& GetEdges(void) const;

      /*!
       *\brief Accesses mutable edges.
       */
      std::vector<Edge>& GetEdgesMutable(void);

      /*!
       *\brief Viewport pan X.
       */
      float GetViewportX(void) const;

      /*!
       *\brief Viewport pan Y.
       */
      float GetViewportY(void) const;

      /*!
       *\brief Viewport zoom factor.
       */
      float GetViewportZoom(void) const;

      /*!
       *\brief Sets viewport transform.
       *
       *\param[in] viewportX Pan X.
       *\param[in] viewportY Pan Y.
       *\param[in] viewportZoom Zoom.
       */
      void SetViewport(float viewportX, float viewportY, float viewportZoom);

      /*!
       *\brief Dirty flag for unsaved edits.
       */
      bool IsDirty(void) const;

      /*!
       *\brief Sets dirty flag.
       *
       *\param[in] dirty New dirty state.
       */
      void SetDirty(bool dirty);

      /*!
       *\brief Associated .cgen path if any.
       */
      const std::string& GetFilePath(void) const;

      /*!
       *\brief Sets associated file path.
       *
       *\param[in] filePath Path string.
       */
      void SetFilePath(std::string_view filePath);

      /*!
       *\brief Next node id counter (serialization).
       */
      NodeId GetNextNodeId(void) const;

      /*!
       *\brief Sets next node id counter.
       *
       *\param[in] nextNodeId Counter value.
       */
      void SetNextNodeId(NodeId nextNodeId);

      /*!
       *\brief Next edge id counter (serialization).
       */
      EdgeId GetNextEdgeId(void) const;

      /*!
       *\brief Sets next edge id counter.
       *
       *\param[in] nextEdgeId Counter value.
       */
      void SetNextEdgeId(EdgeId nextEdgeId);

   private:
      std::vector<Node> _nodes;
      std::vector<Edge> _edges;
      NodeId _nextNodeId = 1;
      EdgeId _nextEdgeId = 1;
      float _viewportX = 0.0f;
      float _viewportY = 0.0f;
      float _viewportZoom = 1.0f;
      bool _dirty = false;
      std::string _filePath;
   };
} // namespace Cgen

#endif // GRAPH_DOCUMENT_H
