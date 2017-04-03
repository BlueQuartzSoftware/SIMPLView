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
#include "StandardSIMPLViewApplication.h"

#if ! defined(_MSC_VER)
#include <unistd.h>
#endif

#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"

#include "Applications/SIMPLView/SIMPLView_UI.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SIMPLViewMenuItems.h"


// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_StandardSIMPLViewApplication.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StandardSIMPLViewApplication::StandardSIMPLViewApplication(int& argc, char** argv) :
  SIMPLViewApplication(argc, argv)
{
  m_Toolbox->setMenuBar(getToolboxMenuBar());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StandardSIMPLViewApplication::~StandardSIMPLViewApplication()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  toolbox->writeSettings();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StandardSIMPLViewApplication::updateRecentFileList(const QString& file)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenu* recentFilesMenu = menuItems->getMenuRecentFiles();
  QAction* clearRecentFilesAction = menuItems->getActionClearRecentFiles();

  // Clear the Recent Items Menu
  recentFilesMenu->clear();

  // Get the list from the static object
  QStringList files = QtSRecentFileList::instance()->fileList();
  foreach(QString file, files)
  {
    QAction* action = recentFilesMenu->addAction(QtSRecentFileList::instance()->parentAndFileName(file));
    action->setData(file);
    action->setVisible(true);
    connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
  }

  recentFilesMenu->addSeparator();
  recentFilesMenu->addAction(clearRecentFilesAction);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StandardSIMPLViewApplication::on_actionClearRecentFiles_triggered()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenu* recentFilesMenu = menuItems->getMenuRecentFiles();
  QAction* clearRecentFilesAction = menuItems->getActionClearRecentFiles();

  // Clear the Recent Items Menu
  recentFilesMenu->clear();
  recentFilesMenu->addSeparator();
  recentFilesMenu->addAction(clearRecentFilesAction);

  // Clear the actual list
  QtSRecentFileList* recents = QtSRecentFileList::instance();
  recents->clear();

  // Write out the empty list
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());
  recents->writeList(prefs.data());

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StandardSIMPLViewApplication::dream3dWindowChanged(SIMPLView_UI* instance)
{
  if (instance->isActiveWindow())
  {
    SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

    m_ActiveWindow = instance;

    // Update the issues menu item with the correct value
    QAction* issuesToggle = m_ActiveWindow->getIssuesDockWidget()->toggleViewAction();
    menuItems->getActionShowIssues()->setChecked(issuesToggle->isChecked());

    // Update the standard output menu item with the correct value
    QAction* stdOutToggle = m_ActiveWindow->getStandardOutputDockWidget()->toggleViewAction();
    menuItems->getActionShowStdOutput()->setChecked(stdOutToggle->isChecked());

    menuItems->getActionPaste()->setEnabled(menuItems->getCanPaste());
  }
  else
  {
    m_PreviousActiveWindow = instance;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StandardSIMPLViewApplication::toolboxWindowChanged()
{
  if (m_Toolbox->isActiveWindow())
  {
    m_ActiveWindow = nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StandardSIMPLViewApplication::unregisterSIMPLViewWindow(SIMPLView_UI* window)
{
  m_SIMPLViewInstances.removeAll(window);

  if (m_PreviousActiveWindow == window)
  {
    m_PreviousActiveWindow = nullptr;
  }

  if (m_SIMPLViewInstances.size() <= 0)
  {
    quit();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenuBar* StandardSIMPLViewApplication::getSIMPLViewMenuBar(SIMPLView_UI* instance)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenuBar* menuBar = new QMenuBar();
  QMenu* menuFile = new QMenu("File", menuBar);
  QMenu* menuEdit = new QMenu("Edit", menuBar);
  QMenu* menuView = new QMenu("View", menuBar);
  QMenu* menuPipeline = new QMenu("Pipeline", menuBar);
  QMenu* menuHelp = new QMenu("Help", menuBar);
  QMenu* menuAdvanced = new QMenu("Advanced", menuBar);
  QAction* actionNew = menuItems->getActionNew();
  QAction* actionOpen = menuItems->getActionOpen();
  QAction* actionSave = menuItems->getActionSave();
  QAction* actionSaveAs = menuItems->getActionSaveAs();
  QMenu* menuRecentFiles = menuItems->getMenuRecentFiles();
  QAction* actionClearRecentFiles = menuItems->getActionClearRecentFiles();
  QAction* actionExit = menuItems->getActionExit();
  QAction* actionClearPipeline = menuItems->getActionClearPipeline();
  QAction* actionShowSIMPLViewHelp = menuItems->getActionShowSIMPLViewHelp();
  QAction* actionCheckForUpdates = menuItems->getActionCheckForUpdates();
  QAction* actionClearCache = menuItems->getActionClearCache();
  QAction* actionClearBookmarks = menuItems->getActionClearBookmarks();
  QAction* actionAboutSIMPLView = menuItems->getActionAboutSIMPLView();
  QAction* actionPluginInformation = menuItems->getActionPluginInformation();
  QAction* actionShowIssues = menuItems->getActionShowIssues();
  QAction* actionShowStdOutput = menuItems->getActionShowStdOutput();
  QAction* actionShowToolbox = menuItems->getActionShowToolbox();
  QAction* actionCut = menuItems->getActionCut();
  QAction* actionCopy = menuItems->getActionCopy();
  QAction* actionPaste = menuItems->getActionPaste();
  QAction* actionUndo = instance->getPipelineViewWidget()->getActionUndo();
  QAction* actionRedo = instance->getPipelineViewWidget()->getActionRedo();

  actionUndo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
  actionRedo->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));

  // Create File Menu
  menuBar->addMenu(menuFile);
  menuFile->addAction(actionNew);
  menuFile->addAction(actionOpen);
  menuFile->addSeparator();
  menuFile->addAction(actionSave);
  menuFile->addAction(actionSaveAs);
  menuFile->addSeparator();
  menuFile->addAction(menuRecentFiles->menuAction());
  menuFile->addSeparator();
  menuFile->addAction(actionExit);
  menuRecentFiles->addSeparator();
  menuRecentFiles->addAction(actionClearRecentFiles);

  // Create Edit Menu
  menuBar->addMenu(menuEdit);
  menuEdit->addAction(actionUndo);
  menuEdit->addAction(actionRedo);
  menuEdit->addSeparator();
  menuEdit->addAction(actionCut);
  menuEdit->addAction(actionCopy);
  menuEdit->addAction(actionPaste);

  // Create View Menu
  menuBar->addMenu(menuView);
  menuView->addAction(actionShowIssues);
  menuView->addAction(actionShowStdOutput);
  menuView->addAction(actionShowToolbox);

  // Create Pipeline Menu
  menuBar->addMenu(menuPipeline);
  menuPipeline->addAction(actionClearPipeline);

  // Create Help Menu
  menuBar->addMenu(menuHelp);
  menuHelp->addAction(actionShowSIMPLViewHelp);
  menuHelp->addSeparator();
  menuHelp->addAction(actionCheckForUpdates);
  menuHelp->addSeparator();
  menuHelp->addMenu(menuAdvanced);
  menuAdvanced->addAction(actionClearCache);
  menuAdvanced->addSeparator();
  menuAdvanced->addAction(actionClearBookmarks);
  menuHelp->addSeparator();
  menuHelp->addAction(actionAboutSIMPLView);
  menuHelp->addAction(actionPluginInformation);

  return menuBar;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenuBar* StandardSIMPLViewApplication::getToolboxMenuBar()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenuBar* menuBar = new QMenuBar();
  QMenu* menuView = new QMenu("View", menuBar);
  QMenu* menuBookmarks = new QMenu("Bookmarks", menuBar);
  QAction* actionShowFilterLibrary = menuItems->getActionShowFilterLibrary();
  QAction* actionShowFilterList = menuItems->getActionShowFilterList();
  QAction* actionShowBookmarks = menuItems->getActionShowBookmarks();
  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();

  // Add the actions to their respective menus
  menuBar->addAction(menuView->menuAction());
  menuBar->addAction(menuBookmarks->menuAction());

  menuView->addAction(actionShowFilterList);
  menuView->addAction(actionShowFilterLibrary);
  menuView->addAction(actionShowBookmarks);

  menuBookmarks->addAction(actionAddBookmark);
  menuBookmarks->addSeparator();
  menuBookmarks->addAction(actionNewFolder);

  return menuBar;
}

