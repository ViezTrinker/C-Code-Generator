/*!
 *\file app.cpp
 *\brief Main application implementation.
 */
#include "gui/app.h"

#include <array>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#endif

#include "codegen/c_codegen.h"
#include "serialize/cgen_serializer.h"

namespace Cgen
{
   App::App(void)
      : _window(sf::VideoMode({1440u, 900u}), "C Code Generator")
      , _buildRunner("build_out")
   {
      _window.setFramerateLimit(60);
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

      _pToolbar->SetBounds(
         sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(width, ToolbarHeight)));
      _pPalette->SetBounds(sf::FloatRect(sf::Vector2f(0.0f, ToolbarHeight),
                                         sf::Vector2f(LeftWidth, height - ToolbarHeight - BottomHeight)));
      _pCanvas->SetBounds(sf::FloatRect(
         sf::Vector2f(LeftWidth, ToolbarHeight),
         sf::Vector2f(width - LeftWidth - RightWidth, height - ToolbarHeight - BottomHeight)));
      _pProperties->SetBounds(sf::FloatRect(
         sf::Vector2f(width - RightWidth, ToolbarHeight),
         sf::Vector2f(RightWidth, height - ToolbarHeight - BottomHeight)));
      _pProgramLog->SetBounds(sf::FloatRect(sf::Vector2f(0.0f, height - BottomHeight),
                                            sf::Vector2f(width * 0.5f, BottomHeight)));
      _pCompilerLog->SetBounds(sf::FloatRect(sf::Vector2f(width * 0.5f, height - BottomHeight),
                                             sf::Vector2f(width * 0.5f, BottomHeight)));
   }

   void App::UpdateTitle(void)
   {
      std::string title = "C Code Generator";
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

   void App::NewDocument(void)
   {
      _document.Reset();
      _pCanvas->SetDocument(&_document);
      _pProperties->SetSelection(&_document, 0);
      _pProgramLog->Clear();
      _pCompilerLog->SetText("New document.\n");
      UpdateTitle();
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
      _pCanvas->SetDocument(&_document);
      _pProperties->SetSelection(&_document, 0);
      _pCompilerLog->SetText("Loaded " + path + "\n");
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
      _pCompilerLog->SetText("Saved " + path + "\n");
      UpdateTitle();
   }

   void App::GenerateCode(void)
   {
      const CodegenOutput output = GenerateCSource(_document);
      _lastGeneratedSource = output.source;
      if (IsErr(output.result))
      {
         _pCompilerLog->SetText("Codegen warnings/errors:\n" + output.diagnostics +
                                "\n--- Generated source still written for inspection ---\n");
      }
      else
      {
         _pCompilerLog->SetText("Code generation succeeded.\n");
      }
      _buildRunner.SetArtifactBaseName(_document.GetFilePath());
      const Result writeResult = _buildRunner.WriteSource(output.source);
      if (IsErr(writeResult))
      {
         _pCompilerLog->Append("Failed to write .c file\n");
         return;
      }
      _pCompilerLog->Append("Wrote " + _buildRunner.GetSourcePath() + "\n");
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

      _pCanvas->SetDocument(&_document);
      Layout();
      UpdateTitle();
      _pCompilerLog->SetText("Ready. Place blocks from the left palette.\n"
                             "Connect amber ports for control flow, blue for data.\n"
                             "Right-click a port to delete its wire.\n"
                             "Use Run to execute; type program input in Program Output.\n");

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
               const ToolbarAction action = _pToolbar->HitTest(point);
               if (action != ToolbarAction::None)
               {
                  HandleToolbar(action);
               }
               else
               {
                  BlockType placeType = BlockType::Literal;
                  if (_pPalette->HitTest(point, &placeType))
                  {
                     _pCanvas->PlaceBlock(placeType, point);
                     _pProperties->SetSelection(&_document, _pCanvas->GetSelectedNodeId());
                     UpdateTitle();
                  }
                  else if (_pProgramLog->HandleClick(point))
                  {
                  }
                  else if (_pProperties->HandleClick(point))
                  {
                  }
                  else if (_pCanvas->HandleMousePress(pMousePress->button, point))
                  {
                     _pProperties->SetSelection(&_document, _pCanvas->GetSelectedNodeId());
                     UpdateTitle();
                  }
               }
            }
            else if (const auto* pMouseRelease = event->getIf<sf::Event::MouseButtonReleased>())
            {
               const sf::Vector2f point(static_cast<float>(pMouseRelease->position.x),
                                        static_cast<float>(pMouseRelease->position.y));
               _pCanvas->HandleMouseRelease(pMouseRelease->button, point);
            }
            else if (const auto* pMouseMove = event->getIf<sf::Event::MouseMoved>())
            {
               const sf::Vector2f point(static_cast<float>(pMouseMove->position.x),
                                        static_cast<float>(pMouseMove->position.y));
               _pCanvas->HandleMouseMove(point);
               if (_document.IsDirty())
               {
                  UpdateTitle();
               }
            }
            else if (const auto* pWheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
               const sf::Vector2f point(static_cast<float>(pWheel->position.x),
                                        static_cast<float>(pWheel->position.y));
               if (_pProgramLog->HandleWheel(pWheel->delta, point))
               {
               }
               else if (_pCompilerLog->HandleWheel(pWheel->delta, point))
               {
               }
               else
               {
                  _pCanvas->HandleWheel(pWheel->delta, point);
               }
            }
            else if (const auto* pText = event->getIf<sf::Event::TextEntered>())
            {
               if (!_pProgramLog->HandleTextEntered(pText->unicode))
               {
                  _pProperties->HandleTextEntered(pText->unicode);
               }
            }
            else if (const auto* pKey = event->getIf<sf::Event::KeyPressed>())
            {
               if (_pProgramLog->HandleKey(pKey->code))
               {
               }
               else if (_pProperties->HandleKey(pKey->code))
               {
               }
               else if (pKey->code == sf::Keyboard::Key::Delete)
               {
                  _pCanvas->DeleteSelection();
                  _pProperties->SetSelection(&_document, _pCanvas->GetSelectedNodeId());
                  UpdateTitle();
               }
               else if ((pKey->code == sf::Keyboard::Key::S) &&
                        (pKey->control))
               {
                  SaveDocument();
               }
               else if ((pKey->code == sf::Keyboard::Key::O) &&
                        (pKey->control))
               {
                  OpenDocument();
               }
               else if ((pKey->code == sf::Keyboard::Key::N) &&
                        (pKey->control))
               {
                  NewDocument();
               }
            }
         }

         PollProgramSession();

         _window.clear(sf::Color(20, 22, 26));
         _pToolbar->Draw(&_window);
         _pPalette->Draw(&_window);
         _pCanvas->Draw(&_window);
         _pProperties->Draw(&_window);
         _pProgramLog->Draw(&_window);
         _pCompilerLog->Draw(&_window);
         _window.display();
      }
      StopProgram();
      return 0;
   }
} // namespace Cgen
