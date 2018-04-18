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

  QList<SIMPLView_UI*> getSIMPLViewInstances();

  void registerSIMPLViewWindow(SIMPLView_UI* window);

  virtual void unregisterSIMPLViewWindow(SIMPLView_UI* window);

  SIMPLView_UI* getActiveWindow();
  void setActiveWindow(SIMPLView_UI* instance);

  bool isCurrentlyRunning(SIMPLView_UI* instance);

  QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> getClipboard();

public slots:
  void setClipboard(QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> clipboard);

  SIMPLView_UI* getNewSIMPLViewInstance();

  SIMPLView_UI* newInstanceFromFile(const QString& filePath);

protected:
  // This is a set of all SIMPLView instances currently available
  QList<SIMPLView_UI*> m_SIMPLViewInstances;

  // The currently active SIMPLView instance
  SIMPLView_UI* m_ActiveWindow;

  QString m_OpenDialogLastFilePath;

  bool m_ShowSplash;
  QSplashScreen* m_SplashScreen;
  QVector<QPluginLoader*> m_PluginLoaders;

  QVector<ISIMPLibPlugin*> loadPlugins();

protected slots:
  void listenExitApplicationTriggered();

  virtual void dream3dWindowChanged(SIMPLView_UI* instance);

private:
  QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*>      m_Clipboard;

  QMenuBar*                             m_DefaultMenuBar = new QMenuBar();
  QSharedPointer<QMenu>                 m_DockMenu;

  QMenu*                                m_MenuRecentFiles = new QMenu("Recent Files", m_DefaultMenuBar);
  QMenu*                                m_MenuFile = new QMenu("File", m_DefaultMenuBar);
  QMenu*                                m_MenuEdit = new QMenu("Edit", m_DefaultMenuBar);
  QMenu*                                m_MenuView = new QMenu("View", m_DefaultMenuBar);
  QMenu*                                m_MenuBookmarks = new QMenu("Bookmarks", m_DefaultMenuBar);
  QMenu*                                m_MenuPipeline = new QMenu("Pipeline", m_DefaultMenuBar);
  QMenu*                                m_MenuHelp = new QMenu("Help", m_DefaultMenuBar);
  QMenu*                                m_MenuAdvanced = new QMenu("Advanced", m_DefaultMenuBar);

  QAction*                              m_ActionNew = new QAction("New...", m_DefaultMenuBar);
  QAction*                              m_ActionOpen = new QAction("Open...", m_DefaultMenuBar);
  QAction*                              m_ActionSave = new QAction("Save", m_DefaultMenuBar);
  QAction*                              m_ActionSaveAs = new QAction("Save As...", m_DefaultMenuBar);
  QAction*                              m_ActionLoadTheme = new QAction("Load Theme", m_DefaultMenuBar);
  QAction*                              m_ActionSaveTheme = new QAction("Save Theme", m_DefaultMenuBar);
  QAction*                              m_ActionClearRecentFiles = new QAction("Clear Recent Files", m_DefaultMenuBar);
  QAction*                              m_ActionExit = new QAction("Exit " + QApplication::applicationName(), m_DefaultMenuBar);
  QAction*                              m_ActionShowSIMPLViewHelp = new QAction(QApplication::applicationName() + " Help", m_DefaultMenuBar);
  QAction*                              m_ActionAboutSIMPLView = new QAction("About " + QApplication::applicationName(), m_DefaultMenuBar);
  QAction*                              m_ActionCheckForUpdates = new QAction("Check For Updates", m_DefaultMenuBar);
  QAction*                              m_ActionPluginInformation = new QAction("Plugin Information", m_DefaultMenuBar);
  QAction*                              m_ActionClearCache = new QAction("Clear Cache", m_DefaultMenuBar);

  QAction*                              m_ActionCut = new QAction("Cut", m_DefaultMenuBar);
  QAction*                              m_ActionCopy = new QAction("Copy", m_DefaultMenuBar);
  QAction*                              m_ActionPaste = new QAction("Paste", m_DefaultMenuBar);
  QAction*                              m_ActionClearPipeline = new QAction("Clear Pipeline", m_DefaultMenuBar);

  QAction*                              m_ActionAddBookmark = new QAction("Add Bookmark", m_DefaultMenuBar);
  QAction*                              m_ActionAddBookmarkFolder = new QAction("Add Bookmark Folder", m_DefaultMenuBar);
  QAction*                              m_ActionClearBookmarks = new QAction("Clear Bookmarks", m_DefaultMenuBar);

  int m_minSplashTime;

  void createDefaultMenuBar();

  QMenu* createMacDockMenu();

  void readSettings();
  void writeSettings();

  SIMPLViewApplication(const SIMPLViewApplication&) = delete; // Copy Constructor Not Implemented
  void operator=(const SIMPLViewApplication&);                // Move assignment Not Implemented
};

#endif /* _SIMPLViewApplication_H */
