/*!
 *\file app.h
 *\brief Main application window wiring GUI panels together.
 */
#ifndef APP_H
#define APP_H

#include <cstdint>
#include <memory>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "build/build_runner.h"
#include "build/process_session.h"
#include "gui/canvas_view.h"
#include "gui/context_menu.h"
#include "gui/document_history.h"
#include "gui/log_pane.h"
#include "gui/palette.h"
#include "gui/property_panel.h"
#include "gui/toolbar.h"
#include "gui/ui_theme_id.h"
#include "model/graph_document.h"

namespace Cgen
{
   /*!
    *\brief Top-level SFML application.
    */
   class App
   {
   public:
      /*!
       *\brief Constructs the application.
       */
      App(void);

      /*!
       *\brief Runs the main loop until the window closes.
       *
       *\return Process exit code.
       */
      int32_t Run(void);

   private:
      void Layout(void);
      void HandleToolbar(ToolbarAction action);
      void ApplyTheme(void);
      void ToggleTheme(void);
      void LoadThemePreference(void);
      void SaveThemePreference(void) const;
      void NewDocument(void);
      void OpenDocument(void);
      void SaveDocument(void);
      void GenerateCode(void);
      void BuildCode(void);
      void RunProgram(void);
      void StopProgram(void);
      void PollProgramSession(void);
      void ShowGeneratedSource(void);
      void CloseSourceView(void);
      void ShowHelp(void);
      void CloseHelpView(void);
      void UndoEdit(void);
      void RedoEdit(void);
      void DeleteSelectedBlock(void);
      void OpenCanvasContextMenu(sf::Vector2f screenPoint);
      void HandleContextMenuClick(sf::Vector2f screenPoint);
      void SyncSelectionUi(void);
      void RequestLiveValidation(void);
      void FlushLiveValidation(void);
      void MaybeFlushLiveValidation(void);
      void RefreshValidationBadges(void);
      void JumpToValidationNode(NodeId nodeId);
      void CreateMatchingFunctionDef(NodeId callNodeId);
      void ToggleOrthogonalWires(void);
      void MaybeAutosave(void);
      void ClearAutosaveFiles(void);
      void PromptRestoreAutosaveIfPresent(void);
      void UpdateTitle(void);
      bool LoadFont(void);
      bool PromptOpenPath(std::string* pOutPath);
      bool PromptSavePath(std::string* pOutPath);

      sf::RenderWindow _window;
      sf::Font _font;
      GraphDocument _document;
      DocumentHistory _history;
      BuildRunner _buildRunner;
      ProcessSession _programSession;
      bool _programSessionActive = false;
      bool _sourceViewVisible = false;
      bool _helpViewVisible = false;
      UiThemeId _themeId = UiThemeId::Dark;
      sf::Clock _autosaveClock;
      sf::Clock _validationClock;
      bool _validationPending = false;
      bool _liveValidationOwnsCompiler = true;
      std::unique_ptr<Toolbar> _pToolbar;
      std::unique_ptr<Palette> _pPalette;
      std::unique_ptr<CanvasView> _pCanvas;
      std::unique_ptr<PropertyPanel> _pProperties;
      std::unique_ptr<LogPane> _pProgramLog;
      std::unique_ptr<LogPane> _pCompilerLog;
      std::unique_ptr<LogPane> _pSourceLog;
      std::unique_ptr<LogPane> _pHelpLog;
      std::unique_ptr<ContextMenu> _pContextMenu;
      std::string _lastGeneratedSource;
   };
} // namespace Cgen

#endif // APP_H
