/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _simplviewapplication_h_
#define _simplviewapplication_h_

#include <QtCore/QSet>
#include <QtCore/QSharedPointer>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QUndoStack>

#include "SVWidgetsLib/Dialogs/UpdateCheck.h"

#define dream3dApp (static_cast<SIMPLViewApplication*>(qApp))

class QSplashScreen;
class SIMPLView_UI;
class QPluginLoader;
class ISIMPLibPlugin;
class SIMPLViewToolbox;
class SVPipelineFilterWidget;
class SVPipelineViewWidget;

/**
 * @brief The SIMPLViewApplication class
 */
class SIMPLViewApplication : public QApplication
{
  Q_OBJECT

public:
  SIMPLViewApplication(int& argc, char** argv);
  ~SIMPLViewApplication();

  enum PasteType
  {
    None,
    Cut,
    Copy
  };

  /**
  * @brief fillVersionData
  * @return
  */
  static UpdateCheck::SIMPLVersionData_t FillVersionData();

  bool initialize(int argc, char* argv[]);

  /**
   * @brief readSettings
   */
  void readSettings();

  /**
   * @brief writeSettings
   */
  void writeSettings();

  void createDefaultMenuBar();

  void createMacDockMenu();

  /**
   * @brief createThemeMenu
   */
  QMenu* createThemeMenu(QActionGroup *actionGroup, QWidget* parent = nullptr);

  QList<SIMPLView_UI*> getSIMPLViewInstances();

  void registerSIMPLViewWindow(SIMPLView_UI* window);

  void unregisterSIMPLViewWindow(SIMPLView_UI* window);

  SIMPLView_UI* getActiveInstance();
  void setActiveWindow(SIMPLView_UI* instance);

  /**
   * @brief event
   * @param event
   * @return
   */
  bool event(QEvent* event);

  /**
   * @brief getRecentFilesMenu
   * @return
   */
  QMenu* getRecentFilesMenu();

public slots:
  void listenNewInstanceTriggered();
  void listenOpenPipelineTriggered();
  void listenClearRecentFilesTriggered();
  void listenClearSIMPLViewCacheTriggered();
  void listenShowSIMPLViewHelpTriggered();
  void listenCheckForUpdatesTriggered();
  void listenDisplayPluginInfoDialogTriggered();
  void listenDisplayAboutSIMPLViewDialogTriggered();
  void listenExitApplicationTriggered();

  SIMPLView_UI* getNewSIMPLViewInstance();

  SIMPLView_UI* newInstanceFromFile(const QString& filePath);

  /**
  * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
  * should be connected to the Signal QtSRecentFileList->fileListChanged
  * @param file The newly added file.
  */
  void updateRecentFileList(const QString& file);

protected:
  // This is a set of all SIMPLView instances currently available
  QList<SIMPLView_UI*> m_SIMPLViewInstances;

  // The currently active SIMPLView instance
  SIMPLView_UI* m_ActiveWindow;

  QString m_OpenDialogLastFilePath;

  bool m_ShowSplash;
  QSplashScreen* m_SplashScreen;
  QVector<QPluginLoader*> m_PluginLoaders;

  /**
   * @brief loadPlugins
   * @return
   */
  QVector<ISIMPLibPlugin*> loadPlugins();

  /**
   * @brief checkForUpdatesAtStartup
   */
  void checkForUpdatesAtStartup();

protected slots:
  /**
  * @brief versionCheckReply
  */
  void versionCheckReply(UpdateCheckData*);

  /**
   * @brief dream3dWindowChanged
   * @param instance
   */
  void dream3dWindowChanged(SIMPLView_UI* instance);

private:
  QMenuBar* m_DefaultMenuBar = nullptr;
  QMenu* m_DockMenu = nullptr;

  QSharedPointer<UpdateCheck>                                       m_UpdateCheck;

  QString                                                           m_LastFilePathOpened;

  QMenu* m_MenuFile = nullptr;
  QMenu* m_MenuEdit = nullptr;
  QMenu* m_MenuView = nullptr;
  QMenu* m_MenuBookmarks = nullptr;
  QMenu* m_MenuPipeline = nullptr;
  QMenu* m_MenuHelp = nullptr;
  QMenu* m_MenuAdvanced = nullptr;
  QMenu* m_MenuRecentFiles = nullptr;
  QMenu* m_MenuThemes = nullptr;

  QAction* m_ActionNew = nullptr;
  QAction* m_ActionOpen = nullptr;
  QAction* m_ActionSave = nullptr;
  QAction* m_ActionSaveAs = nullptr;
  QAction* m_ActionLoadTheme = nullptr;
  QAction* m_ActionSaveTheme = nullptr;
  QAction* m_ActionClearRecentFiles = nullptr;
  QAction* m_ActionExit = nullptr;
  QAction* m_ActionShowSIMPLViewHelp = nullptr;
  QAction* m_ActionAboutSIMPLView = nullptr;
  QAction* m_ActionCheckForUpdates = nullptr;
  QAction* m_ActionPluginInformation = nullptr;
  QAction* m_ActionClearCache = nullptr;

  QAction* m_ActionCut = nullptr;
  QAction* m_ActionCopy = nullptr;
  QAction* m_ActionPaste = nullptr;
  QAction* m_ActionClearPipeline = nullptr;
  QAction* m_ActionUndo = nullptr;
  QAction* m_ActionRedo = nullptr;

  QAction* m_ActionAddBookmark = nullptr;
  QAction* m_ActionAddBookmarkFolder = nullptr;
  QAction* m_ActionClearBookmarks = nullptr;

  QAction* m_ActionShowFilterList = nullptr;
  QAction* m_ActionShowFilterLibrary = nullptr;
  QAction* m_ActionShowBookmarks = nullptr;
  QAction* m_ActionShowPipeline = nullptr;
  QAction* m_ActionShowIssues = nullptr;
  QAction* m_ActionShowConsole = nullptr;
  QAction* m_ActionShowDataBrowser = nullptr;

  QActionGroup* m_ThemeActionGroup = nullptr;

  int m_minSplashTime;

  SIMPLViewApplication(const SIMPLViewApplication&) = delete; // Copy Constructor Not Implemented
  void operator=(const SIMPLViewApplication&);                // Move assignment Not Implemented
};

#endif /* _SIMPLViewApplication_H */
