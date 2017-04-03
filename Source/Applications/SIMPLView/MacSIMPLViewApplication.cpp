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
#include "MacSIMPLViewApplication.h"

#if ! defined(_MSC_VER)
#include <unistd.h>
#endif

#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"

#include "Applications/SIMPLView/SIMPLView_UI.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SIMPLViewMenuItems.h"

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_MacSIMPLViewApplication.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MacSIMPLViewApplication::MacSIMPLViewApplication(int& argc, char** argv) :
SIMPLViewApplication(argc, argv),
m_GlobalMenu(nullptr)
{
  // Create the global menu
  createGlobalMenu();

  // Add custom actions to a dock menu
  m_DockMenu = QSharedPointer<QMenu>(createCustomDockMenu());
  m_DockMenu.data()->setAsDockMenu();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MacSIMPLViewApplication::~MacSIMPLViewApplication()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::updateRecentFileList(const QString& file)
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
void MacSIMPLViewApplication::on_actionClearRecentFiles_triggered()
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
bool MacSIMPLViewApplication::event(QEvent* event)
{
  if (event->type() == QEvent::Close)
  {
    /* We need to write the toolbox's settings here, because we need to write
     * whether the toolbox is showing or not, and that can only be done before
     * the toolbox enters its closeEvent function (the toolbox is already hidden
     * when the closeEvent occurs) */
    SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
    toolbox->writeSettings();

    // We are already handling this event past this point, so don't pass it on
    return false;
  }
  else if (event->type() == QEvent::FileOpen)
  {
    QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
    QString filePath = openEvent->file();

    newInstanceFromFile(filePath, true, true);
  }

  return QApplication::event(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::unregisterSIMPLViewWindow(SIMPLView_UI* window)
{
  m_SIMPLViewInstances.removeAll(window);

  if (m_PreviousActiveWindow == window)
  {
    m_PreviousActiveWindow = nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::dream3dWindowChanged(SIMPLView_UI* instance)
{
  SVPipelineViewWidget* viewWidget = instance->getPipelineViewWidget();

  if (instance->isActiveWindow())
  {
    SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
    menuItems->getActionUndo()->setText(viewWidget->getActionUndo()->text());
    menuItems->getActionRedo()->setText(viewWidget->getActionRedo()->text());

    m_ActiveWindow = instance;
    toSIMPLViewMenuState(instance);
  }
  else if (m_SIMPLViewInstances.size() == 1 && m_Toolbox->isHidden() == true)
  {
    /* If the inactive signal got fired and there are no more windows,
     * this means that the last window has been closed. */
    m_ActiveWindow = nullptr;
    m_PreviousActiveWindow = nullptr;
    toEmptyMenuState();
  }
  else
  {
    m_PreviousActiveWindow = instance;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::toolboxWindowChanged()
{
  if (m_Toolbox->isActiveWindow())
  {
    toToolboxMenuState();
    m_ActiveWindow = nullptr;
  }
  else if (m_SIMPLViewInstances.size() <= 0)
  {
    toEmptyMenuState();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::toToolboxMenuState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  BookmarksModel* model = BookmarksModel::Instance();

  QModelIndex currentBookmark = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->currentIndex();

  menuItems->getActionSave()->setDisabled(true);
  menuItems->getActionSaveAs()->setDisabled(true);
  menuItems->getActionShowIssues()->setDisabled(true);
  menuItems->getActionShowIssues()->setChecked(false);
  menuItems->getActionShowStdOutput()->setDisabled(true);
  menuItems->getActionShowStdOutput()->setChecked(false);
  menuItems->getActionClearPipeline()->setDisabled(true);

  menuItems->getActionShowFilterList()->setEnabled(true);
  menuItems->getActionShowFilterLibrary()->setEnabled(true);
  menuItems->getActionShowBookmarks()->setEnabled(true);

  if (currentBookmark.isValid() == false || model->index(currentBookmark.row(), BookmarksItem::Path, currentBookmark.parent()).data().toString().isEmpty() == true)
  {
    menuItems->getActionAddBookmark()->setEnabled(true);
    menuItems->getActionNewFolder()->setEnabled(true);
  }
  else
  {
    menuItems->getActionAddBookmark()->setDisabled(true);
    menuItems->getActionNewFolder()->setDisabled(true);
  }

  menuItems->getActionCut()->setDisabled(true);
  menuItems->getActionCopy()->setDisabled(true);
  menuItems->getActionPaste()->setDisabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::toSIMPLViewMenuState(SIMPLView_UI* instance)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  if (isCurrentlyRunning(instance) == false && instance->getPipelineViewWidget()->getFilterPipeline()->getFilterContainer().size() > 0)
  {
    menuItems->getActionClearPipeline()->setEnabled(true);
  }
  else
  {
    menuItems->getActionClearPipeline()->setDisabled(true);
  }

  menuItems->getActionShowFilterList()->setDisabled(true);
  menuItems->getActionShowFilterLibrary()->setDisabled(true);
  menuItems->getActionShowBookmarks()->setDisabled(true);
  menuItems->getActionAddBookmark()->setDisabled(true);
  menuItems->getActionNewFolder()->setDisabled(true);

  menuItems->getActionSave()->setEnabled(true);
  menuItems->getActionSaveAs()->setEnabled(true);
  menuItems->getActionShowIssues()->setEnabled(true);
  menuItems->getActionCut()->setEnabled(true);
  menuItems->getActionCopy()->setEnabled(true);
  menuItems->getActionPaste()->setEnabled(menuItems->getCanPaste());
  menuItems->getActionShowStdOutput()->setEnabled(true);

  // Update the issues menu item with the correct value
  QAction* issuesToggle = m_ActiveWindow->getIssuesDockWidget()->toggleViewAction();
  menuItems->getActionShowIssues()->setChecked(issuesToggle->isChecked());

  // Update the standard output menu item with the correct value
  QAction* stdOutToggle = m_ActiveWindow->getStandardOutputDockWidget()->toggleViewAction();
  menuItems->getActionShowStdOutput()->setChecked(stdOutToggle->isChecked());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::toEmptyMenuState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  menuItems->getActionShowFilterList()->setDisabled(true);
  menuItems->getActionShowFilterLibrary()->setDisabled(true);
  menuItems->getActionShowBookmarks()->setDisabled(true);
  menuItems->getActionAddBookmark()->setDisabled(true);
  menuItems->getActionNewFolder()->setDisabled(true);
  menuItems->getActionSave()->setDisabled(true);
  menuItems->getActionSaveAs()->setDisabled(true);
  menuItems->getActionShowIssues()->setDisabled(true);
  menuItems->getActionShowIssues()->setChecked(false);
  menuItems->getActionShowStdOutput()->setDisabled(true);
  menuItems->getActionShowStdOutput()->setChecked(false);
  menuItems->getActionClearPipeline()->setDisabled(true);
  menuItems->getActionCut()->setDisabled(true);
  menuItems->getActionCopy()->setDisabled(true);
  menuItems->getActionPaste()->setDisabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenu* MacSIMPLViewApplication::createCustomDockMenu()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenu* dockMenu = new QMenu();
  dockMenu->addAction(menuItems->getActionNew());
  dockMenu->addAction(menuItems->getActionOpen());
  dockMenu->addSeparator();
  dockMenu->addAction(menuItems->getActionShowSIMPLViewHelp());
  dockMenu->addSeparator();
  dockMenu->addAction(menuItems->getActionCheckForUpdates());
  dockMenu->addSeparator();
  dockMenu->addAction(menuItems->getActionPluginInformation());

  return dockMenu;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::createGlobalMenu()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  QMenu* menuFile = new QMenu("File", m_GlobalMenu.data());
  m_MenuEdit = new QMenu("Edit", m_GlobalMenu.data());
  QMenu* menuView = new QMenu("View", m_GlobalMenu.data());
  QMenu* menuToolbox = new QMenu("Toolbox", m_GlobalMenu.data());
  QMenu* menuBookmarks = new QMenu("Bookmarks", m_GlobalMenu.data());
  QMenu* menuPipeline = new QMenu("Pipeline", m_GlobalMenu.data());
  QMenu* menuHelp = new QMenu("Help", m_GlobalMenu.data());
  QMenu* menuAdvanced = new QMenu("Advanced", m_GlobalMenu.data());
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
  QAction* actionShowFilterLibrary = menuItems->getActionShowFilterLibrary();
  QAction* actionShowFilterList = menuItems->getActionShowFilterList();
  QAction* actionShowBookmarks = menuItems->getActionShowBookmarks();
  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();

  QAction* actionCut = menuItems->getActionCut();
  QAction* actionCopy = menuItems->getActionCopy();
  QAction* actionPaste = menuItems->getActionPaste();
  QAction* actionUndo = menuItems->getActionUndo();
  QAction* actionRedo = menuItems->getActionRedo();

  m_GlobalMenu = QSharedPointer<QMenuBar>(new QMenuBar());

  // Create File Menu
  m_GlobalMenu->addMenu(menuFile);
  menuFile->addAction(actionNew);
  menuFile->addAction(actionOpen);
  menuFile->addSeparator();
  menuFile->addAction(actionSave);
  menuFile->addAction(actionSaveAs);
  menuFile->addSeparator();
  menuFile->addAction(menuRecentFiles->menuAction());
  menuRecentFiles->addSeparator();
  menuRecentFiles->addAction(actionClearRecentFiles);
  menuFile->addSeparator();
  menuFile->addAction(actionExit);

  // Create Edit Menu
  m_GlobalMenu->addMenu(m_MenuEdit);
  m_MenuEdit->addAction(actionUndo);
  m_MenuEdit->addAction(actionRedo);
  m_EditSeparator = m_MenuEdit->addSeparator();
  m_MenuEdit->addAction(actionCut);
  m_MenuEdit->addAction(actionCopy);
  m_MenuEdit->addAction(actionPaste);

  // Create View Menu
  m_GlobalMenu->addMenu(menuView);
  menuView->addAction(actionShowToolbox);
  menuView->addMenu(menuToolbox);
  menuToolbox->addAction(actionShowFilterList);
  menuToolbox->addAction(actionShowFilterLibrary);
  menuToolbox->addAction(actionShowBookmarks);
  menuView->addSeparator();
  menuView->addAction(actionShowIssues);
  menuView->addAction(actionShowStdOutput);

  // Create Bookmarks Menu
  m_GlobalMenu->addMenu(menuBookmarks);
  menuBookmarks->addAction(actionAddBookmark);
  menuBookmarks->addSeparator();
  menuBookmarks->addAction(actionNewFolder);

  // Create Pipeline Menu
  m_GlobalMenu->addMenu(menuPipeline);
  menuPipeline->addAction(actionClearPipeline);

  // Create Help Menu
  m_GlobalMenu->addMenu(menuHelp);
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
}
