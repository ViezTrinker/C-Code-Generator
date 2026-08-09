/*!
 *\file graph_clipboard.h
 *\brief In-app copy/paste of flowchart subgraphs.
 */
#ifndef GRAPH_CLIPBOARD_H
#define GRAPH_CLIPBOARD_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "model/block_type.h"
#include "model/graph_document.h"
#include "model/node.h"

namespace Cgen
{
   /*!
    *\brief Copied node payload (id is local within the clipboard).
    */
   struct ClipboardNode
   {
      BlockType type = BlockType::End;
      float posX = 0.0f;
      float posY = 0.0f;
      PropertyMap properties;
      NodeId localId = 0;
   };

   /*!
    *\brief Copied edge using clipboard-local node ids.
    */
   struct ClipboardEdge
   {
      NodeId fromLocalId = 0;
      std::string fromPort;
      NodeId toLocalId = 0;
      std::string toPort;
   };

   /*!
    *\brief In-memory clipboard contents.
    */
   struct GraphClipboard
   {
      std::vector<ClipboardNode> nodes;
      std::vector<ClipboardEdge> edges;
      bool hasContent = false;
   };

   /*!
    *\brief Copies selected nodes (excluding Start) and internal edges.
    *
    *\param[in] document Source document.
    *\param[in] selectedNodeIds Selected node ids.
    *\param[out] pClipboard Clipboard to fill.
    */
   void CopySelectionToClipboard(const GraphDocument& document,
                                 const std::vector<NodeId>& selectedNodeIds,
                                 GraphClipboard* pClipboard);

   /*!
    *\brief Pastes clipboard contents into the document.
    *
    *\param[in,out] pDocument Destination document.
    *\param[in] clipboard Clipboard contents.
    *\param[in] offsetX Paste X offset.
    *\param[in] offsetY Paste Y offset.
    *\param[out] pOutPastedIds Optional list of new node ids.
    *\return true if anything was pasted.
    */
   bool PasteClipboardIntoDocument(GraphDocument* pDocument,
                                   const GraphClipboard& clipboard,
                                   float offsetX,
                                   float offsetY,
                                   std::vector<NodeId>* pOutPastedIds);
} // namespace Cgen

#endif // GRAPH_CLIPBOARD_H
