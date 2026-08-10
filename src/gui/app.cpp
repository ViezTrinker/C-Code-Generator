/*!
 *\file app.cpp
 *\brief Main application implementation.
 */
#include "gui/app.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

#include "codegen/c_codegen.h"
#include "gui/ui_theme.h"
#include "model/c_type.h"
#include "model/graph_validator.h"
#include "model/node.h"
#include "serialize/autosave.h"
#include "serialize/cgen_serializer.h"

namespace Cgen
{
   namespace
   {
      std::filesystem::path ThemePreferencePath(void)
      {
#ifdef _WIN32
         char appData[MAX_PATH] = {};
         const DWORD length =
            GetEnvironmentVariableA("LOCALAPPDATA", appData, MAX_PATH);
         if ((length > 0) && (length < MAX_PATH))
         {
            return std::filesystem::path(appData) / "GraphicalCCodeGenerator" /
                   "ui_theme.txt";
         }
#endif
         return std::filesystem::path("ui_theme.txt");
      }
   } // namespace

   App::App(void)
      : _window(sf::VideoMode({1440u, 900u}), "Graphical C Code Generator")
      , _buildRunner("build_out")
   {
      _window.setFramerateLimit(60);
      LoadThemePreference();
   }

   void App::LoadThemePreference(void)
   {
      const std::filesystem::path path = ThemePreferencePath();
      std::ifstream input(path);
      if (!input.is_open())
      {
         return;
      }
      std::string value;
      input >> value;
      UiThemeId parsed = UiThemeId::Dark;
      if (UiThemeIdFromString(value, &parsed))
      {
         _themeId = parsed;
      }
   }

   void App::SaveThemePreference(void) const
   {
      const std::filesystem::path path = ThemePreferencePath();
      std::error_code errorCode;
      std::filesystem::create_directories(path.parent_path(), errorCode);
      std::ofstream output(path, std::ios::trunc);
      if (!output.is_open())
      {
         return;
      }
      output << UiThemeIdToString(_themeId) << '\n';
   }

   void App::ApplyTheme(void)
   {
      const UiTheme& theme = GetUiTheme(_themeId);
      if (_pToolbar != nullptr)
      {
         _pToolbar->SetTheme(theme);
      }
      if (_pPalette != nullptr)
      {
         _pPalette->SetTheme(theme);
      }
      if (_pCanvas != nullptr)
      {
         _pCanvas->SetTheme(theme);
      }
      if (_pProperties != nullptr)
      {
         _pProperties->SetTheme(theme);
      }
      if (_pProgramLog != nullptr)
      {
         _pProgramLog->SetTheme(theme);
      }
      if (_pCompilerLog != nullptr)
      {
         _pCompilerLog->SetTheme(theme);
      }
      if (_pSourceLog != nullptr)
      {
         _pSourceLog->SetTheme(theme);
      }
      if (_pHelpLog != nullptr)
      {
         _pHelpLog->SetTheme(theme);
      }
      if (_pContextMenu != nullptr)
      {
         _pContextMenu->SetTheme(theme);
      }
   }

   void App::ToggleTheme(void)
   {
      if (_themeId == UiThemeId::Dark)
      {
         _themeId = UiThemeId::Light;
      }
      else
      {
         _themeId = UiThemeId::Dark;
      }
      ApplyTheme();
      Layout();
      SaveThemePreference();
   }

   bool App::LoadFont(void)
   {
      constexpr std::array<const char*, 4> Candidates = {
         "C:/Windows/Fonts/consola.ttf",
         "C:/Windows/Fonts/consolab.ttf",
         "C:/Windows/Fonts/segoeui.ttf",
         "C:/Windows/Fonts/arial.ttf"
      };
      for (size_t index = 0; index < Candidates.size(); ++index)
      {
         if (_font.openFromFile(Candidates[index]))
         {
            return true;
         }
      }
      return false;
   }

