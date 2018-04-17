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

#include "SIMPLView/SIMPLView_UI.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SIMPLViewMenuItems.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MacSIMPLViewApplication::MacSIMPLViewApplication(int& argc, char** argv) :
SIMPLViewApplication(argc, argv)
{
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

    newInstanceFromFile(filePath);
  }

  return QApplication::event(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::initializeDummyDockWidgetActions()
{
  if (m_SIMPLViewInstances.size() > 0)
  {
    m_DummyDockWidgetActions = m_SIMPLViewInstances[0]->getDummyDockWidgetActions();
  }
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
  if (instance->isActiveWindow())
  {
    PipelineModel* model = instance->getPipelineModel();
    m_MenuEdit->insertAction(m_EditSeparator, model->getActionRedo());
    m_MenuEdit->insertAction(model->getActionRedo(), model->getActionUndo());

    m_ActiveWindow = instance;
    toSIMPLViewMenuState(instance);
  }
  else if (m_SIMPLViewInstances.size() == 1)
  {
    /* If the inactive signal got fired and there are no more windows,
     * this means that the last window has been closed. */
    m_ActiveWindow = nullptr;
    m_PreviousActiveWindow = nullptr;
    toEmptyMenuState();
  }
  else
  {
    PipelineModel* model = instance->getPipelineModel();

    m_MenuEdit->removeAction(model->getActionRedo());
    m_MenuEdit->removeAction(model->getActionUndo());

    instance->removeDockWidgetActions(m_MenuView);

    m_PreviousActiveWindow = instance;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MacSIMPLViewApplication::toSIMPLViewMenuState(SIMPLView_UI* instance)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  PipelineModel* model = instance->getPipelineModel();
  if (isCurrentlyRunning(instance) == false && model->rowCount() > 0)
  {
    menuItems->getActionClearPipeline()->setEnabled(true);
  }
  else
  {
    menuItems->getActionClearPipeline()->setDisabled(true);
  }

  for (int i = 0; i < m_DummyDockWidgetActions.size(); i++)
  {
    m_MenuView->removeAction(m_DummyDockWidgetActions[i]);
  }

  menuItems->getActionShowFilterList()->setDisabled(true);
  menuItems->getActionShowFilterLibrary()->setDisabled(true);
  menuItems->getActionShowBookmarks()->setDisabled(true);
  menuItems->getActionAddBookmark()->setDisabled(true);
  menuItems->getActionNewFolder()->setDisabled(true);

  menuItems->getActionSave()->setEnabled(true);
  menuItems->getActionSaveAs()->setEnabled(true);
  menuItems->getActionCut()->setEnabled(true);
  menuItems->getActionCopy()->setEnabled(true);
  menuItems->getActionPaste()->setEnabled(menuItems->getCanPaste());

  instance->insertDockWidgetActions(m_MenuView);
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

  m_MenuView->addActions(m_DummyDockWidgetActions);

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
