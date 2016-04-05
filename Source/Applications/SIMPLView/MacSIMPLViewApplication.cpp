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

#include "QtSupportLib/QRecentFileList.h"

#include "Applications/SIMPLView/SIMPLView_UI.h"
#include "Applications/SIMPLView/SIMPLViewToolbox.h"
#include "Applications/SIMPLView/SIMPLViewMenuItems.h"

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_MacSIMPLViewApplication.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MacSIMPLViewApplication::MacSIMPLViewApplication(int& argc, char** argv) :
SIMPLViewApplication(argc, argv),
m_GlobalMenu(NULL)
{
  // Create the global menu
  createGlobalMenu();

  // Add custom actions to a dock menu
  QMenu* dockMenu = createCustomDockMenu();
  qt_mac_set_dock_menu(dockMenu);
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
  QStringList files = QRecentFileList::instance()->fileList();
  foreach(QString file, files)
  {
    QAction* action = new QAction(recentFilesMenu);
    action->setText(QRecentFileList::instance()->parentAndFileName(file));
    action->setData(file);
    action->setVisible(true);
    recentFilesMenu->addAction(action);
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
  QRecentFileList* recents = QRecentFileList::instance();
  recents->clear();

  // Write out the empty list
  QSharedPointer<SIMPLViewSettings> prefs = QSharedPointer<SIMPLViewSettings>(new SIMPLViewSettings());
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

  return QApplication::event(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::unregisterSIMPLViewWindow(SIMPLView_UI* window)
{
  m_SIMPLViewInstances.removeAll(window);

  if (m_SIMPLViewInstances.size() <= 0)
  {
    m_ActiveWindow = NULL;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::dream3dWindowChanged(SIMPLView_UI* instance, QUndoStack* undoStack)
{
  if (instance->isActiveWindow())
  {
    SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

    if (NULL != m_ActionUndo)
    {
      m_MenuEdit->removeAction(m_ActionUndo);
      delete m_ActionUndo;
    }

    if (NULL != m_ActionRedo)
    {
      m_MenuEdit->removeAction(m_ActionRedo);
      delete m_ActionRedo;
    }

    m_ActionUndo = undoStack->createUndoAction(instance);
    m_ActionRedo = undoStack->createRedoAction(instance);

    m_ActionUndo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    m_ActionRedo->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));

    m_MenuEdit->insertAction(m_EditSeparator, m_ActionRedo);
    m_MenuEdit->insertAction(m_ActionRedo, m_ActionUndo);

    m_ActiveWindow = instance;
    toSIMPLViewMenuState(instance);

    // Update the issues menu item with the correct value
    QAction* issuesToggle = m_ActiveWindow->getIssuesDockWidget()->toggleViewAction();
    menuItems->getActionShowIssues()->setChecked(issuesToggle->isChecked());
  }
  else if (m_SIMPLViewInstances.size() <= 0)
  {
    /* If the inactive signal got fired and there are no more windows,
     * this means that the last window has been closed. */
    m_ActiveWindow = NULL;
    toEmptyMenuState();
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

  if (isCurrentlyRunning(instance) == true)
  {
    menuItems->getActionClearPipeline()->setDisabled(true);
  }
  else
  {
    menuItems->getActionClearPipeline()->setEnabled(true);
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
  menuItems->getActionPaste()->setEnabled(true);
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

  QMenu* menuFile = new QMenu("File", m_GlobalMenu);
  m_MenuEdit = new QMenu("Edit", m_GlobalMenu);
  QMenu* menuView = new QMenu("View", m_GlobalMenu);
  QMenu* menuToolbox = new QMenu("Toolbox", m_GlobalMenu);
  QMenu* menuBookmarks = new QMenu("Bookmarks", m_GlobalMenu);
  QMenu* menuPipeline = new QMenu("Pipeline", m_GlobalMenu);
  QMenu* menuHelp = new QMenu("Help", m_GlobalMenu);
  QMenu* menuAdvanced = new QMenu("Advanced", m_GlobalMenu);
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
  QAction* actionShowToolbox = menuItems->getActionShowToolbox();
  QAction* actionShowFilterLibrary = menuItems->getActionShowFilterLibrary();
  QAction* actionShowFilterList = menuItems->getActionShowFilterList();
  QAction* actionShowBookmarks = menuItems->getActionShowBookmarks();
  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();

  QAction* actionCut = menuItems->getActionCut();
  QAction* actionCopy = menuItems->getActionCopy();
  QAction* actionPaste = menuItems->getActionPaste();

  m_GlobalMenu = new QMenuBar(NULL);

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
