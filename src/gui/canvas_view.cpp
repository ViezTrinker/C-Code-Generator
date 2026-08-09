/*!
 *\file canvas_view.cpp
 *\brief Flowchart canvas interaction and rendering.
 */
#include "gui/canvas_view.h"

#include <algorithm>
#include <cmath>

#include <SFML/Window/Keyboard.hpp>

#include "gui/block_placement.h"
#include "model/graph_layout.h"

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
         for (size_t portIndex = 0; portIndex < node.ports.size(); ++portIndex)
         {
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

      if (IsSpaceHeld())
      {
         _isPanning = true;
         _isMarquee = false;
         _isDraggingNode = false;
         return true;
      }

      PortHit portHit;
      if (HitTestPort(world, &portHit))
      {
         if (portHit.direction == PortDirection::Out)
         {
            _wireStart = portHit;
            _wirePreviewWorld = world;
         }
         else if (_wireStart.has_value())
         {
            PushCheckpoint();
            _pDocument->Connect(_wireStart->nodeId,
                                _wireStart->portName,
                                portHit.nodeId,
                                portHit.portName,
                                nullptr);
            _wireStart.reset();
         }
         return true;
      }

      const NodeId hitNode = HitTestNode(world);
      if (hitNode != 0)
      {
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
               PushCheckpoint();
               _pDocument->Connect(_wireStart->nodeId,
                                   _wireStart->portName,
                                   portHit.nodeId,
                                   portHit.portName,
                                   nullptr);
            }
            _wireStart.reset();
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
      PushCheckpoint();
      _pDocument->RemoveEdge(edgeId);
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

      const std::vector<Edge>& edges = _pDocument->GetEdges();
      for (size_t index = 0; index < edges.size(); ++index)
      {
         const Edge& edge = edges[index];
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
         sf::VertexArray preview(sf::PrimitiveType::Lines, 2);
         preview[0].position = fromScreen;
         preview[0].color = sf::Color(200, 200, 200);
         preview[1].position = toScreen;
         preview[1].color = sf::Color(200, 200, 200);
         pTarget->draw(preview);
      }

      const std::vector<Node>& nodes = _pDocument->GetNodes();
      for (size_t index = 0; index < nodes.size(); ++index)
      {
         const Node& node = nodes[index];
         const sf::Vector2f topLeft = WorldToScreen(sf::Vector2f(node.posX, node.posY));
         const float zoom = _pDocument->GetViewportZoom();
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

         for (size_t portIndex = 0; portIndex < node.ports.size(); ++portIndex)
         {
            const Port& port = node.ports[portIndex];
            const sf::Vector2f portScreen = WorldToScreen(PortWorldPosition(node, portIndex));
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
            pTarget->draw(circle);
         }
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
         marquee.setSize(sf::Vector2f(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y));
         marquee.setFillColor(sf::Color(80, 140, 220, 40));
         marquee.setOutlineColor(sf::Color(120, 180, 255));
         marquee.setOutlineThickness(1.0f);
         pTarget->draw(marquee);
      }

      pTarget->setView(previousView);
   }
} // namespace Cgen