   void App::Layout(void)
   {
      const auto windowSize = _window.getSize();
      const float width = static_cast<float>(windowSize.x);
      const float height = static_cast<float>(windowSize.y);
      constexpr float ToolbarHeight = 40.0f;
      constexpr float BottomHeight = 180.0f;
      constexpr float LeftWidth = 170.0f;
      constexpr float RightWidth = 240.0f;
      constexpr float PanelGap = 4.0f;

      _pToolbar->SetBounds(
         sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(width, ToolbarHeight)));
      _pPalette->SetBounds(sf::FloatRect(
         sf::Vector2f(0.0f, ToolbarHeight),
         sf::Vector2f(LeftWidth, height - ToolbarHeight - BottomHeight)));
      const float canvasX = LeftWidth + PanelGap;
      const float canvasWidth = width - LeftWidth - RightWidth - (PanelGap * 2.0f);
      _pCanvas->SetBounds(sf::FloatRect(
         sf::Vector2f(canvasX, ToolbarHeight),
         sf::Vector2f(canvasWidth, height - ToolbarHeight - BottomHeight)));
      _pProperties->SetBounds(sf::FloatRect(
         sf::Vector2f(width - RightWidth, ToolbarHeight),
         sf::Vector2f(RightWidth, height - ToolbarHeight - BottomHeight)));
      _pProgramLog->SetBounds(sf::FloatRect(sf::Vector2f(0.0f, height - BottomHeight),
                                            sf::Vector2f(width * 0.5f, BottomHeight)));
      _pCompilerLog->SetBounds(sf::FloatRect(sf::Vector2f(width * 0.5f, height - BottomHeight),
                                             sf::Vector2f(width * 0.5f, BottomHeight)));
      const sf::FloatRect overlayBounds(
         sf::Vector2f(0.0f, ToolbarHeight),
         sf::Vector2f(width, height - ToolbarHeight - BottomHeight));
      _pSourceLog->SetBounds(overlayBounds);
      _pHelpLog->SetBounds(overlayBounds);
   }

   void App::UpdateTitle(void)
   {
      std::string title = "Graphical C Code Generator";
      if (!_document.GetFilePath().empty())
      {
         title.append(" - ");
         title.append(_document.GetFilePath());
      }
      if (_document.IsDirty())
      {
         title.append(" *");
      }
      _window.setTitle(title);
   }

   void App::SyncSelectionUi(void)
   {
      std::vector<NodeId> selected = _pCanvas->GetSelectedNodeIds();
      std::vector<NodeId> valid;
      for (size_t index = 0; index < selected.size(); ++index)
      {
         if (_document.FindNode(selected[index]) != nullptr)
         {
            valid.push_back(selected[index]);
         }
      }
      if (valid.size() != selected.size())
      {
         _pCanvas->SetSelectedNodeIds(valid);
      }
      _pProperties->SetSelection(&_document, _pCanvas->GetSelectedNodeId());
      RequestLiveValidation();
      UpdateTitle();
   }

   void App::RequestLiveValidation(void)
   {
      _validationPending = true;
      _validationClock.restart();
      _liveValidationOwnsCompiler = true;
   }

   void App::FlushLiveValidation(void)
   {
      if (_pCanvas == nullptr)
      {
         return;
      }
      const ValidationReport report = ValidateGraph(_document);
      std::unordered_map<NodeId, ValidationSeverity> severityByNodeId;
      BuildNodeSeverityMap(report, &severityByNodeId);
      _pCanvas->SetValidationSeverityMap(severityByNodeId);

      if ((_pCompilerLog != nullptr) && _liveValidationOwnsCompiler)
      {
         _pCompilerLog->SetValidationReport(
            report, "\n(Live validation — Generate C for source output)\n");
      }
   }

   void App::MaybeFlushLiveValidation(void)
   {
      if (_pProperties != nullptr)
      {
         if (_pProperties->ConsumeDidEdit())
         {
            RequestLiveValidation();
         }
      }
      if (!_validationPending)
      {
         return;
      }
      constexpr float ValidationDebounceSeconds = 0.35f;
      if (_validationClock.getElapsedTime().asSeconds() < ValidationDebounceSeconds)
      {
         return;
      }
      _validationPending = false;
      FlushLiveValidation();
   }

   void App::RefreshValidationBadges(void)
   {
      FlushLiveValidation();
   }

   void App::ToggleOrthogonalWires(void)
   {
      if (_document.GetOrthogonalWires() == GraphDocument::OrthogonalWires::Yes)
      {
         _document.SetOrthogonalWires(GraphDocument::OrthogonalWires::No);
      }
      else
      {
         _document.SetOrthogonalWires(GraphDocument::OrthogonalWires::Yes);
      }
      _document.SetDirty(true);
      if (_pProperties != nullptr)
      {
         _pProperties->ReloadFromDocument();
      }
      UpdateTitle();
   }

   void App::ClearAutosaveFiles(void)
   {
      ClearAutosave(CrashAutosavePath());
      const std::filesystem::path sibling =
         SiblingAutosavePath(_document.GetFilePath());
      if (!sibling.empty())
      {
         ClearAutosave(sibling);
      }
   }

   void App::MaybeAutosave(void)
   {
      if (!_document.IsDirty())
      {
         return;
      }
      constexpr float AutosaveIntervalSeconds = 30.0f;
      if (_autosaveClock.getElapsedTime().asSeconds() < AutosaveIntervalSeconds)
      {
         return;
      }
      _autosaveClock.restart();

      const std::string originalPath = _document.GetFilePath();
      const Result crashResult =
         WriteAutosave(_document, CrashAutosavePath(), originalPath);
      if (IsErr(crashResult))
      {
         return;
      }

      const std::filesystem::path sibling = SiblingAutosavePath(originalPath);
      if (!sibling.empty())
      {
         WriteAutosave(_document, sibling, originalPath);
      }
   }

   void App::PromptRestoreAutosaveIfPresent(void)
   {
      const std::filesystem::path crashPath = CrashAutosavePath();
      if (!AutosaveExists(crashPath))
      {
         return;
      }

      bool shouldRestore = false;
#ifdef _WIN32
      const int answer =
         MessageBoxA(nullptr,
                     "A recovered autosave was found from a previous session.\n"
                     "Restore it?",
                     "Crash Recovery",
                     MB_YESNO | MB_ICONQUESTION);
      shouldRestore = (answer == IDYES);
#else
      shouldRestore = true;
#endif
      if (!shouldRestore)
      {
         ClearAutosave(crashPath);
         return;
      }

      std::string diagnostics;
      const Result result = LoadAutosave(crashPath, &_document, &diagnostics);
      if (IsErr(result))
      {
         _pCompilerLog->SetText("Failed to restore autosave\n" + diagnostics);
         ClearAutosave(crashPath);
         return;
      }

      _pCanvas->SetDocument(&_document);
      _pProperties->SetSelection(&_document, 0);
      _history.Clear();
      ClearAutosave(crashPath);
      _autosaveClock.restart();
      _pCompilerLog->SetText(
         "Restored autosave. Save the document to keep your work.\n");
      SyncSelectionUi();
      UpdateTitle();
   }

   void App::JumpToValidationNode(NodeId nodeId)
   {
      if ((nodeId == 0) || (_document.FindNode(nodeId) == nullptr))
      {
         return;
      }
      _pCanvas->SetSelectedNodeId(nodeId);
      _pCanvas->CenterOnNode(nodeId);
      SyncSelectionUi();
   }

   void App::NewDocument(void)
   {
      CloseSourceView();
      CloseHelpView();
      if (_pContextMenu != nullptr)
      {
         _pContextMenu->Close();
      }
      ClearAutosaveFiles();
      _history.Clear();
      _document.Reset();
      _pCanvas->SetDocument(&_document);
      _pCanvas->ClearValidationSeverityMap();
      _pProperties->SetSelection(&_document, 0);
      _pProgramLog->Clear();
      _pCompilerLog->SetText("New document.\n");
      _autosaveClock.restart();
      SyncSelectionUi();
   }

   bool App::PromptOpenPath(std::string* pOutPath)
   {
      if (pOutPath == nullptr)
      {
         return false;
      }
#ifdef _WIN32
      std::array<char, MAX_PATH> fileBuffer {};
      OPENFILENAMEA openFileName {};
      openFileName.lStructSize = sizeof(openFileName);
      openFileName.hwndOwner = nullptr;
      openFileName.lpstrFilter = "CGen Files (*.cgen)\0*.cgen\0All Files (*.*)\0*.*\0";
      openFileName.lpstrFile = fileBuffer.data();
      openFileName.nMaxFile = MAX_PATH;
      openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
      openFileName.lpstrDefExt = "cgen";
      if (GetOpenFileNameA(&openFileName) == TRUE)
      {
         *pOutPath = fileBuffer.data();
         return true;
      }
      return false;
#else
      *pOutPath = "document.cgen";
      return true;
#endif
   }

   bool App::PromptSavePath(std::string* pOutPath)
   {
      if (pOutPath == nullptr)
      {
         return false;
      }
#ifdef _WIN32
      std::array<char, MAX_PATH> fileBuffer {};
      if (!_document.GetFilePath().empty())
      {
         strncpy_s(fileBuffer.data(),
                   fileBuffer.size(),
                   _document.GetFilePath().c_str(),
                   _TRUNCATE);
      }
      OPENFILENAMEA openFileName {};
      openFileName.lStructSize = sizeof(openFileName);
      openFileName.hwndOwner = nullptr;
      openFileName.lpstrFilter = "CGen Files (*.cgen)\0*.cgen\0All Files (*.*)\0*.*\0";
      openFileName.lpstrFile = fileBuffer.data();
      openFileName.nMaxFile = MAX_PATH;
      openFileName.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
      openFileName.lpstrDefExt = "cgen";
      if (GetSaveFileNameA(&openFileName) == TRUE)
      {
         *pOutPath = fileBuffer.data();
         return true;
      }
      return false;
#else
      *pOutPath = "document.cgen";
      return true;
#endif
   }

   void App::OpenDocument(void)
   {
      std::string path;
      if (!PromptOpenPath(&path))
      {
         return;
      }
      std::string diagnostics;
      const Result result = LoadCgenFile(path, &_document, &diagnostics);
      if (IsErr(result))
      {
         _pCompilerLog->SetText("Failed to open .cgen\n" + diagnostics);
         return;
      }
      ClearAutosaveFiles();
      _pCanvas->SetDocument(&_document);
      _pProperties->SetSelection(&_document, 0);
      CloseSourceView();
      CloseHelpView();
      if (_pContextMenu != nullptr)
      {
         _pContextMenu->Close();
      }
      _history.Clear();
      _autosaveClock.restart();
      _pCompilerLog->SetText("Loaded " + path + "\n");
      SyncSelectionUi();
      UpdateTitle();
   }

   void App::SaveDocument(void)
   {
      std::string path = _document.GetFilePath();
      if (path.empty())
      {
         if (!PromptSavePath(&path))
         {
            return;
         }
      }
      const Result result = SaveCgenFile(_document, path);
      if (IsErr(result))
      {
         _pCompilerLog->SetText("Failed to save .cgen\n");
         return;
      }
      _document.SetFilePath(path);
      _document.SetDirty(false);
      ClearAutosaveFiles();
      _autosaveClock.restart();
      _pCompilerLog->SetText("Saved " + path + "\n");
      UpdateTitle();
   }

   void App::GenerateCode(void)
   {
      const ValidationReport report = ValidateGraph(_document);
      const CodegenOutput output = GenerateCSource(_document);
      _lastGeneratedSource = output.source;
      _liveValidationOwnsCompiler = false;

      std::string footer;
      if (IsErr(output.result))
      {
         footer = "\nCodegen warnings/errors:\n" + output.diagnostics +
                  "\n--- Generated source still written for inspection ---\n";
      }
      else
      {
         footer = "\nCode generation succeeded.\n";
      }
      _pCompilerLog->SetValidationReport(report, footer);

      std::unordered_map<NodeId, ValidationSeverity> severityByNodeId;
      BuildNodeSeverityMap(report, &severityByNodeId);
      _pCanvas->SetValidationSeverityMap(severityByNodeId);

      _buildRunner.SetArtifactBaseName(_document.GetFilePath());
      const Result writeResult = _buildRunner.WriteSource(output.source);
      if (IsErr(writeResult))
      {
         _pCompilerLog->Append("Failed to write .c file\n");
         return;
      }
      _pCompilerLog->Append("Wrote " + _buildRunner.GetSourcePath() + "\n");
      if (_document.GetClangFormatOnGenerate() ==
          GraphDocument::ClangFormatOnGenerate::Yes)
      {
         const Result formatResult = _buildRunner.TryClangFormatSource();
         if (IsOk(formatResult))
         {
            _pCompilerLog->Append("clang-format applied (or skipped if unavailable).\n");
            std::ifstream formatted(_buildRunner.GetSourcePath(), std::ios::binary);
            if (formatted.is_open())
            {
               _lastGeneratedSource.assign((std::istreambuf_iterator<char>(formatted)),
                                           std::istreambuf_iterator<char>());
            }
         }
         else
         {
            _pCompilerLog->Append("clang-format failed; keeping unformatted source.\n");
         }
      }
      ShowGeneratedSource();
   }

   void App::ShowGeneratedSource(void)
   {
      CloseHelpView();
      if (_pContextMenu != nullptr)
      {
         _pContextMenu->Close();
      }
      std::string display;
      display.append("// ");
      display.append(_buildRunner.GetSourcePath());
      display.append("\n");
      display.append("// Press Esc to close this view.\n\n");
      display.append(_lastGeneratedSource);
      _pSourceLog->SetText(display);
      _sourceViewVisible = true;
   }

   void App::CloseSourceView(void)
   {
      _sourceViewVisible = false;
   }

   void App::ShowHelp(void)
   {
      CloseSourceView();
      if (_pContextMenu != nullptr)
      {
         _pContextMenu->Close();
      }
      _pHelpLog->SetText(
         "Graphical C Code Generator — Help\n"
         "Press Esc or ? again to close.\n\n"
         "Editing\n"
         "- Drag a block from the left Blocks panel onto the canvas to place it.\n"
         "- Type in the Blocks filter to find blocks by name.\n"
         "- Drag blocks to move them. Middle-drag or Space+drag pans; left-drag empty for marquee.\n"
         "- Drag from an output port to rewire; drop on an input replaces an existing wire.\n"
         "- Ortho toggles elbow wire routing (also Document orthogonalWires).\n"
         "- Validation issues appear live in Compiler (click to jump) and as red/yellow block outlines.\n"
         "- Shift+click toggles multi-select. Ctrl+A selects all.\n"
         "- Mouse wheel pans the canvas vertically; Shift+wheel pans horizontally.\n"
         "- Ctrl+wheel zooms. Arrow keys also pan. Hover a port to see its name.\n"
         "- Drag from an output port to an input port to wire blocks.\n"
         "- While dragging a wire: green = compatible types, red = incompatible (drop refused).\n"
         "- Amber ports = control flow, blue ports = data.\n"
         "- Click a block, then edit its properties on the right (Enter commits).\n"
         "- Click choice fields (type, op, access, function) for dropdowns; scroll the list with the mouse wheel; you can still type.\n"
         "- Double-click a FunctionDef to collapse/expand its body on the canvas.\n"
         "- Properties show a short C: preview of the selected block.\n\n"
         "Delete / clipboard\n"
         "- Select block(s) and press Delete or Backspace.\n"
         "- Ctrl+C / Ctrl+V copy and paste a subgraph (Start excluded).\n"
         "- Right-click a block → Delete Block (Start cannot be deleted).\n"
         "- Right-click a Call → Create Matching FunctionDef when no/unknown function exists.\n"
         "- Right-click a wired port → Delete Wire.\n\n"
         "Undo / layout\n"
         "- Ctrl+Z undoes up to 64 graph edits (place, delete, wire, tidy, property commit).\n"
         "- Each Enter-committed property change is one undo step; a node drag is one step.\n"
         "- Ctrl+Y redoes the last undone edit.\n"
         "- Tidy or Ctrl+L auto-layouts control flow left-to-right.\n"
         "- Snap toggles grid snap (on by default) and snaps the selection.\n"
         "- AlignL / AlignT align a multi-selection.\n"
         "- Fit / Ctrl+0 fits all blocks; Fit Sel / Ctrl+Shift+0 fits the selection.\n"
         "- Bottom-right minimap: click or drag to jump around large graphs.\n"
         "- Clear selection to edit Document fileDescription and clangFormat.\n"
         "- FunctionDef uses paramCount + paramNName/paramNType (Param ports); Call Arg ports follow that function.\n\n"
         "File & build\n"
         "- Ctrl+N New, Ctrl+O Open, Ctrl+S Save (.cgen).\n"
         "- Generate C validates the graph (click issues to jump), writes .c, shows source.\n"
         "- Build compiles with gcc. Run executes; type input in Program Output.\n"
         "- Stop terminates a running program.\n"
         "- CLI: --codegen writes only; add --compile or --run as needed.\n\n"
         "Help\n"
         "- Toolbar ? or F1 opens this help.\n");
      _helpViewVisible = true;
   }

   void App::CloseHelpView(void)
   {
      _helpViewVisible = false;
   }

   void App::UndoEdit(void)
   {
      if (!_history.Undo(&_document))
      {
         return;
      }
      SyncSelectionUi();
      _pProperties->ReloadFromDocument();
   }

   void App::RedoEdit(void)
   {
      if (!_history.Redo(&_document))
      {
         return;
      }
      SyncSelectionUi();
      _pProperties->ReloadFromDocument();
   }

   void App::DeleteSelectedBlock(void)
   {
      _pCanvas->DeleteSelection();
      SyncSelectionUi();
   }

   void App::OpenCanvasContextMenu(sf::Vector2f screenPoint)
   {
      CanvasHitInfo hit;
      if (!_pCanvas->QueryHit(screenPoint, &hit))
      {
         return;
      }
      if (hit.kind == CanvasHitKind::Wire)
      {
         _pContextMenu->OpenDeleteWire(screenPoint, hit.edgeId);
         return;
      }
      if (hit.kind == CanvasHitKind::Node)
      {
         const Node* pNode = _document.FindNode(hit.nodeId);
         if ((pNode == nullptr) || (pNode->type == BlockType::Start))
         {
            _pContextMenu->Close();
            return;
         }
         _pCanvas->SetSelectedNodeId(hit.nodeId);
         SyncSelectionUi();
         _pContextMenu->OpenNodeMenu(screenPoint, hit.nodeId, pNode->type);
         return;
      }
      _pContextMenu->Close();
   }

   void App::HandleContextMenuClick(sf::Vector2f screenPoint)
   {
      NodeId nodeId = 0;
      EdgeId edgeId = 0;
      const ContextMenuAction action =
         _pContextMenu->HitTest(screenPoint, &nodeId, &edgeId);
      _pContextMenu->Close();
      if (action == ContextMenuAction::DeleteBlock)
      {
         _pCanvas->DeleteNode(nodeId);
         SyncSelectionUi();
         return;
      }
      if (action == ContextMenuAction::DeleteWire)
      {
         _pCanvas->DeleteEdge(edgeId);
         SyncSelectionUi();
         return;
      }
      if (action == ContextMenuAction::CreateMatchingFunctionDef)
      {
         CreateMatchingFunctionDef(nodeId);
      }
   }

   void App::CreateMatchingFunctionDef(NodeId callNodeId)
   {
      Node* pCall = _document.FindNodeMutable(callNodeId);
      if ((pCall == nullptr) || (pCall->type != BlockType::Call))
      {
         return;
      }

      std::string functionName = "helper";
      const auto functionIterator = pCall->properties.find("function");
      if ((functionIterator != pCall->properties.end()) &&
          (!functionIterator->second.empty()))
      {
         functionName = functionIterator->second;
      }
      else
      {
         pCall->properties["function"] = functionName;
      }

      std::string returnType = "void";
      const auto returnIterator = pCall->properties.find("returnType");
      if ((returnIterator != pCall->properties.end()) &&
          (!returnIterator->second.empty()))
      {
         returnType = returnIterator->second;
      }

      uint32_t arity = 0;
      std::array<std::string, MaxFunctionParams> argTypes {};
      for (size_t index = 0; index < argTypes.size(); ++index)
      {
         argTypes[index] = "int32_t";
      }
      for (size_t portIndex = 0; portIndex < pCall->ports.size(); ++portIndex)
      {
         const Port& port = pCall->ports[portIndex];
         if ((!port.visible) || (port.direction != PortDirection::In))
         {
            continue;
         }
         if (port.name.rfind("Arg", 0) != 0)
         {
            continue;
         }
         if (arity >= argTypes.size())
         {
            break;
         }
         const std::string typeText = CTypeToString(port.dataType);
         if (!typeText.empty())
         {
            argTypes[arity] = typeText;
         }
         ++arity;
      }

      _history.PushCheckpoint(_document);
      const NodeId functionId =
         _document.AddNode(BlockType::FunctionDef,
                           pCall->posX,
                           pCall->posY + 140.0f);
      Node* pFunction = _document.FindNodeMutable(functionId);
      if (pFunction == nullptr)
      {
         return;
      }
      pFunction->properties["name"] = functionName;
      pFunction->properties["returnType"] = returnType;
      pFunction->properties["paramCount"] = std::to_string(arity);
      for (uint32_t paramIndex = 0; paramIndex < arity; ++paramIndex)
      {
         const std::string indexText = std::to_string(paramIndex);
         pFunction->properties["param" + indexText + "Name"] =
            "arg" + indexText;
         pFunction->properties["param" + indexText + "Type"] =
            argTypes[paramIndex];
      }
      SyncFunctionDefParams(pFunction);
      SyncCallArgPorts(pCall, &_document);
      _pCanvas->SetSelectedNodeId(functionId);
      SyncSelectionUi();
   }

   void App::BuildCode(void)
   {
      _buildRunner.SetArtifactBaseName(_document.GetFilePath());
      if (_lastGeneratedSource.empty())
      {
         GenerateCode();
      }
      else
      {
         _buildRunner.WriteSource(_lastGeneratedSource);
      }
      const BuildResult buildResult = _buildRunner.Compile();
      _liveValidationOwnsCompiler = false;
      std::string log;
      log.append("Command: ");
      log.append(buildResult.command);
      log.append("\n");
      if (buildResult.output.empty())
      {
         log.append("(no compiler output)\n");
      }
      else
      {
         log.append(buildResult.output);
      }
      log.append("Exit code: ");
      log.append(std::to_string(buildResult.exitCode));
      log.append("\n");
      _pCompilerLog->SetText(log);
   }

   void App::RunProgram(void)
   {
      BuildCode();
      StopProgram();
      _pProgramLog->Clear();
      _pProgramLog->SetText("Running " + _buildRunner.GetExecutablePath() + "\n"
                            "Type input in the line below, then press Enter.\n"
                            "Use Stop to terminate a running program.\n\n");
      const Result startResult =
         _programSession.Start(_buildRunner.GetExecutablePath());
      if (IsErr(startResult))
      {
         _pProgramLog->Append("Failed to start process.\n");
         _programSessionActive = false;
         return;
      }
      _programSessionActive = true;
      _pProperties->Blur();
      _pProgramLog->FocusInput();
   }

   void App::StopProgram(void)
   {
      if (_programSessionActive || _programSession.IsRunning())
      {
         _programSession.Stop();
         _programSessionActive = false;
         _pProgramLog->Append("\n[Process stopped]\n");
      }
   }

   void App::PollProgramSession(void)
   {
      if (!_programSessionActive)
      {
         return;
      }

      if (_pProgramLog->HasPendingInput())
      {
         const std::string line = _pProgramLog->TakePendingInput();
         std::string payload = line;
         payload.push_back('\n');
         _pProgramLog->Append(std::string("> ") + line + "\n");
         if (IsErr(_programSession.WriteStdin(payload)))
         {
            _pProgramLog->Append("[Failed to write stdin]\n");
         }
      }

      std::string chunk;
      if (_programSession.ConsumeOutput(&chunk))
      {
         _pProgramLog->Append(chunk);
      }

      if (!_programSession.IsRunning())
      {
         std::string trailing;
         if (_programSession.ConsumeOutput(&trailing))
         {
            _pProgramLog->Append(trailing);
         }
         _pProgramLog->Append("\nExit code: " +
                              std::to_string(_programSession.GetExitCode()) + "\n");
         _programSessionActive = false;
      }
   }

   void App::HandleToolbar(ToolbarAction action)
   {
      switch (action)
      {
         case ToolbarAction::NewDocument:
            NewDocument();
            break;
         case ToolbarAction::Open:
            OpenDocument();
            break;
         case ToolbarAction::Save:
            SaveDocument();
            break;
         case ToolbarAction::Generate:
            GenerateCode();
            break;
         case ToolbarAction::Build:
            BuildCode();
            break;
         case ToolbarAction::Run:
            RunProgram();
            break;
         case ToolbarAction::Stop:
            StopProgram();
            break;
         case ToolbarAction::Tidy:
            _pCanvas->TidyLayout();
            SyncSelectionUi();
            break;
         case ToolbarAction::Snap:
            _pCanvas->ToggleSnapToGrid();
            _pCanvas->SnapSelectionToGrid();
            SyncSelectionUi();
            break;
         case ToolbarAction::AlignLeft:
            _pCanvas->AlignSelectedNodes(AlignSelection::Left);
            SyncSelectionUi();
            break;
         case ToolbarAction::AlignTop:
            _pCanvas->AlignSelectedNodes(AlignSelection::Top);
            SyncSelectionUi();
            break;
         case ToolbarAction::OrthogonalWires:
            ToggleOrthogonalWires();
            break;
         case ToolbarAction::FitAll:
            _pCanvas->FitAllNodes();
            break;
         case ToolbarAction::FitSelection:
            _pCanvas->FitSelection();
            break;
         case ToolbarAction::Theme:
            ToggleTheme();
            break;
         case ToolbarAction::Help:
            if (_helpViewVisible)
            {
               CloseHelpView();
            }
            else
            {
               ShowHelp();
            }
            break;
         case ToolbarAction::None:
            break;
      }
   }

   int32_t App::Run(void)
   {
      if (!LoadFont())
      {
         return 1;
      }

      _pToolbar = std::make_unique<Toolbar>(_font);
      _pPalette = std::make_unique<Palette>(_font);
      _pCanvas = std::make_unique<CanvasView>(_font);
      _pProperties = std::make_unique<PropertyPanel>(_font);
      _pProgramLog = std::make_unique<LogPane>("Program Output",
                                               _font,
                                               LogInputMode::Enabled);
      _pCompilerLog =
         std::make_unique<LogPane>("Compiler", _font, LogInputMode::Disabled);
      _pSourceLog = std::make_unique<LogPane>("Generated C (Esc to close)",
                                              _font,
                                              LogInputMode::Disabled);
      _pHelpLog = std::make_unique<LogPane>("Help (Esc to close)",
                                            _font,
                                            LogInputMode::Disabled);
      _pContextMenu = std::make_unique<ContextMenu>(_font);

      _pCanvas->SetDocument(&_document);
      _pCanvas->SetHistory(&_history);
      _pProperties->SetHistory(&_history);
      ApplyTheme();
      Layout();
      UpdateTitle();
      _pCompilerLog->SetText("Ready. Click ? for keyboard shortcuts and editing help.\n"
                             "Drag blocks from the left Blocks panel onto the canvas.\n"
                             "Connect amber ports for control flow, blue for data.\n"
                             "Delete/Backspace removes the selected block; Ctrl+Z / Ctrl+Y undo/redo.\n");
      PromptRestoreAutosaveIfPresent();
      _autosaveClock.restart();
      RequestLiveValidation();

      while (_window.isOpen())
      {
         while (const std::optional<sf::Event> event = _window.pollEvent())
         {
            if (event->is<sf::Event::Closed>())
            {
               StopProgram();
               _window.close();
            }
            else if (const auto* pResized = event->getIf<sf::Event::Resized>())
            {
               const sf::Vector2u size(pResized->size.x, pResized->size.y);
               _window.setView(sf::View(sf::FloatRect(sf::Vector2f(0.0f, 0.0f),
                                                      sf::Vector2f(static_cast<float>(size.x),
                                                                   static_cast<float>(size.y)))));
               Layout();
            }
            else if (const auto* pMousePress = event->getIf<sf::Event::MouseButtonPressed>())
            {
               const sf::Vector2f point(static_cast<float>(pMousePress->position.x),
                                        static_cast<float>(pMousePress->position.y));
               if (pMousePress->button == sf::Mouse::Button::Left)
               {
                  const ToolbarAction action = _pToolbar->HitTest(point);
                  if (action != ToolbarAction::None)
                  {
                     if (_pContextMenu->IsOpen())
                     {
                        _pContextMenu->Close();
                     }
                     HandleToolbar(action);
                     continue;
                  }
               }
               if (_pContextMenu->IsOpen())
               {
                  if (_pContextMenu->Contains(point))
                  {
                     HandleContextMenuClick(point);
                  }
                  else
                  {
                     _pContextMenu->Close();
                  }
               }
               else if (_helpViewVisible && _pHelpLog->Contains(point))
               {
               }
               else if (_sourceViewVisible && _pSourceLog->Contains(point))
               {
               }
               else if (pMousePress->button == sf::Mouse::Button::Right)
               {
                  if (_pCanvas->Contains(point))
                  {
                     OpenCanvasContextMenu(point);
                  }
               }
               else
               {
                  BlockType placeType = BlockType::Literal;
                  const PaletteClickResult paletteClick =
                     _pPalette->HandleClick(point, &placeType);
                  if (paletteClick == PaletteClickResult::BeginDrag)
                  {
                     _pProperties->Blur();
                     _pProgramLog->BlurInput();
                     _pPalette->BlurFilter();
                  }
                  else if (paletteClick == PaletteClickResult::Consumed)
                  {
                     if (_pPalette->IsFilterFocused())
                     {
                        _pProperties->Blur();
                        _pProgramLog->BlurInput();
                     }
                  }
                  else
                  {
                     NodeId jumpNodeId = 0;
                     if (_pCompilerLog->HandleClick(point, &jumpNodeId))
                     {
                        _pProperties->Blur();
                        _pPalette->BlurFilter();
                        if (jumpNodeId != 0)
                        {
                           JumpToValidationNode(jumpNodeId);
                        }
                     }
                     else if (_pProgramLog->HandleClick(point, &jumpNodeId))
                     {
                        _pProperties->Blur();
                        _pPalette->BlurFilter();
                     }
                     else if (_pProperties->HandleClick(point))
                     {
                        _pPalette->BlurFilter();
                        if (_pProperties->HasKeyboardFocus())
                        {
                           _pProgramLog->BlurInput();
                        }
                     }
                     else if (_pCanvas->HandleMousePress(pMousePress->button, point))
                     {
                        _pProperties->Blur();
                        _pPalette->BlurFilter();
                        SyncSelectionUi();
                     }
                  }
               }
            }
            else if (const auto* pMouseRelease = event->getIf<sf::Event::MouseButtonReleased>())
            {
               _pToolbar->HandleMouseRelease();
               const sf::Vector2f point(static_cast<float>(pMouseRelease->position.x),
                                        static_cast<float>(pMouseRelease->position.y));
               if ((pMouseRelease->button == sf::Mouse::Button::Left) &&
                   _pPalette->IsBlockDragActive())
               {
                  BlockType dragType = BlockType::End;
                  if (_pPalette->FinishBlockDrag(&dragType))
                  {
                     if (_pCanvas->Contains(point))
                     {
                        _pCanvas->PlaceBlock(dragType, point);
                        SyncSelectionUi();
                     }
                  }
               }
               else
               {
                  _pPalette->HandleMouseRelease();
               }
               if ((!_sourceViewVisible) && (!_helpViewVisible))
               {
                  _pCanvas->HandleMouseRelease(pMouseRelease->button, point);
                  SyncSelectionUi();
               }
            }
            else if (const auto* pMouseMove = event->getIf<sf::Event::MouseMoved>())
            {
               const sf::Vector2f point(static_cast<float>(pMouseMove->position.x),
                                        static_cast<float>(pMouseMove->position.y));
               _pToolbar->HandleMouseMove(point);
               _pPalette->HandleMouseMove(point);
               if ((!_sourceViewVisible) && (!_helpViewVisible))
               {
                  _pCanvas->HandleMouseMove(point);
                  if (_document.IsDirty())
                  {
                     UpdateTitle();
                  }
               }
            }
            else if (const auto* pWheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
               const sf::Vector2f point(static_cast<float>(pWheel->position.x),
                                        static_cast<float>(pWheel->position.y));
               const bool horizontal =
                  (pWheel->wheel == sf::Mouse::Wheel::Horizontal);
               if (_helpViewVisible && _pHelpLog->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_sourceViewVisible && _pSourceLog->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_pProgramLog->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_pCompilerLog->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_pPalette->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_pProperties->HandleWheel(pWheel->delta, point))
               {
               }
               else if ((!_sourceViewVisible) && (!_helpViewVisible))
               {
                  _pCanvas->HandleWheel(pWheel->delta, point, horizontal);
               }
            }
            else if (const auto* pText = event->getIf<sf::Event::TextEntered>())
            {
               if (_pProgramLog->IsInputFocused())
               {
                  _pProgramLog->HandleTextEntered(pText->unicode);
               }
               else if (_pPalette->IsFilterFocused())
               {
                  _pPalette->HandleTextEntered(pText->unicode);
               }
               else
               {
                  _pProperties->HandleTextEntered(pText->unicode);
               }
            }
            else if (const auto* pKey = event->getIf<sf::Event::KeyPressed>())
            {
               if (pKey->code == sf::Keyboard::Key::Escape)
               {
                  if (_pPalette->IsBlockDragActive())
                  {
                     _pPalette->CancelBlockDrag();
                  }
                  else if (_pContextMenu->IsOpen())
                  {
                     _pContextMenu->Close();
                  }
                  else if (_helpViewVisible)
                  {
                     CloseHelpView();
                  }
                  else if (_sourceViewVisible)
                  {
                     CloseSourceView();
                  }
                  else if (_pPalette->IsFilterFocused() &&
                           _pPalette->HandleKey(pKey->code))
                  {
                  }
               }
               else if (pKey->code == sf::Keyboard::Key::F1)
               {
                  if (_helpViewVisible)
                  {
                     CloseHelpView();
                  }
                  else
                  {
                     ShowHelp();
                  }
               }
               else if (_pProgramLog->IsInputFocused() &&
                        _pProgramLog->HandleKey(pKey->code))
               {
               }
               else if (_pPalette->IsFilterFocused() &&
                        _pPalette->HandleKey(pKey->code))
               {
               }
               else if (_pProperties->HasKeyboardFocus() &&
                        _pProperties->HandleKey(pKey->code))
               {
               }
               else if ((pKey->code == sf::Keyboard::Key::Delete) ||
                        (pKey->code == sf::Keyboard::Key::Backspace))
               {
                  DeleteSelectedBlock();
               }
               else if ((pKey->code == sf::Keyboard::Key::Z) && (pKey->control))
               {
                  UndoEdit();
               }
               else if ((pKey->code == sf::Keyboard::Key::Y) && (pKey->control))
               {
                  RedoEdit();
               }
               else if ((pKey->code == sf::Keyboard::Key::S) && (pKey->control))
               {
                  SaveDocument();
               }
               else if ((pKey->code == sf::Keyboard::Key::O) && (pKey->control))
               {
                  OpenDocument();
               }
               else if ((pKey->code == sf::Keyboard::Key::N) && (pKey->control))
               {
                  NewDocument();
               }
               else if ((pKey->code == sf::Keyboard::Key::C) && (pKey->control))
               {
                  _pCanvas->CopySelection();
               }
               else if ((pKey->code == sf::Keyboard::Key::V) && (pKey->control))
               {
                  _pCanvas->PasteClipboard();
                  SyncSelectionUi();
               }
               else if ((pKey->code == sf::Keyboard::Key::A) && (pKey->control))
               {
                  _pCanvas->SelectAll();
                  SyncSelectionUi();
               }
               else if ((pKey->code == sf::Keyboard::Key::L) && (pKey->control))
               {
                  _pCanvas->TidyLayout();
                  SyncSelectionUi();
               }
               else if ((pKey->code == sf::Keyboard::Key::Num0) && (pKey->control) &&
                        (pKey->shift))
               {
                  _pCanvas->FitSelection();
               }
               else if ((pKey->code == sf::Keyboard::Key::Num0) && (pKey->control))
               {
                  _pCanvas->FitAllNodes();
               }
               else if (_pCanvas->HandlePanKey(pKey->code))
               {
               }
            }
         }

         PollProgramSession();
         MaybeAutosave();
         MaybeFlushLiveValidation();

         _window.clear(GetUiTheme(_themeId).windowClear);
         _pToolbar->Draw(&_window);
         _pCanvas->Draw(&_window);
         _pPalette->Draw(&_window);
         _pProperties->Draw(&_window);
         _pProgramLog->Draw(&_window);
         _pCompilerLog->Draw(&_window);
         if (_sourceViewVisible)
         {
            _pSourceLog->Draw(&_window);
         }
         if (_helpViewVisible)
         {
            _pHelpLog->Draw(&_window);
         }
         _pContextMenu->Draw(&_window);
         _pPalette->DrawDragGhost(&_window);
         _pToolbar->DrawHoverTip(&_window);
         _pPalette->DrawHoverTip(&_window);
         _window.display();
      }
      StopProgram();
      return 0;
   }
} // namespace Cgen
