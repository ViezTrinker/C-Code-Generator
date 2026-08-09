/*!
 *\file canvas_view.cpp
 *\brief Flowchart canvas interaction and rendering.
 */
#include "gui/canvas_view.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

#include <SFML/Window/Keyboard.hpp>

#include "gui/block_placement.h"
#include "model/c_type.h"
#include "model/graph_layout.h"
#include "model/node.h"
#include "model/result.h"

namespace Cgen
{
   namespace
   {
      constexpr float PortRadius = 6.0f;
      constexpr float MarqueeThreshold = 4.0f;
      constexpr float WheelPanPixels = 48.0f;
      constexpr float KeyPanPixels = 64.0f;
      constexpr float MinViewportZoom = 0.15f;
      constexpr float MaxViewportZoom = 2.5f;
      constexpr float FitPaddingWorld = 48.0f;
      constexpr float MinimapWidth = 176.0f;
      constexpr float MinimapHeight = 120.0f;
      constexpr float MinimapMargin = 10.0f;
      constexpr float MinimapContentPad = 24.0f;
      constexpr float DoubleClickSeconds = 0.35f;

      bool IsShiftHeld(void)
      {
         return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
      }

      bool IsCtrlHeld(void)
      {
         return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
      }

      bool IsSpaceHeld(void)
      {
         return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
      }

      sf::FloatRect NormalizeRect(sf::Vector2f a, sf::Vector2f b)
      {
         const float left = std::min(a.x, b.x);
         const float top = std::min(a.y, b.y);
         const float width = std::fabs(a.x - b.x);
         const float height = std::fabs(a.y - b.y);
         return sf::FloatRect(sf::Vector2f(left, top), sf::Vector2f(width, height));
      }
   } // namespace

   CanvasView::CanvasView(const sf::Font& font)
      : _pFont(&font)
   {
   }

   void CanvasView::SetBounds(const sf::FloatRect& bounds)
   {
      _bounds = bounds;
   }

   void CanvasView::SetDocument(GraphDocument* pDocument)
   {
      _pDocument = pDocument;
      ClearSelection();
      _wireStart.reset();
      ClearWireHoverFeedback();
      _isDraggingNode = false;
      _isPanning = false;
      _isMarquee = false;
      _dragCheckpointTaken = false;
      _pasteCascade = 0;
   }

   void CanvasView::SetHistory(DocumentHistory* pHistory)
   {
      _pHistory = pHistory;
   }

   bool CanvasView::Contains(sf::Vector2f point) const
   {
      return _bounds.contains(point);
   }

   void CanvasView::ClearSelection(void)
   {
      _selectedNodeIds.clear();
   }

   void CanvasView::AddToSelection(NodeId nodeId)
   {
      if ((nodeId == 0) || IsNodeSelected(nodeId))
      {
         return;
      }
      _selectedNodeIds.push_back(nodeId);
   }

   void CanvasView::RemoveFromSelection(NodeId nodeId)
   {
      for (size_t index = 0; index < _selectedNodeIds.size(); ++index)
      {
         if (_selectedNodeIds[index] == nodeId)
         {
            _selectedNodeIds.erase(_selectedNodeIds.begin() +
                                   static_cast<std::ptrdiff_t>(index));
            return;
         }
      }
   }

   void CanvasView::ToggleSelection(NodeId nodeId)
   {
      if (IsNodeSelected(nodeId))
      {
         RemoveFromSelection(nodeId);
      }
      else
      {
         AddToSelection(nodeId);
      }
   }

   bool CanvasView::IsNodeSelected(NodeId nodeId) const
   {
      for (size_t index = 0; index < _selectedNodeIds.size(); ++index)
      {
         if (_selectedNodeIds[index] == nodeId)
         {
            return true;
         }
      }
      return false;
   }

   NodeId CanvasView::GetSelectedNodeId(void) const
   {
      if (_selectedNodeIds.empty())
      {
         return 0;
      }
      return _selectedNodeIds.back();
   }

   const std::vector<NodeId>& CanvasView::GetSelectedNodeIds(void) const
   {
      return _selectedNodeIds;
   }

   void CanvasView::SetSelectedNodeId(NodeId nodeId)
   {
      ClearSelection();
      if (nodeId != 0)
      {
         _selectedNodeIds.push_back(nodeId);
      }
   }

   void CanvasView::SetSelectedNodeIds(const std::vector<NodeId>& nodeIds)
   {
      _selectedNodeIds = nodeIds;
   }

