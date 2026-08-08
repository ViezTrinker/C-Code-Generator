/*!
 *\file canvas_view.cpp
 *\brief Flowchart canvas interaction and rendering.
 */
#include "gui/canvas_view.h"

#include <cmath>

namespace Cgen
{
   namespace
   {
      constexpr float NodeWidth = 140.0f;
      constexpr float NodeHeight = 56.0f;
      constexpr float PortRadius = 6.0f;
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
      _selectedNodeId = 0;
      _wireStart.reset();
   }

   NodeId CanvasView::GetSelectedNodeId(void) const
   {
      return _selectedNodeId;
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
                           sf::Vector2f(NodeWidth, NodeHeight));
   }

   sf::Vector2f CanvasView::PortWorldPosition(const Node& node, size_t portIndex) const
   {
      const Port& port = node.ports[portIndex];
      float offsetY = 18.0f;
      size_t sameSideIndex = 0;
      for (size_t index = 0; index < portIndex; ++index)
      {
         if (node.ports[index].direction == port.direction)
         {
            ++sameSideIndex;
         }
      }
      offsetY += static_cast<float>(sameSideIndex) * 14.0f;
      if (port.direction == PortDirection::In)
      {
         return sf::Vector2f(node.posX, node.posY + offsetY);
      }
      return sf::Vector2f(node.posX + NodeWidth, node.posY + offsetY);
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
      const sf::Vector2f world = ScreenToWorld(screenPoint);
      const NodeId id = _pDocument->AddNode(blockType, world.x, world.y);
      _selectedNodeId = id;
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
         return true;
      }

      if (button == sf::Mouse::Button::Right)
      {
         PortHit hit;
         if (HitTestPort(world, &hit))
         {
            const std::vector<Edge>& edges = _pDocument->GetEdges();
            for (size_t index = 0; index < edges.size(); ++index)
            {
               const Edge& edge = edges[index];
               const bool matchesFrom =
                  (edge.fromNodeId == hit.nodeId) && (edge.fromPort == hit.portName);
               const bool matchesTo =
                  (edge.toNodeId == hit.nodeId) && (edge.toPort == hit.portName);
               if (matchesFrom || matchesTo)
               {
                  _pDocument->RemoveEdge(edge.id);
                  return true;
               }
            }
         }
         return true;
      }

      if (button != sf::Mouse::Button::Left)
      {
         return false;
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
      _selectedNodeId = hitNode;
      if (hitNode != 0)
      {
         _isDraggingNode = true;
      }
      else
      {
         _isPanning = true;
      }
      return true;
   }

   bool CanvasView::HandleMouseRelease(sf::Mouse::Button button, sf::Vector2f screenPoint)
   {
      (void)screenPoint;
      if (button == sf::Mouse::Button::Left)
      {
         _isDraggingNode = false;
         _isPanning = false;
         if (_wireStart.has_value())
         {
            const sf::Vector2f world = ScreenToWorld(screenPoint);
            PortHit portHit;
            if (HitTestPort(world, &portHit) &&
                (portHit.direction == PortDirection::In))
            {
               _pDocument->Connect(_wireStart->nodeId,
                                   _wireStart->portName,
                                   portHit.nodeId,
                                   portHit.portName,
                                   nullptr);
            }
            _wireStart.reset();
         }
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
         const float zoom = _pDocument->GetViewportZoom();
         _pDocument->SetViewport(_pDocument->GetViewportX() - (delta.x / zoom),
                                 _pDocument->GetViewportY() - (delta.y / zoom),
                                 zoom);
         return;
      }

      if (_isDraggingNode && (_selectedNodeId != 0))
      {
         Node* pNode = _pDocument->FindNodeMutable(_selectedNodeId);
         if (pNode != nullptr)
         {
            const float zoom = _pDocument->GetViewportZoom();
            pNode->posX += delta.x / zoom;
            pNode->posY += delta.y / zoom;
            _pDocument->SetDirty(true);
         }
      }
   }

   void CanvasView::HandleWheel(float delta, sf::Vector2f screenPoint)
   {
      if ((_pDocument == nullptr) || (!_bounds.contains(screenPoint)))
      {
         return;
      }
      const sf::Vector2f before = ScreenToWorld(screenPoint);
      float zoom = _pDocument->GetViewportZoom();
      zoom *= (delta > 0.0f) ? 1.1f : (1.0f / 1.1f);
      if (zoom < 0.35f)
      {
         zoom = 0.35f;
      }
      if (zoom > 2.5f)
      {
         zoom = 2.5f;
      }
      _pDocument->SetViewport(_pDocument->GetViewportX(),
                              _pDocument->GetViewportY(),
                              zoom);
      const sf::Vector2f after = ScreenToWorld(screenPoint);
      _pDocument->SetViewport(_pDocument->GetViewportX() + (before.x - after.x),
                              _pDocument->GetViewportY() + (before.y - after.y),
                              zoom);
   }

   void CanvasView::DeleteSelection(void)
   {
      if ((_pDocument == nullptr) || (_selectedNodeId == 0))
      {
         return;
      }
      if (IsOk(_pDocument->RemoveNode(_selectedNodeId)))
      {
         _selectedNodeId = 0;
      }
   }

   void CanvasView::Draw(sf::RenderTarget* pTarget) const
   {
      if ((pTarget == nullptr) || (_pFont == nullptr) || (_pDocument == nullptr))
      {
         return;
      }

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
         sf::RectangleShape shape;
         shape.setPosition(topLeft);
         shape.setSize(sf::Vector2f(NodeWidth * zoom, NodeHeight * zoom));
         if (node.id == _selectedNodeId)
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
   }
} // namespace Cgen