   void CanvasView::CenterOnNode(NodeId nodeId)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const Node* pNode = _pDocument->FindNode(nodeId);
      if (pNode == nullptr)
      {
         return;
      }
      const float zoom = _pDocument->GetViewportZoom();
      const float nodeHeight = ComputeBlockNodeHeight(*pNode);
      const float centerWorldX = pNode->posX + (BlockNodeWidth * 0.5f);
      const float centerWorldY = pNode->posY + (nodeHeight * 0.5f);
      const float viewW = _bounds.size.x / zoom;
      const float viewH = _bounds.size.y / zoom;
      _pDocument->SetViewport(centerWorldX - (viewW * 0.5f),
                              centerWorldY - (viewH * 0.5f),
                              zoom);
   }

   void CanvasView::FitNodes(const std::vector<NodeId>& nodeIds)
   {
      if ((_pDocument == nullptr) || nodeIds.empty() || (_bounds.size.x <= 1.0f) ||
          (_bounds.size.y <= 1.0f))
      {
         return;
      }

      float minX = 0.0f;
      float minY = 0.0f;
      float maxX = 0.0f;
      float maxY = 0.0f;
      bool hasBounds = false;
      for (size_t index = 0; index < nodeIds.size(); ++index)
      {
         const Node* pNode = _pDocument->FindNode(nodeIds[index]);
         if (pNode == nullptr)
         {
            continue;
         }
         const float nodeHeight = ComputeBlockNodeHeight(*pNode);
         const float nodeMaxX = pNode->posX + BlockNodeWidth;
         const float nodeMaxY = pNode->posY + nodeHeight;
         if (!hasBounds)
         {
            minX = pNode->posX;
            minY = pNode->posY;
            maxX = nodeMaxX;
            maxY = nodeMaxY;
            hasBounds = true;
         }
         else
         {
            minX = std::min(minX, pNode->posX);
            minY = std::min(minY, pNode->posY);
            maxX = std::max(maxX, nodeMaxX);
            maxY = std::max(maxY, nodeMaxY);
         }
      }
      if (!hasBounds)
      {
         return;
      }

      minX -= FitPaddingWorld;
      minY -= FitPaddingWorld;
      maxX += FitPaddingWorld;
      maxY += FitPaddingWorld;

      const float worldWidth = std::max(1.0f, maxX - minX);
      const float worldHeight = std::max(1.0f, maxY - minY);
      float zoom = std::min(_bounds.size.x / worldWidth, _bounds.size.y / worldHeight);
      if (zoom < MinViewportZoom)
      {
         zoom = MinViewportZoom;
      }
      if (zoom > MaxViewportZoom)
      {
         zoom = MaxViewportZoom;
      }

      const float viewW = _bounds.size.x / zoom;
      const float viewH = _bounds.size.y / zoom;
      const float centerX = (minX + maxX) * 0.5f;
      const float centerY = (minY + maxY) * 0.5f;
      _pDocument->SetViewport(centerX - (viewW * 0.5f),
                              centerY - (viewH * 0.5f),
                              zoom);
   }

   void CanvasView::FitAllNodes(void)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      std::vector<NodeId> nodeIds;
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      nodeIds.reserve(nodes.size());
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         nodeIds.push_back(nodes[index].id);
      }
      FitNodes(nodeIds);
   }

   void CanvasView::FitSelection(void)
   {
      if (_selectedNodeIds.empty())
      {
         FitAllNodes();
         return;
      }
      FitNodes(_selectedNodeIds);
   }

   void CanvasView::SelectAll(void)
   {
      ClearSelection();
      if (_pDocument == nullptr)
      {
         return;
      }
      for (const Node& node : _pDocument->GetNodes())
      {
         _selectedNodeIds.push_back(node.id);
      }
   }

   void CanvasView::CopySelection(void)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      CopySelectionToClipboard(*_pDocument, _selectedNodeIds, &_clipboard);
      _pasteCascade = 0;
   }

   void CanvasView::PasteClipboard(void)
   {
      if ((_pDocument == nullptr) || (!_clipboard.hasContent) ||
          _clipboard.nodes.empty())
      {
         return;
      }
      ++_pasteCascade;

      float minX = _clipboard.nodes.front().posX;
      float minY = _clipboard.nodes.front().posY;
      float maxX = minX;
      float maxY = minY;
      float tallestHeight = BlockNodeHeight;
      for (size_t index = 0; index < _clipboard.nodes.size(); ++index)
      {
         const ClipboardNode& item = _clipboard.nodes[index];
         if (item.posX < minX)
         {
            minX = item.posX;
         }
         if (item.posY < minY)
         {
            minY = item.posY;
         }
         if (item.posX > maxX)
         {
            maxX = item.posX;
         }
         if (item.posY > maxY)
         {
            maxY = item.posY;
         }
         const Node probe = CreateNode(0, item.type, 0.0f, 0.0f);
         const float itemHeight = ComputeBlockNodeHeight(probe);
         if (itemHeight > tallestHeight)
         {
            tallestHeight = itemHeight;
         }
      }
      const float groupWidth = (maxX - minX) + BlockNodeWidth + BlockPlacementGap;
      const float groupHeight = (maxY - minY) + tallestHeight + BlockPlacementGap;
      const float cascade = static_cast<float>(_pasteCascade);
      const float preferredOffsetX = groupWidth * cascade;
      const float preferredOffsetY = groupHeight * cascade;

      WorldPosition preferred;
      preferred.x = _clipboard.nodes.front().posX + preferredOffsetX;
      preferred.y = _clipboard.nodes.front().posY + preferredOffsetY;
      const WorldPosition freeOrigin =
         FindFreeBlockWorldPosition(preferred, _pDocument->GetNodes());
      const float adjustedOffsetX = freeOrigin.x - _clipboard.nodes.front().posX;
      const float adjustedOffsetY = freeOrigin.y - _clipboard.nodes.front().posY;

      PushCheckpoint();
      std::vector<NodeId> pasted;
      if (!PasteClipboardIntoDocument(
             _pDocument, _clipboard, adjustedOffsetX, adjustedOffsetY, &pasted))
      {
         return;
      }
      _selectedNodeIds = pasted;
   }

   void CanvasView::TidyLayout(void)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      PushCheckpoint();
      ApplyAutoLayout(_pDocument);
   }

   void CanvasView::PushCheckpoint(void)
   {
      if ((_pHistory != nullptr) && (_pDocument != nullptr))
      {
         _pHistory->PushCheckpoint(*_pDocument);
      }
   }

   sf::Vector2f CanvasView::ScreenToWorld(sf::Vector2f screenPoint) const
   {
      if (_pDocument == nullptr)
      {
         return screenPoint;
      }
      const float zoom = _pDocument->GetViewportZoom();
      const float worldX =
         ((screenPoint.x - _bounds.position.x) / zoom) + _pDocument->GetViewportX();
      const float worldY =
         ((screenPoint.y - _bounds.position.y) / zoom) + _pDocument->GetViewportY();
      return sf::Vector2f(worldX, worldY);
   }

   sf::Vector2f CanvasView::WorldToScreen(sf::Vector2f worldPoint) const
   {
      if (_pDocument == nullptr)
      {
         return worldPoint;
      }
      const float zoom = _pDocument->GetViewportZoom();
      const float screenX =
         ((worldPoint.x - _pDocument->GetViewportX()) * zoom) + _bounds.position.x;
      const float screenY =
         ((worldPoint.y - _pDocument->GetViewportY()) * zoom) + _bounds.position.y;
      return sf::Vector2f(screenX, screenY);
   }

   sf::FloatRect CanvasView::NodeBounds(const Node& node) const
   {
      return sf::FloatRect(sf::Vector2f(node.posX, node.posY),
                           sf::Vector2f(BlockNodeWidth, ComputeBlockNodeHeight(node)));
   }

   sf::Vector2f CanvasView::PortWorldPosition(const Node& node, size_t portIndex) const
   {
      const Port& port = node.ports[portIndex];
      float offsetY = BlockPortTopOffset;
      size_t sameSideIndex = 0;
      for (size_t index = 0; index < portIndex; ++index)
      {
         if (!node.ports[index].visible)
         {
            continue;
         }
         if (node.ports[index].direction == port.direction)
         {
            ++sameSideIndex;
         }
      }
      offsetY += static_cast<float>(sameSideIndex) * BlockPortSpacing;
      if (port.direction == PortDirection::In)
      {
         return sf::Vector2f(node.posX, node.posY + offsetY);
      }
      return sf::Vector2f(node.posX + BlockNodeWidth, node.posY + offsetY);
   }

   bool CanvasView::HitTestPort(sf::Vector2f worldPoint, PortHit* pOutHit) const
   {
      if ((pOutHit == nullptr) || (_pDocument == nullptr))
      {
         return false;
      }
      const float hitRadius = PortRadius + 4.0f;
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
      {
         const Node& node = nodes[nodeIndex];
         if (IsNodeHiddenByCollapse(node.id))
         {
            continue;
         }
         for (size_t portIndex = 0; portIndex < node.ports.size(); ++portIndex)
         {
            if (!node.ports[portIndex].visible)
            {
               continue;
            }
            const sf::Vector2f portPos = PortWorldPosition(node, portIndex);
            const float deltaX = worldPoint.x - portPos.x;
            const float deltaY = worldPoint.y - portPos.y;
            const float distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));
            if (distance <= hitRadius)
            {
               pOutHit->nodeId = node.id;
               pOutHit->portName = node.ports[portIndex].name;
               pOutHit->worldPosition = portPos;
               pOutHit->kind = node.ports[portIndex].kind;
               pOutHit->direction = node.ports[portIndex].direction;
               pOutHit->dataType = node.ports[portIndex].dataType;
               return true;
            }
         }
      }
      return false;
   }

   NodeId CanvasView::HitTestNode(sf::Vector2f worldPoint) const
   {
      if (_pDocument == nullptr)
      {
         return 0;
      }
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const size_t reverseIndex = nodes.size() - 1 - index;
         if (IsNodeHiddenByCollapse(nodes[reverseIndex].id))
         {
            continue;
         }
         if (NodeBounds(nodes[reverseIndex]).contains(worldPoint))
         {
            return nodes[reverseIndex].id;
         }
      }
      return 0;
   }

   bool CanvasView::QueryHit(sf::Vector2f screenPoint, CanvasHitInfo* pOutHit) const
   {
      if ((pOutHit == nullptr) || (_pDocument == nullptr) || (!_bounds.contains(screenPoint)))
      {
         return false;
      }

      pOutHit->kind = CanvasHitKind::Empty;
      pOutHit->nodeId = 0;
      pOutHit->edgeId = 0;

      const sf::Vector2f world = ScreenToWorld(screenPoint);
      PortHit portHit;
      if (HitTestPort(world, &portHit))
      {
         const Edge* pOutgoing =
            _pDocument->FindOutgoingEdge(portHit.nodeId, portHit.portName);
         const Edge* pIncoming =
            _pDocument->FindIncomingEdge(portHit.nodeId, portHit.portName);
         const Edge* pEdge = (pOutgoing != nullptr) ? pOutgoing : pIncoming;
         if (pEdge != nullptr)
         {
            pOutHit->kind = CanvasHitKind::Wire;
            pOutHit->edgeId = pEdge->id;
            pOutHit->nodeId = portHit.nodeId;
            return true;
         }
      }

      const NodeId hitNode = HitTestNode(world);
      if (hitNode != 0)
      {
         pOutHit->kind = CanvasHitKind::Node;
         pOutHit->nodeId = hitNode;
         return true;
      }

      return true;
   }

   void CanvasView::PlaceBlock(BlockType blockType, sf::Vector2f screenPoint)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      if (!_bounds.contains(screenPoint))
      {
         const sf::Vector2f centerScreen(
            _bounds.position.x + (_bounds.size.x * 0.5f),
            _bounds.position.y + (_bounds.size.y * 0.5f));
         screenPoint = centerScreen;
      }
      PushCheckpoint();
      const sf::Vector2f preferredScreen = ScreenToWorld(screenPoint);
      WorldPosition preferred;
      preferred.x = preferredScreen.x;
      preferred.y = preferredScreen.y;
      const Node probe = CreateNode(0, blockType, 0.0f, 0.0f);
      const float proposedHeight = ComputeBlockNodeHeight(probe);
      const WorldPosition world = FindFreeBlockWorldPosition(
         preferred, _pDocument->GetNodes(), proposedHeight);
      const NodeId id = _pDocument->AddNode(blockType, world.x, world.y);
      SetSelectedNodeId(id);
   }

   bool CanvasView::HandleMousePress(sf::Mouse::Button button, sf::Vector2f screenPoint)
   {
      if ((_pDocument == nullptr) || (!_bounds.contains(screenPoint)))
      {
         return false;
      }
      _lastScreenPoint = screenPoint;
      const sf::Vector2f world = ScreenToWorld(screenPoint);

      if (button == sf::Mouse::Button::Middle)
      {
         _isPanning = true;
         _isMarquee = false;
         _isMinimapDragging = false;
         return true;
      }

      if (button == sf::Mouse::Button::Right)
      {
         return true;
      }

      if (button != sf::Mouse::Button::Left)
      {
         return false;
      }

      if (HandleMinimapPress(screenPoint))
      {
         return true;
      }

      if (IsSpaceHeld())
      {
         _isPanning = true;
         _isMarquee = false;
         _isDraggingNode = false;
         _isMinimapDragging = false;
         return true;
      }

      PortHit portHit;
      if (HitTestPort(world, &portHit))
      {
         if (portHit.direction == PortDirection::Out)
         {
            _wireStart = portHit;
            _wirePreviewWorld = world;
            ClearWireHoverFeedback();
         }
         else if (_wireStart.has_value())
         {
            TryConnectWire(*_wireStart, portHit);
            _wireStart.reset();
            ClearWireHoverFeedback();
         }
         return true;
      }

      const NodeId hitNode = HitTestNode(world);
      if (hitNode != 0)
      {
         const Node* pHitNode = _pDocument->FindNode(hitNode);
         if ((pHitNode != nullptr) && (pHitNode->type == BlockType::FunctionDef) &&
             (_lastClickNodeId == hitNode) &&
             (_lastClickClock.getElapsedTime().asSeconds() <= DoubleClickSeconds))
         {
            ToggleFunctionCollapsed(hitNode);
            _lastClickNodeId = 0;
            return true;
         }
         _lastClickNodeId = hitNode;
         _lastClickClock.restart();

         if (IsShiftHeld())
         {
            ToggleSelection(hitNode);
         }
         else if (!IsNodeSelected(hitNode))
         {
            SetSelectedNodeId(hitNode);
         }
         _isDraggingNode = true;
         _dragCheckpointTaken = false;
         _isMarquee = false;
      }
      else
      {
         _lastClickNodeId = 0;
         if (!IsShiftHeld())
         {
            ClearSelection();
         }
         _isMarquee = true;
         _marqueeStartWorld = world;
         _marqueeEndWorld = world;
         _isDraggingNode = false;
      }
      return true;
   }

   bool CanvasView::HandleMouseRelease(sf::Mouse::Button button, sf::Vector2f screenPoint)
   {
      if (button == sf::Mouse::Button::Left)
      {
         if (_wireStart.has_value())
         {
            const sf::Vector2f world = ScreenToWorld(screenPoint);
            PortHit portHit;
            if (HitTestPort(world, &portHit) &&
                (portHit.direction == PortDirection::In))
            {
               TryConnectWire(*_wireStart, portHit);
            }
            _wireStart.reset();
            ClearWireHoverFeedback();
         }
         if (_isMarquee && (_pDocument != nullptr))
         {
            const sf::FloatRect rect =
               NormalizeRect(_marqueeStartWorld, _marqueeEndWorld);
            const float dragSize =
               std::fabs(_marqueeEndWorld.x - _marqueeStartWorld.x) +
               std::fabs(_marqueeEndWorld.y - _marqueeStartWorld.y);
            if (dragSize >= MarqueeThreshold)
            {
               if (!IsShiftHeld())
               {
                  ClearSelection();
               }
               for (const Node& node : _pDocument->GetNodes())
               {
                  if (IsNodeHiddenByCollapse(node.id))
                  {
                     continue;
                  }
                  const sf::FloatRect bounds = NodeBounds(node);
                  const sf::Vector2f center(
                     bounds.position.x + (bounds.size.x * 0.5f),
                     bounds.position.y + (bounds.size.y * 0.5f));
                  if (rect.contains(center))
                  {
                     AddToSelection(node.id);
                  }
               }
            }
         }
         _isDraggingNode = false;
         _isPanning = false;
         _isMarquee = false;
         _isMinimapDragging = false;
         _dragCheckpointTaken = false;
         return true;
      }
      if (button == sf::Mouse::Button::Middle)
      {
         _isPanning = false;
         return true;
      }
      return false;
   }

   void CanvasView::HandleMouseMove(sf::Vector2f screenPoint)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const sf::Vector2f delta = screenPoint - _lastScreenPoint;
      _lastScreenPoint = screenPoint;
      const sf::Vector2f world = ScreenToWorld(screenPoint);
      _wirePreviewWorld = world;

      if (_isMinimapDragging)
      {
         HandleMinimapPress(screenPoint);
         return;
      }

      PortHit hoverHit;
      if (HitTestPort(world, &hoverHit))
      {
         _hasHoveredPort = true;
         _hoveredPortNodeId = hoverHit.nodeId;
         _hoveredPortName = hoverHit.portName;
         _hoveredPortScreen = WorldToScreen(hoverHit.worldPosition);
      }
      else
      {
         _hasHoveredPort = false;
         _hoveredPortNodeId = 0;
         _hoveredPortName.clear();
      }

      if (_wireStart.has_value())
      {
         UpdateWireHoverFeedback(world);
         return;
      }

      if (_isPanning)
      {
         PanByScreenDelta(delta.x, delta.y);
         return;
      }

      if (_isMarquee)
      {
         _marqueeEndWorld = world;
         return;
      }

      if (_isDraggingNode && (!_selectedNodeIds.empty()))
      {
         if (!_dragCheckpointTaken)
         {
            PushCheckpoint();
            _dragCheckpointTaken = true;
         }
         const float zoom = _pDocument->GetViewportZoom();
         const float deltaX = delta.x / zoom;
         const float deltaY = delta.y / zoom;
         for (size_t index = 0; index < _selectedNodeIds.size(); ++index)
         {
            Node* pNode = _pDocument->FindNodeMutable(_selectedNodeIds[index]);
            if (pNode == nullptr)
            {
               continue;
            }
            pNode->posX += deltaX;
            pNode->posY += deltaY;
         }
         _pDocument->SetDirty(true);
      }
   }

   void CanvasView::PanByScreenDelta(float screenDeltaX, float screenDeltaY)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const float zoom = _pDocument->GetViewportZoom();
      if (zoom <= 0.0f)
      {
         return;
      }
      _pDocument->SetViewport(_pDocument->GetViewportX() - (screenDeltaX / zoom),
                              _pDocument->GetViewportY() - (screenDeltaY / zoom),
                              zoom);
   }

   void CanvasView::ZoomAtScreenPoint(float delta, sf::Vector2f screenPoint)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const sf::Vector2f before = ScreenToWorld(screenPoint);
      float zoom = _pDocument->GetViewportZoom();
      zoom *= (delta > 0.0f) ? 1.1f : (1.0f / 1.1f);
      if (zoom < MinViewportZoom)
      {
         zoom = MinViewportZoom;
      }
      if (zoom > MaxViewportZoom)
      {
         zoom = MaxViewportZoom;
      }
      _pDocument->SetViewport(_pDocument->GetViewportX(),
                              _pDocument->GetViewportY(),
                              zoom);
      const sf::Vector2f after = ScreenToWorld(screenPoint);
      _pDocument->SetViewport(_pDocument->GetViewportX() + (before.x - after.x),
                              _pDocument->GetViewportY() + (before.y - after.y),
                              zoom);
   }

   void CanvasView::HandleWheel(float delta,
                                sf::Vector2f screenPoint,
                                bool horizontal)
   {
      if ((_pDocument == nullptr) || (!_bounds.contains(screenPoint)))
      {
         return;
      }
      if (IsCtrlHeld())
      {
         ZoomAtScreenPoint(delta, screenPoint);
         return;
      }

      const float panAmount = delta * WheelPanPixels;
      if (horizontal || IsShiftHeld())
      {
         PanByScreenDelta(panAmount, 0.0f);
         return;
      }
      PanByScreenDelta(0.0f, panAmount);
   }

   bool CanvasView::HandlePanKey(sf::Keyboard::Key keyCode)
   {
      if (_pDocument == nullptr)
      {
         return false;
      }
      if (keyCode == sf::Keyboard::Key::Left)
      {
         PanByScreenDelta(KeyPanPixels, 0.0f);
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Right)
      {
         PanByScreenDelta(-KeyPanPixels, 0.0f);
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Up)
      {
         PanByScreenDelta(0.0f, KeyPanPixels);
         return true;
      }
      if (keyCode == sf::Keyboard::Key::Down)
      {
         PanByScreenDelta(0.0f, -KeyPanPixels);
         return true;
      }
      return false;
   }

   void CanvasView::DeleteSelection(void)
   {
      if (_selectedNodeIds.empty() || (_pDocument == nullptr))
      {
         return;
      }
      std::vector<NodeId> toDelete = _selectedNodeIds;
      bool anyDeletable = false;
      for (size_t index = 0; index < toDelete.size(); ++index)
      {
         const Node* pNode = _pDocument->FindNode(toDelete[index]);
         if ((pNode != nullptr) && (pNode->type != BlockType::Start))
         {
            anyDeletable = true;
            break;
         }
      }
      if (!anyDeletable)
      {
         return;
      }
      PushCheckpoint();
      for (size_t index = 0; index < toDelete.size(); ++index)
      {
         const Node* pNode = _pDocument->FindNode(toDelete[index]);
         if ((pNode == nullptr) || (pNode->type == BlockType::Start))
         {
            continue;
         }
         _pDocument->RemoveNode(toDelete[index]);
      }
      ClearSelection();
   }

   void CanvasView::DeleteEdge(EdgeId edgeId)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const Edge* pEdge = nullptr;
      const std::vector<Edge>& edges = _pDocument->GetEdges();
      for (size_t index = 0; index < edges.size(); ++index)
      {
         if (edges[index].id == edgeId)
         {
            pEdge = &edges[index];
            break;
         }
      }
      NodeId printfNodeId = 0;
      if (pEdge != nullptr)
      {
         printfNodeId = pEdge->toNodeId;
      }
      PushCheckpoint();
      _pDocument->RemoveEdge(edgeId);
      if (printfNodeId != 0)
      {
         SyncPrintfArgVisibility(_pDocument->FindNodeMutable(printfNodeId),
                                 _pDocument);
      }
   }

   void CanvasView::DeleteNode(NodeId nodeId)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      const Node* pNode = _pDocument->FindNode(nodeId);
      if (pNode == nullptr)
      {
         return;
      }
      if (pNode->type == BlockType::Start)
      {
         return;
      }
      PushCheckpoint();
      if (IsOk(_pDocument->RemoveNode(nodeId)))
      {
         RemoveFromSelection(nodeId);
      }
   }

   void CanvasView::CollectFunctionBodyBounds(NodeId functionId,
                                             float* pOutMinX,
                                             float* pOutMinY,
                                             float* pOutMaxX,
                                             float* pOutMaxY) const
   {
      if ((pOutMinX == nullptr) || (pOutMinY == nullptr) || (pOutMaxX == nullptr) ||
          (pOutMaxY == nullptr) || (_pDocument == nullptr))
      {
         return;
      }
      const Node* pFunction = _pDocument->FindNode(functionId);
      if (pFunction == nullptr)
      {
         return;
      }

      *pOutMinX = pFunction->posX;
      *pOutMinY = pFunction->posY;
      *pOutMaxX = pFunction->posX + BlockNodeWidth;
      *pOutMaxY = pFunction->posY + ComputeBlockNodeHeight(*pFunction);

      std::queue<NodeId> pending;
      std::unordered_set<NodeId> visited;
      const Edge* pBody = _pDocument->FindOutgoingEdge(functionId, "Body");
      if (pBody != nullptr)
      {
         pending.push(pBody->toNodeId);
      }

      while (!pending.empty())
      {
         const NodeId currentId = pending.front();
         pending.pop();
         if (visited.find(currentId) != visited.end())
         {
            continue;
         }
         visited.insert(currentId);
         const Node* pNode = _pDocument->FindNode(currentId);
         if (pNode == nullptr)
         {
            continue;
         }
         const float height = ComputeBlockNodeHeight(*pNode);
         if (pNode->posX < *pOutMinX)
         {
            *pOutMinX = pNode->posX;
         }
         if (pNode->posY < *pOutMinY)
         {
            *pOutMinY = pNode->posY;
         }
         if ((pNode->posX + BlockNodeWidth) > *pOutMaxX)
         {
            *pOutMaxX = pNode->posX + BlockNodeWidth;
         }
         if ((pNode->posY + height) > *pOutMaxY)
         {
            *pOutMaxY = pNode->posY + height;
         }
         for (size_t portIndex = 0; portIndex < pNode->ports.size(); ++portIndex)
         {
            const Port& port = pNode->ports[portIndex];
            if ((port.kind != PortKind::Control) ||
                (port.direction != PortDirection::Out))
            {
               continue;
            }
            const Edge* pEdge =
               _pDocument->FindOutgoingEdge(currentId, port.name);
            if ((pEdge != nullptr) &&
                (visited.find(pEdge->toNodeId) == visited.end()))
            {
               pending.push(pEdge->toNodeId);
            }
         }
      }
   }

   void CanvasView::CollectFunctionBodyNodeIds(NodeId functionId,
                                              std::unordered_set<NodeId>* pOutIds) const
   {
      if ((pOutIds == nullptr) || (_pDocument == nullptr))
      {
         return;
      }

      std::queue<NodeId> pending;
      const Edge* pBody = _pDocument->FindOutgoingEdge(functionId, "Body");
      if (pBody != nullptr)
      {
         pending.push(pBody->toNodeId);
      }

      while (!pending.empty())
      {
         const NodeId currentId = pending.front();
         pending.pop();
         if (pOutIds->find(currentId) != pOutIds->end())
         {
            continue;
         }
         pOutIds->insert(currentId);
         const Node* pNode = _pDocument->FindNode(currentId);
         if (pNode == nullptr)
         {
            continue;
         }
         for (size_t portIndex = 0; portIndex < pNode->ports.size(); ++portIndex)
         {
            const Port& port = pNode->ports[portIndex];
            if ((port.kind != PortKind::Control) ||
                (port.direction != PortDirection::Out))
            {
               continue;
            }
            const Edge* pEdge = _pDocument->FindOutgoingEdge(currentId, port.name);
            if ((pEdge != nullptr) &&
                (pOutIds->find(pEdge->toNodeId) == pOutIds->end()))
            {
               pending.push(pEdge->toNodeId);
            }
         }
      }
   }

   bool CanvasView::IsFunctionCollapsed(const Node& node) const
   {
      if (node.type != BlockType::FunctionDef)
      {
         return false;
      }
      const auto iterator = node.properties.find("collapsed");
      if (iterator == node.properties.end())
      {
         return false;
      }
      return iterator->second == "1";
   }

   bool CanvasView::IsNodeHiddenByCollapse(NodeId nodeId) const
   {
      if (_pDocument == nullptr)
      {
         return false;
      }
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         if (!IsFunctionCollapsed(nodes[index]))
         {
            continue;
         }
         std::unordered_set<NodeId> bodyIds;
         CollectFunctionBodyNodeIds(nodes[index].id, &bodyIds);
         if (bodyIds.find(nodeId) != bodyIds.end())
         {
            return true;
         }
      }
      return false;
   }

   void CanvasView::ToggleFunctionCollapsed(NodeId functionId)
   {
      if (_pDocument == nullptr)
      {
         return;
      }
      Node* pFunction = _pDocument->FindNodeMutable(functionId);
      if ((pFunction == nullptr) || (pFunction->type != BlockType::FunctionDef))
      {
         return;
      }

      PushCheckpoint();
      const bool willCollapse = !IsFunctionCollapsed(*pFunction);
      pFunction->properties["collapsed"] = willCollapse ? "1" : "0";
      if (willCollapse)
      {
         std::unordered_set<NodeId> bodyIds;
         CollectFunctionBodyNodeIds(functionId, &bodyIds);
         for (std::unordered_set<NodeId>::const_iterator iterator = bodyIds.begin();
              iterator != bodyIds.end();
              ++iterator)
         {
            RemoveFromSelection(*iterator);
         }
      }
      _pDocument->SetDirty(true);
   }

   bool CanvasView::TryConnectWire(const PortHit& fromHit, const PortHit& toHit)
   {
      if (_pDocument == nullptr)
      {
         return false;
      }
      if (toHit.direction != PortDirection::In)
      {
         return false;
      }
      if (fromHit.kind != toHit.kind)
      {
         return false;
      }
      if ((fromHit.kind == PortKind::Data) &&
          (!AreTypesCompatible(fromHit.dataType, toHit.dataType)))
      {
         return false;
      }

      PushCheckpoint();
      const Result connectResult =
         _pDocument->Connect(fromHit.nodeId,
                             fromHit.portName,
                             toHit.nodeId,
                             toHit.portName,
                             nullptr);
      if (IsErr(connectResult))
      {
         return false;
      }

      Node* pToNode = _pDocument->FindNodeMutable(toHit.nodeId);
      SyncPrintfArgVisibility(pToNode, _pDocument);
      return true;
   }

   void CanvasView::UpdateWireHoverFeedback(sf::Vector2f worldPoint)
   {
      ClearWireHoverFeedback();
      if (!_wireStart.has_value())
      {
         return;
      }

      PortHit hoverPort;
      if ((!HitTestPort(worldPoint, &hoverPort)) ||
          (hoverPort.direction != PortDirection::In))
      {
         return;
      }

      const std::string fromType = CTypeToString(_wireStart->dataType);
      const std::string toType = CTypeToString(hoverPort.dataType);
      if (_wireStart->kind != hoverPort.kind)
      {
         _wireHoverStatus = WireHoverStatus::Incompatible;
         _wireHoverHint = "Port kind mismatch";
         return;
      }
      if ((_wireStart->kind == PortKind::Data) &&
          (!AreTypesCompatible(_wireStart->dataType, hoverPort.dataType)))
      {
         _wireHoverStatus = WireHoverStatus::Incompatible;
         _wireHoverHint = fromType + " → " + toType + " (incompatible)";
         return;
      }

      _wireHoverStatus = WireHoverStatus::Compatible;
      if (_wireStart->kind == PortKind::Data)
      {
         _wireHoverHint = fromType + " → " + toType;
      }
      else
      {
         _wireHoverHint = "Control flow OK";
      }
   }

   void CanvasView::ClearWireHoverFeedback(void)
   {
      _wireHoverStatus = WireHoverStatus::None;
      _wireHoverHint.clear();
   }

   void CanvasView::DrawFunctionRegions(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pDocument == nullptr))
      {
         return;
      }
      constexpr float RegionPad = 18.0f;
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         if (nodes[index].type != BlockType::FunctionDef)
         {
            continue;
         }
         float minX = 0.0f;
         float minY = 0.0f;
         float maxX = 0.0f;
         float maxY = 0.0f;
         if (IsFunctionCollapsed(nodes[index]))
         {
            minX = nodes[index].posX;
            minY = nodes[index].posY;
            maxX = nodes[index].posX + BlockNodeWidth;
            maxY = nodes[index].posY + ComputeBlockNodeHeight(nodes[index]);
         }
         else
         {
            CollectFunctionBodyBounds(nodes[index].id, &minX, &minY, &maxX, &maxY);
         }
         const sf::Vector2f topLeft =
            WorldToScreen(sf::Vector2f(minX - RegionPad, minY - RegionPad));
         const sf::Vector2f bottomRight =
            WorldToScreen(sf::Vector2f(maxX + RegionPad, maxY + RegionPad));
         sf::RectangleShape region;
         region.setPosition(topLeft);
         region.setSize(sf::Vector2f(bottomRight.x - topLeft.x,
                                     bottomRight.y - topLeft.y));
         region.setFillColor(sf::Color(36, 32, 58, 90));
         region.setOutlineColor(sf::Color(110, 90, 170, 160));
         region.setOutlineThickness(1.0f);
         pTarget->draw(region);
      }
   }

   void CanvasView::DrawStickyFunctionHeaders(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr) || (_pDocument == nullptr))
      {
         return;
      }
      const float zoom = _pDocument->GetViewportZoom();
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         if (node.type != BlockType::FunctionDef)
         {
            continue;
         }
         const sf::Vector2f topLeft = WorldToScreen(sf::Vector2f(node.posX, node.posY));
         if (topLeft.y >= _bounds.position.y)
         {
            continue;
         }
         const auto nameIterator = node.properties.find("name");
         const auto returnIterator = node.properties.find("returnType");
         const auto paramsIterator = node.properties.find("params");
         std::string header = "fn ";
         if (returnIterator != node.properties.end())
         {
            header.append(returnIterator->second);
            header.append(" ");
         }
         if (nameIterator != node.properties.end())
         {
            header.append(nameIterator->second);
         }
         else
         {
            header.append("helper");
         }
         header.append("(");
         if (paramsIterator != node.properties.end())
         {
            header.append(paramsIterator->second);
         }
         header.append(")");

         sf::RectangleShape bar;
         bar.setPosition(sf::Vector2f(_bounds.position.x, _bounds.position.y));
         bar.setSize(sf::Vector2f(_bounds.size.x, 22.0f * std::max(zoom, 0.7f)));
         bar.setFillColor(sf::Color(48, 40, 78, 220));
         pTarget->draw(bar);

         sf::Text label(*_pFont, header, 12);
         label.setFillColor(sf::Color(220, 210, 255));
         label.setPosition(sf::Vector2f(_bounds.position.x + 8.0f,
                                        _bounds.position.y + 3.0f));
         pTarget->draw(label);
      }
   }

   void CanvasView::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr) || (_pDocument == nullptr))
      {
         return;
      }

      const sf::View previousView = pTarget->getView();
      const auto targetSize = pTarget->getSize();
      if ((targetSize.x == 0u) || (targetSize.y == 0u) ||
          (_bounds.size.x <= 0.0f) || (_bounds.size.y <= 0.0f))
      {
         return;
      }

      sf::View clipView;
      clipView.setSize(_bounds.size);
      clipView.setCenter(sf::Vector2f(
         _bounds.position.x + (_bounds.size.x * 0.5f),
         _bounds.position.y + (_bounds.size.y * 0.5f)));
      clipView.setViewport(sf::FloatRect(
         sf::Vector2f(_bounds.position.x / static_cast<float>(targetSize.x),
                      _bounds.position.y / static_cast<float>(targetSize.y)),
         sf::Vector2f(_bounds.size.x / static_cast<float>(targetSize.x),
                      _bounds.size.y / static_cast<float>(targetSize.y))));
      pTarget->setView(clipView);

      sf::RectangleShape background;
      background.setPosition(_bounds.position);
      background.setSize(_bounds.size);
      background.setFillColor(sf::Color(24, 26, 30));
      pTarget->draw(background);

      DrawFunctionRegions(pTarget);

      std::unordered_set<NodeId> hiddenIds;
      const std::vector<Node>& allNodes = _pDocument->GetNodes();
      for (size_t index = 0; index < allNodes.size(); ++index)
      {
         if (IsFunctionCollapsed(allNodes[index]))
         {
            CollectFunctionBodyNodeIds(allNodes[index].id, &hiddenIds);
         }
      }

      const std::vector<Edge>& edges = _pDocument->GetEdges();
      for (size_t index = 0; index < edges.size(); ++index)
      {
         const Edge& edge = edges[index];
         if ((hiddenIds.find(edge.fromNodeId) != hiddenIds.end()) ||
             (hiddenIds.find(edge.toNodeId) != hiddenIds.end()))
         {
            continue;
         }
         const Node* pFrom = _pDocument->FindNode(edge.fromNodeId);
         const Node* pTo = _pDocument->FindNode(edge.toNodeId);
         if ((pFrom == nullptr) || (pTo == nullptr))
         {
            continue;
         }
         sf::Vector2f fromWorld {};
         sf::Vector2f toWorld {};
         bool foundFrom = false;
         bool foundTo = false;
         for (size_t portIndex = 0; portIndex < pFrom->ports.size(); ++portIndex)
         {
            if (pFrom->ports[portIndex].name == edge.fromPort)
            {
               fromWorld = PortWorldPosition(*pFrom, portIndex);
               foundFrom = true;
               break;
            }
         }
         for (size_t portIndex = 0; portIndex < pTo->ports.size(); ++portIndex)
         {
            if (pTo->ports[portIndex].name == edge.toPort)
            {
               toWorld = PortWorldPosition(*pTo, portIndex);
               foundTo = true;
               break;
            }
         }
         if ((!foundFrom) || (!foundTo))
         {
            continue;
         }
         const sf::Vector2f fromScreen = WorldToScreen(fromWorld);
         const sf::Vector2f toScreen = WorldToScreen(toWorld);
         sf::VertexArray line(sf::PrimitiveType::Lines, 2);
         const Port* pPort = FindPort(*pFrom, edge.fromPort);
         const sf::Color color =
            ((pPort != nullptr) && (pPort->kind == PortKind::Data))
               ? sf::Color(80, 180, 255)
               : sf::Color(240, 180, 70);
         line[0].position = fromScreen;
         line[0].color = color;
         line[1].position = toScreen;
         line[1].color = color;
         pTarget->draw(line);
      }

      if (_wireStart.has_value())
      {
         const sf::Vector2f fromScreen = WorldToScreen(_wireStart->worldPosition);
         const sf::Vector2f toScreen = WorldToScreen(_wirePreviewWorld);
         sf::Color previewColor(200, 200, 200);
         if (_wireHoverStatus == WireHoverStatus::Compatible)
         {
            previewColor = sf::Color(90, 220, 130);
         }
         else if (_wireHoverStatus == WireHoverStatus::Incompatible)
         {
            previewColor = sf::Color(230, 90, 90);
         }
         sf::VertexArray preview(sf::PrimitiveType::Lines, 2);
         preview[0].position = fromScreen;
         preview[0].color = previewColor;
         preview[1].position = toScreen;
         preview[1].color = previewColor;
         pTarget->draw(preview);
      }

      const std::vector<Node>& nodes = _pDocument->GetNodes();
      const float zoom = _pDocument->GetViewportZoom();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         if (hiddenIds.find(node.id) != hiddenIds.end())
         {
            continue;
         }
         const sf::Vector2f topLeft = WorldToScreen(sf::Vector2f(node.posX, node.posY));
         const float nodeHeight = ComputeBlockNodeHeight(node);
         sf::RectangleShape shape;
         shape.setPosition(topLeft);
         shape.setSize(sf::Vector2f(BlockNodeWidth * zoom, nodeHeight * zoom));
         if (IsNodeSelected(node.id))
         {
            shape.setFillColor(sf::Color(70, 90, 130));
            shape.setOutlineColor(sf::Color(255, 220, 100));
            shape.setOutlineThickness(2.0f);
         }
         else if (node.type == BlockType::FunctionDef)
         {
            shape.setFillColor(sf::Color(52, 42, 88));
            shape.setOutlineColor(sf::Color(150, 120, 220));
            shape.setOutlineThickness(2.0f);
         }
         else if (IsExpressionBlock(node.type))
         {
            shape.setFillColor(sf::Color(45, 70, 55));
            shape.setOutlineColor(sf::Color(90, 140, 110));
            shape.setOutlineThickness(1.0f);
         }
         else
         {
            shape.setFillColor(sf::Color(55, 60, 75));
            shape.setOutlineColor(sf::Color(120, 130, 150));
            shape.setOutlineThickness(1.0f);
         }
         pTarget->draw(shape);

         sf::Text label(*_pFont, std::string(BlockTypeLabel(node.type)), 14);
         label.setFillColor(sf::Color::White);
         label.setPosition(sf::Vector2f(topLeft.x + (8.0f * zoom), topLeft.y + (6.0f * zoom)));
         pTarget->draw(label);
         if (node.type == BlockType::FunctionDef)
         {
            const auto nameIterator = node.properties.find("name");
            if (nameIterator != node.properties.end())
            {
               sf::Text nameLabel(*_pFont, nameIterator->second, 11);
               nameLabel.setFillColor(sf::Color(200, 190, 255));
               nameLabel.setPosition(sf::Vector2f(topLeft.x + (8.0f * zoom),
                                                   topLeft.y + (22.0f * zoom)));
               pTarget->draw(nameLabel);
            }
            if (IsFunctionCollapsed(node))
            {
               sf::Text collapsedLabel(*_pFont, "[collapsed]", 10);
               collapsedLabel.setFillColor(sf::Color(255, 200, 120));
               collapsedLabel.setPosition(
                  sf::Vector2f(topLeft.x + (8.0f * zoom),
                               topLeft.y + (36.0f * zoom)));
               pTarget->draw(collapsedLabel);
            }
         }

         for (size_t portIndex = 0; portIndex < node.ports.size(); ++portIndex)
         {
            const Port& port = node.ports[portIndex];
            if (!port.visible)
            {
               continue;
            }
            const sf::Vector2f portScreen =
               WorldToScreen(PortWorldPosition(node, portIndex));
            sf::CircleShape circle(PortRadius * zoom);
            circle.setOrigin(sf::Vector2f(PortRadius * zoom, PortRadius * zoom));
            circle.setPosition(portScreen);
            if (port.kind == PortKind::Data)
            {
               circle.setFillColor(sf::Color(80, 180, 255));
            }
            else
            {
               circle.setFillColor(sf::Color(240, 180, 70));
            }
            if (_wireStart.has_value() && (port.direction == PortDirection::In))
            {
               const bool kindsMatch = (_wireStart->kind == port.kind);
               const bool typesOk =
                  (port.kind != PortKind::Data) ||
                  AreTypesCompatible(_wireStart->dataType, port.dataType);
               const bool isHoveredTarget =
                  _hasHoveredPort && (_hoveredPortNodeId == node.id) &&
                  (_hoveredPortName == port.name);
               if ((!kindsMatch) || (!typesOk))
               {
                  circle.setOutlineColor(sf::Color(200, 70, 70, isHoveredTarget ? 255 : 140));
                  circle.setOutlineThickness((isHoveredTarget ? 2.5f : 1.5f) * zoom);
               }
               else if (isHoveredTarget)
               {
                  circle.setOutlineColor(sf::Color(90, 220, 130));
                  circle.setOutlineThickness(2.5f * zoom);
               }
               else
               {
                  circle.setOutlineColor(sf::Color(70, 160, 100, 110));
                  circle.setOutlineThickness(1.2f * zoom);
               }
            }
            pTarget->draw(circle);

            if (zoom >= 0.7f)
            {
               sf::Text portLabel(*_pFont, port.name, 9);
               portLabel.setFillColor(sf::Color(190, 195, 210));
               if (port.direction == PortDirection::In)
               {
                  portLabel.setPosition(sf::Vector2f(portScreen.x + (8.0f * zoom),
                                                     portScreen.y - (6.0f * zoom)));
               }
               else
               {
                  portLabel.setPosition(sf::Vector2f(portScreen.x - (42.0f * zoom),
                                                     portScreen.y - (6.0f * zoom)));
               }
               pTarget->draw(portLabel);
            }
         }
      }

      if (_hasHoveredPort)
      {
         std::string tipText = _hoveredPortName;
         if (!_wireHoverHint.empty())
         {
            tipText.append(" — ");
            tipText.append(_wireHoverHint);
         }
         else if (_wireStart.has_value() &&
                  (_wireStart->kind == PortKind::Data))
         {
            tipText.append("  ");
            tipText.append(CTypeToString(_wireStart->dataType));
         }
         sf::Text tip(*_pFont, tipText, 12);
         tip.setFillColor(sf::Color::White);
         if (_wireHoverStatus == WireHoverStatus::Compatible)
         {
            tip.setFillColor(sf::Color(180, 255, 200));
         }
         else if (_wireHoverStatus == WireHoverStatus::Incompatible)
         {
            tip.setFillColor(sf::Color(255, 180, 180));
         }
         tip.setPosition(sf::Vector2f(_hoveredPortScreen.x + 12.0f,
                                      _hoveredPortScreen.y - 18.0f));
         const sf::FloatRect tipBounds = tip.getLocalBounds();
         sf::RectangleShape tipBg;
         tipBg.setPosition(tip.getPosition() + sf::Vector2f(-4.0f, -2.0f));
         tipBg.setSize(sf::Vector2f(tipBounds.size.x + 8.0f, tipBounds.size.y + 8.0f));
         tipBg.setFillColor(sf::Color(20, 20, 28, 220));
         tipBg.setOutlineColor(sf::Color(160, 160, 180));
         tipBg.setOutlineThickness(1.0f);
         pTarget->draw(tipBg);
         pTarget->draw(tip);
      }

      if (_isMarquee)
      {
         const sf::FloatRect worldRect =
            NormalizeRect(_marqueeStartWorld, _marqueeEndWorld);
         const sf::Vector2f topLeft = WorldToScreen(worldRect.position);
         const sf::Vector2f bottomRight = WorldToScreen(sf::Vector2f(
            worldRect.position.x + worldRect.size.x,
            worldRect.position.y + worldRect.size.y));
         sf::RectangleShape marquee;
         marquee.setPosition(topLeft);
         marquee.setSize(sf::Vector2f(bottomRight.x - topLeft.x,
                                      bottomRight.y - topLeft.y));
         marquee.setFillColor(sf::Color(80, 140, 220, 40));
         marquee.setOutlineColor(sf::Color(120, 180, 255));
         marquee.setOutlineThickness(1.0f);
         pTarget->draw(marquee);
      }

      DrawStickyFunctionHeaders(pTarget);
      pTarget->setView(previousView);
      DrawMinimap(pTarget);
   }

   sf::FloatRect CanvasView::MinimapScreenRect(void) const
   {
      return sf::FloatRect(
         sf::Vector2f(_bounds.position.x + _bounds.size.x - MinimapMargin - MinimapWidth,
                      _bounds.position.y + _bounds.size.y - MinimapMargin - MinimapHeight),
         sf::Vector2f(MinimapWidth, MinimapHeight));
   }

   bool CanvasView::HandleMinimapPress(sf::Vector2f screenPoint)
   {
      if (_pDocument == nullptr)
      {
         return false;
      }
      const sf::FloatRect minimap = MinimapScreenRect();
      if (!minimap.contains(screenPoint))
      {
         if (!_isMinimapDragging)
         {
            return false;
         }
         screenPoint.x = std::clamp(screenPoint.x,
                                    minimap.position.x,
                                    minimap.position.x + minimap.size.x);
         screenPoint.y = std::clamp(screenPoint.y,
                                    minimap.position.y,
                                    minimap.position.y + minimap.size.y);
      }

      float minX = 0.0f;
      float minY = 0.0f;
      float maxX = 0.0f;
      float maxY = 0.0f;
      bool hasBounds = false;
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         const float nodeHeight = ComputeBlockNodeHeight(node);
         const float nodeMaxX = node.posX + BlockNodeWidth;
         const float nodeMaxY = node.posY + nodeHeight;
         if (!hasBounds)
         {
            minX = node.posX;
            minY = node.posY;
            maxX = nodeMaxX;
            maxY = nodeMaxY;
            hasBounds = true;
         }
         else
         {
            minX = std::min(minX, node.posX);
            minY = std::min(minY, node.posY);
            maxX = std::max(maxX, nodeMaxX);
            maxY = std::max(maxY, nodeMaxY);
         }
      }
      if (!hasBounds)
      {
         minX = _pDocument->GetViewportX();
         minY = _pDocument->GetViewportY();
         maxX = minX + (_bounds.size.x / std::max(0.01f, _pDocument->GetViewportZoom()));
         maxY = minY + (_bounds.size.y / std::max(0.01f, _pDocument->GetViewportZoom()));
      }
      minX -= MinimapContentPad;
      minY -= MinimapContentPad;
      maxX += MinimapContentPad;
      maxY += MinimapContentPad;

      const float worldWidth = std::max(1.0f, maxX - minX);
      const float worldHeight = std::max(1.0f, maxY - minY);
      const float localX = (screenPoint.x - minimap.position.x) / minimap.size.x;
      const float localY = (screenPoint.y - minimap.position.y) / minimap.size.y;
      const float centerWorldX = minX + (localX * worldWidth);
      const float centerWorldY = minY + (localY * worldHeight);
      const float zoom = _pDocument->GetViewportZoom();
      const float viewW = _bounds.size.x / zoom;
      const float viewH = _bounds.size.y / zoom;
      _pDocument->SetViewport(centerWorldX - (viewW * 0.5f),
                              centerWorldY - (viewH * 0.5f),
                              zoom);
      _isMinimapDragging = true;
      _isPanning = false;
      _isMarquee = false;
      _isDraggingNode = false;
      return true;
   }

   void CanvasView::DrawMinimap(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pDocument == nullptr))
      {
         return;
      }

      const sf::FloatRect minimap = MinimapScreenRect();
      sf::RectangleShape background;
      background.setPosition(minimap.position);
      background.setSize(minimap.size);
      background.setFillColor(sf::Color(18, 20, 26, 210));
      background.setOutlineColor(sf::Color(110, 120, 140));
      background.setOutlineThickness(1.0f);
      pTarget->draw(background);

      float minX = 0.0f;
      float minY = 0.0f;
      float maxX = 0.0f;
      float maxY = 0.0f;
      bool hasBounds = false;
      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         const float nodeHeight = ComputeBlockNodeHeight(node);
         const float nodeMaxX = node.posX + BlockNodeWidth;
         const float nodeMaxY = node.posY + nodeHeight;
         if (!hasBounds)
         {
            minX = node.posX;
            minY = node.posY;
            maxX = nodeMaxX;
            maxY = nodeMaxY;
            hasBounds = true;
         }
         else
         {
            minX = std::min(minX, node.posX);
            minY = std::min(minY, node.posY);
            maxX = std::max(maxX, nodeMaxX);
            maxY = std::max(maxY, nodeMaxY);
         }
      }
      if (!hasBounds)
      {
         minX = _pDocument->GetViewportX();
         minY = _pDocument->GetViewportY();
         maxX = minX + (_bounds.size.x / std::max(0.01f, _pDocument->GetViewportZoom()));
         maxY = minY + (_bounds.size.y / std::max(0.01f, _pDocument->GetViewportZoom()));
      }
      minX -= MinimapContentPad;
      minY -= MinimapContentPad;
      maxX += MinimapContentPad;
      maxY += MinimapContentPad;

      const float worldWidth = std::max(1.0f, maxX - minX);
      const float worldHeight = std::max(1.0f, maxY - minY);
      const float scaleX = minimap.size.x / worldWidth;
      const float scaleY = minimap.size.y / worldHeight;

      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         const float nodeHeight = ComputeBlockNodeHeight(node);
         sf::RectangleShape nodeShape;
         nodeShape.setPosition(sf::Vector2f(
            minimap.position.x + ((node.posX - minX) * scaleX),
            minimap.position.y + ((node.posY - minY) * scaleY)));
         nodeShape.setSize(sf::Vector2f(std::max(2.0f, BlockNodeWidth * scaleX),
                                        std::max(2.0f, nodeHeight * scaleY)));
         if (node.type == BlockType::FunctionDef)
         {
            nodeShape.setFillColor(sf::Color(140, 100, 200));
         }
         else if (node.type == BlockType::Start)
         {
            nodeShape.setFillColor(sf::Color(80, 180, 100));
         }
         else if (node.type == BlockType::End)
         {
            nodeShape.setFillColor(sf::Color(200, 90, 90));
         }
         else
         {
            nodeShape.setFillColor(sf::Color(150, 160, 180));
         }
         pTarget->draw(nodeShape);
      }

      const float zoom = std::max(0.01f, _pDocument->GetViewportZoom());
      const float viewX = _pDocument->GetViewportX();
      const float viewY = _pDocument->GetViewportY();
      const float viewW = _bounds.size.x / zoom;
      const float viewH = _bounds.size.y / zoom;
      sf::RectangleShape viewport;
      viewport.setPosition(sf::Vector2f(minimap.position.x + ((viewX - minX) * scaleX),
                                        minimap.position.y + ((viewY - minY) * scaleY)));
      viewport.setSize(sf::Vector2f(viewW * scaleX, viewH * scaleY));
      viewport.setFillColor(sf::Color(90, 160, 255, 45));
      viewport.setOutlineColor(sf::Color(120, 190, 255));
      viewport.setOutlineThickness(1.0f);
      pTarget->draw(viewport);
   }
} // namespace Cgen
