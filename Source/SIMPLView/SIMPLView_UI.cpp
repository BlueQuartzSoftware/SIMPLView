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

#include "SIMPLView_UI.h"

//-- Qt Includes
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileInfoList>
#include <QtCore/QMimeData>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QToolButton>

//-- SIMPLView Includes
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/Plugin/PluginManager.h"

#include "SVWidgetsLib/QtSupport/QtSMacros.h"
#include "SVWidgetsLib/QtSupport/QtSPluginFrame.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSStyles.h"
#include "SVWidgetsLib/QtSupport/QtSFileUtils.h"
#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Dialogs/UpdateCheck.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckData.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckDialog.h"
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/Widgets/BookmarksModel.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/BookmarksTreeView.h"
#include "SVWidgetsLib/Widgets/FilterLibraryToolboxWidget.h"
#include "SVWidgetsLib/Widgets/SIMPLViewMenuItems.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"
#include "SVWidgetsLib/Widgets/SIMPLController.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineTreeView.h"
#include "SVWidgetsLib/Widgets/PipelineListWidget.h"
#include "SVWidgetsLib/Widgets/StatusBarWidget.h"
#ifdef SIMPL_USE_QtWebEngine
#include "SVWidgetsLib/Widgets/SVUserManualDialog.h"
#else
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMessageBox>
#endif

#ifdef SIMPL_USE_MKDOCS
#define URL_GENERATOR QtSDocServer
#include "SVWidgetsLib/QtSupport/QtSDocServer.h"
#endif

#ifdef SIMPL_USE_DISCOUNT
#define URL_GENERATOR QtSHelpUrlGenerator
#include "SVWidgetsLib/QtSupport/QtSHelpUrlGenerator.h"
#endif


#include "SIMPLView/MacSIMPLViewApplication.h"
#include "SIMPLView/AboutSIMPLView.h"
#include "SIMPLView/SIMPLView.h"
#include "SIMPLView/SIMPLViewConstants.h"
#include "SIMPLView/SIMPLViewVersion.h"
#include "SIMPLView/StandardSIMPLViewApplication.h"

#include "BrandedStrings.h"

// Initialize private static member variable
QString SIMPLView_UI::m_OpenDialogLastFilePath = "";

namespace Detail
{

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void fillVersionData(UpdateCheck::SIMPLVersionData_t& data)
{
  data.complete = SIMPLView::Version::Complete();
  data.major = SIMPLView::Version::Major();
  data.minor = SIMPLView::Version::Minor();
  data.patch = SIMPLView::Version::Patch();
  data.package = SIMPLView::Version::Package();
  data.revision = SIMPLView::Version::Revision();
  data.packageComplete = SIMPLView::Version::PackageComplete();
  data.buildDate = SIMPLView::Version::BuildDate();
  data.appName = BrandedStrings::ApplicationName;
}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI::SIMPLView_UI(QWidget* parent)
: QMainWindow(parent)
, m_SIMPLController(new SIMPLController(this))
, m_ActivePlugin(nullptr)
, m_FilterManager(nullptr)
, m_FilterWidgetManager(nullptr)
#if !defined(Q_OS_MAC)
, m_InstanceMenuBar(nullptr)
#endif
, m_OpenedFilePath("")
{
  m_OpenDialogLastFilePath = QDir::homePath();

  // Register all of the Filters we know about - the rest will be loaded through plugins
  //  which all should have been loaded by now.
  m_FilterManager = FilterManager::Instance();
  // m_FilterManager->RegisterKnownFilters(m_FilterManager);

  // Register all the known filterWidgets
  m_FilterWidgetManager = FilterWidgetManager::Instance();
  m_FilterWidgetManager->RegisterKnownFilterWidgets();

  // Calls the Parent Class to do all the Widget Initialization that were created
  // using the QDesigner program
  setupUi(this);

// Set up the menu
#if !defined(Q_OS_MAC)
  // Create the menu
  m_InstanceMenuBar = standardApp->getSIMPLViewMenuBar(this);

  setMenuBar(m_InstanceMenuBar);
#endif
  dream3dApp->registerSIMPLViewWindow(this);

  // Do our own widget initializations
  setupGui();

  this->setAcceptDrops(true);

  // Read various settings
  readSettings();
  if(HideDockSetting::OnError == m_HideErrorTable)
  {
    issuesDockWidget->setHidden(true);
  }
  if(HideDockSetting::OnError == m_HideStdOutput)
  {
    stdOutDockWidget->setHidden(true);
  }

  // Set window modified to false
  setWindowModified(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI::~SIMPLView_UI()
{
  for(QMap<QWidget*, QTextEdit*>::iterator iter = m_StdOutputTabMap.begin(); iter != m_StdOutputTabMap.end(); ++iter)
  {
    delete iter.key();
  }

  disconnectSignalsSlots();

  writeSettings();
  cleanupPipeline();
  dream3dApp->unregisterSIMPLViewWindow(this);

  if(dream3dApp->activeWindow() == this)
  {
    dream3dApp->setActiveWindow(nullptr);
  }

#if !defined(Q_OS_MAC)
  if(m_InstanceMenuBar)
  {
    delete m_InstanceMenuBar;
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::resizeEvent(QResizeEvent* event)
{
  QMainWindow::resizeEvent(event);

  emit parentResized();

  // We need to write the window settings so that any new windows will open with these window settings
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());
  writeWindowSettings(prefs.data());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLView_UI::savePipeline()
{
  if(isWindowModified() == true)
  {
    QString filePath;
    if(m_OpenedFilePath.isEmpty())
    {
      // When the file hasn't been saved before, the same functionality as a "Save As" occurs...
      bool didSave = savePipelineAs();
      return didSave;
    }
    else
    {
      filePath = m_OpenedFilePath;
    }

    // Fix the separators
    filePath = QDir::toNativeSeparators(filePath);

    // Write the pipeline
    SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();
    viewWidget->writePipeline(filePath);

    // Set window title and save flag
    QFileInfo prefFileInfo = QFileInfo(filePath);
    setWindowTitle("[*]" + prefFileInfo.baseName() + " - " + BrandedStrings::ApplicationName);
    setWindowModified(false);

    // Add file to the recent files list
    QtSRecentFileList* list = QtSRecentFileList::instance();
    list->addFile(filePath);
  }

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLView_UI::savePipelineAs()
{
  QString proposedFile = m_OpenDialogLastFilePath + QDir::separator() + "Untitled.json";
  QString filePath = QFileDialog::getSaveFileName(this, tr("Save Pipeline To File"), proposedFile, tr("Json File (*.json);;SIMPLView File (*.dream3d);;All Files (*.*)"));
  if(true == filePath.isEmpty())
  {
    return false;
  }

  filePath = QDir::toNativeSeparators(filePath);

  // If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if(fi.suffix().isEmpty())
  {
    filePath.append(".json");
    fi.setFile(filePath);
  }

  // Write the pipeline
  SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();
  int err = viewWidget->writePipeline(filePath);

  if(err >= 0)
  {
    // Set window title and save flag
    setWindowTitle("[*]" + fi.baseName() + " - " + BrandedStrings::ApplicationName);
    setWindowModified(false);

    // Add file to the recent files list
    QtSRecentFileList* list = QtSRecentFileList::instance();
    list->addFile(filePath);

    m_OpenedFilePath = filePath;
  }
  else
  {
    return false;
  }

  // Cache the last directory
  m_OpenDialogLastFilePath = filePath;

  QMessageBox bookmarkMsgBox(this);
  bookmarkMsgBox.setWindowTitle("Pipeline Saved");
  bookmarkMsgBox.setText("The pipeline has been saved.");
  bookmarkMsgBox.setInformativeText("Would you also like to bookmark this pipeline?");
  bookmarkMsgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  bookmarkMsgBox.setDefaultButton(QMessageBox::Yes);
  int ret = bookmarkMsgBox.exec();

  if(ret == QMessageBox::Yes)
  {
    emit bookmarkNeedsToBeAdded(filePath, QModelIndex());
  }

  return true;
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void SIMPLView_UI::closeEvent(QCloseEvent* event)
{
  if(dream3dApp->isCurrentlyRunning(this) == true)
  {
    QMessageBox runningPipelineBox;
    runningPipelineBox.setWindowTitle("Pipeline is Running");
    runningPipelineBox.setText("There is a pipeline currently running.\nPlease cancel the running pipeline and try again.");
    runningPipelineBox.setStandardButtons(QMessageBox::Ok);
    runningPipelineBox.setIcon(QMessageBox::Warning);
    runningPipelineBox.exec();
    event->ignore();
    return;
  }

  QMessageBox::StandardButton choice = checkDirtyDocument();
  if(choice == QMessageBox::Cancel)
  {
    event->ignore();
    return;
  }

  event->accept();
}

// -----------------------------------------------------------------------------
//  Read our settings from a file
// -----------------------------------------------------------------------------
void SIMPLView_UI::readSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  // Have the pipeline builder read its settings from the prefs file
  readWindowSettings(prefs.data());
  readVersionSettings(prefs.data());

  // Read dock widget settings
  prefs->beginGroup("DockWidgetSettings");

  prefs->beginGroup("Issues Dock Widget");
  readDockWidgetSettings(prefs.data(), issuesDockWidget);
  readHideDockSettings(prefs.data(), m_HideErrorTable);
  prefs->endGroup();

  prefs->beginGroup("Standard Output Dock Widget");
  readDockWidgetSettings(prefs.data(), stdOutDockWidget);
  readHideDockSettings(prefs.data(), m_HideStdOutput);
  prefs->endGroup();

  prefs->endGroup();

  m_ShowFilterWidgetDeleteDialog = prefs->value("Show 'Delete Filter Widgets' Dialog", QVariant(true)).toBool();

  QtSRecentFileList::instance()->readList(prefs.data());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::readWindowSettings(QtSSettings* prefs)
{
  bool ok = false;
  prefs->beginGroup("WindowSettings");
  if(prefs->contains(QString("MainWindowGeometry")))
  {
    QByteArray geo_data = prefs->value("MainWindowGeometry", QByteArray());
    ok = restoreGeometry(geo_data);
    if(!ok)
    {
      qDebug() << "Error Restoring the Window Geometry"
               << "\n";
    }
  }

  if(prefs->contains(QString("MainWindowState")))
  {
    QByteArray layout_data = prefs->value("MainWindowState", QByteArray());
    restoreState(layout_data);
  }

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::readDockWidgetSettings(QtSSettings* prefs, QDockWidget* dw)
{
  restoreDockWidget(dw);

  QString name = dw->objectName();
  bool b = prefs->value(dw->objectName(), QVariant(false)).toBool();
  dw->setHidden(b);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::readHideDockSettings(QtSSettings* prefs, HideDockSetting& value)
{
  int showError = static_cast<int>(HideDockSetting::Ignore);
  int hideDockSetting = prefs->value("Show / Hide On Error", QVariant(showError)).toInt();
  value = static_cast<HideDockSetting>(hideDockSetting);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::readVersionSettings(QtSSettings* prefs)
{
}

// -----------------------------------------------------------------------------
//  Write our Prefs to file
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  // Have the pipeline builder write its settings to the prefs file
  writeWindowSettings(prefs.data());
  // Have the version check widet write its preferences.
  writeVersionCheckSettings(prefs.data());

  prefs->beginGroup("DockWidgetSettings");

  prefs->beginGroup("Issues Dock Widget");
  writeDockWidgetSettings(prefs.data(), issuesDockWidget);
  writeHideDockSettings(prefs.data(), m_HideErrorTable);
  prefs->endGroup();

  prefs->beginGroup("Standard Output Dock Widget");
  writeDockWidgetSettings(prefs.data(), stdOutDockWidget);
  writeHideDockSettings(prefs.data(), m_HideStdOutput);
  prefs->endGroup();

  prefs->endGroup();

  prefs->setValue("Show 'Delete Filter Widgets' Dialog", m_ShowFilterWidgetDeleteDialog);

  QtSRecentFileList::instance()->writeList(prefs.data());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeVersionCheckSettings(QtSSettings* prefs)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeWindowSettings(QtSSettings* prefs)
{
  prefs->beginGroup("WindowSettings");
  QByteArray geo_data = saveGeometry();
  QByteArray layout_data = saveState();
  prefs->setValue(QString("MainWindowGeometry"), geo_data);
  prefs->setValue(QString("MainWindowState"), layout_data);

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeDockWidgetSettings(QtSSettings* prefs, QDockWidget* dw)
{
  prefs->setValue(dw->objectName(), dw->isHidden());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeHideDockSettings(QtSSettings* prefs, HideDockSetting value)
{
  int valuei = static_cast<int>(value);
  prefs->setValue("Show / Hide On Error", valuei);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::checkForUpdatesAtStartup()
{

  UpdateCheck::SIMPLVersionData_t data = dream3dApp->FillVersionData();
  UpdateCheckDialog d(data, this);
  if(d.getAutomaticallyBtn()->isChecked())
  {
    QtSSettings updatePrefs;

    updatePrefs.beginGroup(UpdateCheckDialog::GetUpdatePreferencesGroup());
    QDate lastUpdateCheckDate = updatePrefs.value(UpdateCheckDialog::GetUpdateCheckKey(), QString("")).toDate();
    updatePrefs.endGroup();

    QDate systemDate;
    QDate currentDateToday = systemDate.currentDate();

    QDate dailyThreshold = lastUpdateCheckDate.addDays(1);
    QDate weeklyThreshold = lastUpdateCheckDate.addDays(7);
    QDate monthlyThreshold = lastUpdateCheckDate.addMonths(1);

    if((d.getHowOftenComboBox()->currentIndex() == UpdateCheckDialog::UpdateCheckDaily && currentDateToday >= dailyThreshold) ||
       (d.getHowOftenComboBox()->currentIndex() == UpdateCheckDialog::UpdateCheckWeekly && currentDateToday >= weeklyThreshold) ||
       (d.getHowOftenComboBox()->currentIndex() == UpdateCheckDialog::UpdateCheckMonthly && currentDateToday >= monthlyThreshold))
    {
      m_UpdateCheck = QSharedPointer<UpdateCheck>(new UpdateCheck(data, this));

      connect(m_UpdateCheck.data(), SIGNAL(latestVersion(UpdateCheckData*)), this, SLOT(versionCheckReply(UpdateCheckData*)));

      m_UpdateCheck->checkVersion(SIMPLView::UpdateWebsite::UpdateWebSite);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setupGui()
{
  // Automatically check for updates at startup if the user has indicated that preference before
  checkForUpdatesAtStartup();

  createSIMPLViewMenuSystem();

  if (USE_PIPELINE_TREE_WIDGET)
  {
    setupPipelineTreeView();
  }
  else
  {
    setupPipelineListWidget();
  }

  // Add the Issues widget as the observer of the controller
  m_SIMPLController->addPipelineMessageObserver(issuesWidget);

  // pipelineViewWidget->setScrollArea(pipelineViewScrollArea);

  // pipelineViewScrollArea->verticalScrollBar()->setSingleStep(5);

  // Hook up the signals from the various docks to the PipelineViewWidget that will either add a filter
  // or load an entire pipeline into the view
  connectSignalsSlots();

  // This will set the initial list of filters in the FilterListToolboxWidget
  // Tell the Filter Library that we have more Filters (potentially)
  getFilterLibraryToolboxWidget()->refreshFilterGroups();

  // Read the toolbox settings and update the filter list
  filterListWidget->updateFilterList(true);

  // Setup the undo stack.  This must occur after the connectSignalsSlots function.
  m_SIMPLController->setupUndoStack();

  issuesDockWidget->toggleViewAction()->setText("Show " + issuesDockWidget->toggleViewAction()->text());
  stdOutDockWidget->toggleViewAction()->setText("Show " + stdOutDockWidget->toggleViewAction()->text());
  dataBrowserDockWidget->toggleViewAction()->setText("Show " + dataBrowserDockWidget->toggleViewAction()->text());
  pipelineDockWidget->toggleViewAction()->setText("Show " + pipelineDockWidget->toggleViewAction()->text());

  // Shortcut to close the window
  new QShortcut(QKeySequence(QKeySequence::Close), this, SLOT(close()));

  m_StatusBar = new StatusBarWidget();
  this->statusBar()->insertPermanentWidget(0, m_StatusBar, 0);

  m_StatusBar->setButtonAction(issuesDockWidget, StatusBarWidget::Button::Issues);
  m_StatusBar->setButtonAction(stdOutDockWidget, StatusBarWidget::Button::Console);
  m_StatusBar->setButtonAction(dataBrowserDockWidget, StatusBarWidget::Button::DataStructure);
  m_StatusBar->setButtonAction(pipelineDockWidget, StatusBarWidget::Button::Pipeline);

  connect(issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), m_StatusBar, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), this, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(issuesWidget, SIGNAL(showTable(bool)), issuesDockWidget, SLOT(setVisible(bool)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setupPipelineListWidget()
{
  m_ListWidget = new PipelineListWidget(pipelineInteralWidget);
  verticalLayout->insertWidget(0, m_ListWidget);

  SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();

  // Create the model
  PipelineModel* model = new PipelineModel(this);
  model->setMaxNumberOfPipelines(1);
  viewWidget->setModel(model);

  // Set the Data Browser widget into the Pipeline View widget
  viewWidget->setDataStructureWidget(dataBrowserWidget);

  // Set the IssuesWidget as a PipelineMessageObserver Object.
  viewWidget->addPipelineMessageObserver(issuesWidget);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setupPipelineTreeView()
{
  m_PipelineTreeView = new PipelineTreeView(pipelineInteralWidget);
  m_PipelineTreeView->setAlternatingRowColors(true);
  verticalLayout->insertWidget(0, m_PipelineTreeView);

  connect(m_PipelineTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(listenPipelineContextMenuRequested(const QPoint&)));

  PipelineModel* model = new PipelineModel(m_PipelineTreeView);
  m_PipelineTreeView->setModel(model);

  m_PipelineTreeView->header()->setSectionResizeMode(PipelineItem::TreeItemData::Name, QHeaderView::Stretch);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenBookmarksContextMenuRequested(const QModelIndex &currentIndex, QModelIndexList selectedIndexes, const QPoint &mappedPos)
{
  BookmarksModel* model = BookmarksModel::Instance();

  QAction* actionAddBookmark = m_MenuItems->getActionAddBookmark();
  QAction* actionNewFolder = m_MenuItems->getActionNewFolder();
  QAction* actionRenameBookmark = m_MenuItems->getActionRenameBookmark();
  QAction* actionRemoveBookmark = m_MenuItems->getActionRemoveBookmark();
  QAction* actionLocateFile = m_MenuItems->getActionLocateFile();
  QAction* actionShowBookmarkInFileSystem = m_MenuItems->getActionShowBookmarkInFileSystem();
  QAction* actionOpenBookmark = m_MenuItems->getActionOpenBookmark();
  QAction* actionExecuteBookmark = m_MenuItems->getActionOpenExecuteBookmark();

  QMenu menu;
  if(currentIndex.isValid() == false)
  {
    menu.addAction(actionAddBookmark);
    {
      QAction* separator = new QAction(this);
      separator->setSeparator(true);
      menu.addAction(separator);
    }
    menu.addAction(actionNewFolder);
  }
  else
  {
    QModelIndex actualIndex = model->index(currentIndex.row(), BookmarksItem::Path, currentIndex.parent());
    QString path = actualIndex.data().toString();
    if(selectedIndexes.size() > 1)
    {
      actionRemoveBookmark->setText("Remove Items");
      menu.addAction(actionRemoveBookmark);
    }
    else if(path.isEmpty() == false)
    {
      bool itemHasErrors = model->data(actualIndex, Qt::UserRole).value<bool>();
      if(itemHasErrors == true)
      {
        menu.addAction(actionLocateFile);

        {
          QAction* separator = new QAction(this);
          separator->setSeparator(true);
          menu.addAction(separator);
        }

        actionRemoveBookmark->setText("Remove Bookmark");
        menu.addAction(actionRemoveBookmark);
      }
      else
      {
        menu.addAction(actionOpenBookmark);
        menu.addAction(actionExecuteBookmark);
        {
          QAction* separator = new QAction(this);
          separator->setSeparator(true);
          menu.addAction(separator);
        }
        actionRenameBookmark->setText("Rename Bookmark");
        menu.addAction(actionRenameBookmark);
        actionRemoveBookmark->setText("Remove Bookmark");
        menu.addAction(actionRemoveBookmark);
        {
          QAction* separator = new QAction(this);
          separator->setSeparator(true);
          menu.addAction(separator);
        }

        menu.addAction(actionShowBookmarkInFileSystem);
      }
    }
    else if(path.isEmpty())
    {
      menu.addAction(actionAddBookmark);

      actionRenameBookmark->setText("Rename Folder");
      menu.addAction(actionRenameBookmark);

      {
        QAction* separator = new QAction(this);
        separator->setSeparator(true);
        menu.addAction(separator);
      }

      actionRemoveBookmark->setText("Remove Folder");
      menu.addAction(actionRemoveBookmark);

      {
        QAction* separator = new QAction(this);
        separator->setSeparator(true);
        menu.addAction(separator);
      }

      menu.addAction(actionNewFolder);
    }
  }

  menu.exec(mappedPos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::createSIMPLViewMenuSystem()
{
  m_SIMPLViewMenu = QSharedPointer<QMenuBar>(new QMenuBar());
  m_MenuItems = new SIMPLViewMenuItems(this);

  QMenu* menuFile = m_MenuItems->getMenuFile();
  QMenu* menuEdit = m_MenuItems->getMenuEdit();
  QMenu* menuView = m_MenuItems->getMenuView();
  QMenu* menuBookmarks = m_MenuItems->getMenuBookmarks();
  QMenu* menuPipeline = m_MenuItems->getMenuPipeline();
  QMenu* menuHelp = m_MenuItems->getMenuHelp();
  QMenu* menuAdvanced = m_MenuItems->getMenuAdvanced();
  QAction* actionNew = m_MenuItems->getActionNew();
  QAction* actionOpen = m_MenuItems->getActionOpen();
  QAction* actionSave = m_MenuItems->getActionSave();
  QAction* actionSaveAs = m_MenuItems->getActionSaveAs();
  QMenu* menuRecentFiles = m_MenuItems->getMenuRecentFiles();
  QAction* actionClearRecentFiles = m_MenuItems->getActionClearRecentFiles();
  QAction* actionExit = m_MenuItems->getActionExit();
  QAction* actionClearPipeline = m_MenuItems->getActionClearPipeline();
  QAction* actionShowSIMPLViewHelp = m_MenuItems->getActionShowSIMPLViewHelp();
  QAction* actionCheckForUpdates = m_MenuItems->getActionCheckForUpdates();
  QAction* actionClearCache = m_MenuItems->getActionClearCache();
  QAction* actionClearBookmarks = m_MenuItems->getActionClearBookmarks();
  QAction* actionAboutSIMPLView = m_MenuItems->getActionAboutSIMPLView();
  QAction* actionPluginInformation = m_MenuItems->getActionPluginInformation();
  QAction* actionAddBookmark = m_MenuItems->getActionAddBookmark();
  QAction* actionNewFolder = m_MenuItems->getActionNewFolder();
  QAction* actionNewPipeline = m_MenuItems->getActionNewPipeline();
  QAction* actionShowBookmarkInFileSystem = m_MenuItems->getActionShowBookmarkInFileSystem();
  QAction* actionRenameBookmark = m_MenuItems->getActionRenameBookmark();
  QAction* actionRemoveBookmark = m_MenuItems->getActionRemoveBookmark();
  QAction* actionOpenBookmark = m_MenuItems->getActionOpenBookmark();
  QAction* actionOpenExecuteBookmark = m_MenuItems->getActionOpenExecuteBookmark();

  QAction* actionCut = m_MenuItems->getActionCut();
  QAction* actionCopy = m_MenuItems->getActionCopy();
  QAction* actionPaste = m_MenuItems->getActionPaste();

  // Menu Items Connections
  connect(actionNew, SIGNAL(triggered()), this, SLOT(listenNewInstanceTriggered()));
  connect(actionNewPipeline, SIGNAL(triggered()), this, SLOT(listenNewPipelineTriggered()));
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(listenOpenPipelineTriggered()));
  connect(actionSave, SIGNAL(triggered()), this, SLOT(listenSavePipelineTriggered()));
  connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(listenSavePipelineAsTriggered()));
  connect(actionExit, SIGNAL(triggered()), this, SLOT(listenExitApplicationTriggered()));
  connect(actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(listenClearRecentFilesTriggered()));
  connect(actionAboutSIMPLView, SIGNAL(triggered()), this, SLOT(listenAboutSIMPLViewTriggered()));
  connect(actionCheckForUpdates, SIGNAL(triggered()), this, SLOT(listenCheckForUpdatesTriggered()));
  connect(actionShowSIMPLViewHelp, SIGNAL(triggered()), this, SLOT(listenShowSIMPLViewHelpTriggered()));
  connect(actionPluginInformation, SIGNAL(triggered()), this, SLOT(listenPluginInfoTriggered()));
  connect(actionClearPipeline, SIGNAL(triggered()), this, SLOT(listenClearPipelineTriggered()));
  connect(actionShowBookmarkInFileSystem, SIGNAL(triggered()), this, SLOT(listenShowBookmarkInFileSystemTriggered()));
  connect(actionRenameBookmark, SIGNAL(triggered()), this, SLOT(listenRenameBookmarkTriggered()));
  connect(actionRemoveBookmark, SIGNAL(triggered()), this, SLOT(listenRemoveBookmarkTriggered()));
  connect(actionClearCache, SIGNAL(triggered()), this, SLOT(listenClearCacheTriggered()));
  connect(actionClearBookmarks, SIGNAL(triggered()), this, SLOT(listenClearBookmarksTriggered()));
  connect(actionOpenBookmark, SIGNAL(triggered()), this, SLOT(listenOpenBookmarkTriggered()));
  connect(actionOpenExecuteBookmark, SIGNAL(triggered()), this, SLOT(listenOpenExecuteBookmarkTriggered()));
  connect(actionAddBookmark, SIGNAL(triggered()), this, SLOT(listenAddBookmarkTriggered()));
  connect(actionNewFolder, SIGNAL(triggered()), this, SLOT(listenNewFolderTriggered()));
  connect(actionCut, SIGNAL(triggered()), this, SLOT(listenCutTriggered()));
  connect(actionCopy, SIGNAL(triggered()), this, SLOT(listenCopyTriggered()));
  connect(actionPaste, SIGNAL(triggered()), this, SLOT(listenPasteTriggered()));

  // Create File Menu
  m_SIMPLViewMenu->addMenu(menuFile);
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
  m_SIMPLViewMenu->addMenu(menuEdit);
  menuEdit->addSeparator();
  menuEdit->addAction(actionCut);
  menuEdit->addAction(actionCopy);
  menuEdit->addAction(actionPaste);

  // Create View Menu
//  m_SIMPLViewMenu->addMenu(menuView);
//  menuView->addAction(actionShowToolbox);
//  menuView->addMenu(menuToolbox);
//  menuToolbox->addAction(actionShowFilterList);
//  menuToolbox->addAction(actionShowFilterLibrary);
//  menuToolbox->addAction(actionShowBookmarks);
//  menuView->addSeparator();

  // Create Bookmarks Menu
  m_SIMPLViewMenu->addMenu(menuBookmarks);
  menuBookmarks->addAction(actionAddBookmark);
  menuBookmarks->addSeparator();
  menuBookmarks->addAction(actionNewFolder);

  // Create Pipeline Menu
  m_SIMPLViewMenu->addMenu(menuPipeline);
  menuPipeline->addAction(actionClearPipeline);

  // Create Help Menu
  m_SIMPLViewMenu->addMenu(menuHelp);
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

  QClipboard* clipboard = QApplication::clipboard();
  connect(clipboard, SIGNAL(dataChanged()), this, SLOT(updatePasteAvailability()));

  // Run this once, so that the Paste button availability is updated for what is currently on the system clipboard
  updatePasteAvailability();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------s
void SIMPLView_UI::updatePasteAvailability()
{
  QClipboard* clipboard = QApplication::clipboard();
  QString text = clipboard->text();

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromString(text);

  QAction* actionPaste = m_SIMPLViewMenu->findChild<QAction*>("Paste");
  if(text.isEmpty() || FilterPipeline::NullPointer() == pipeline)
  {
    actionPaste->setDisabled(true);
  }
  else
  {
    actionPaste->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::disconnectSignalsSlots()
{
  DocRequestManager* docRequester = DocRequestManager::Instance();
  PipelineModel* model = getPipelineModel();

  disconnect(docRequester, SIGNAL(showFilterDocs(const QString&)), this, SLOT(showFilterHelp(const QString&)));

  disconnect(docRequester, SIGNAL(showFilterDocUrl(const QUrl&)), this, SLOT(showFilterHelpUrl(const QUrl&)));

  disconnect(this, SIGNAL(bookmarkNeedsToBeAdded(const QString&, const QModelIndex&)), getBookmarksToolboxWidget(), SLOT(addBookmark(const QString&, const QModelIndex&)));

  disconnect(m_SIMPLController, &SIMPLController::statusMessageGenerated, 0, 0);
  disconnect(m_SIMPLController, &SIMPLController::standardOutputMessageGenerated, 0, 0);

  disconnect(m_SIMPLController, &SIMPLController::undoActionGenerated, 0, 0);

  disconnect(m_SIMPLController, &SIMPLController::redoActionGenerated, 0, 0);

  // Connection that allows the Pipeline Tree controller to clear the Issues Table
  disconnect(m_SIMPLController, &SIMPLController::clearIssuesTriggered, issuesWidget, &IssuesWidget::clearIssues);

  // Connection that displays issues in the Issue Table when the preflight is finished
  disconnect(m_SIMPLController, &SIMPLController::preflightFinished, 0, 0);

  // Connection that refreshes the Data Browser when the preflight is finished
  disconnect(m_SIMPLController, &SIMPLController::preflightFinished, 0, 0);

  // Connection that does post-preflight updates on this instance of SIMPLView_UI when the preflight is finished
  disconnect(m_SIMPLController, &SIMPLController::preflightFinished, this, &SIMPLView_UI::preflightDidFinish);

  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();

    disconnect(viewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)), this, SLOT(setFilterInputWidget(FilterInputWidget*)));

    disconnect(viewWidget, SIGNAL(filterInputWidgetNeedsCleared()), this, SLOT(clearFilterInputWidget()));

    disconnect(viewWidget, SIGNAL(filterInputWidgetEdited()), this, SLOT(markDocumentAsDirty()));

    disconnect(viewWidget, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));

    disconnect(viewWidget, SIGNAL(preflightFinished(int)), this, SLOT(preflightDidFinish(int)));

    disconnect(viewWidget, &SVPipelineViewWidget::undoCommandCreated, m_SIMPLController, &SIMPLController::addUndoCommand);

    disconnect(viewWidget, &SVPipelineViewWidget::undoRequested, m_SIMPLController, &SIMPLController::undo);

    disconnect(viewWidget, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

    disconnect(viewWidget, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

    disconnect(viewWidget, SIGNAL(deleteKeyPressed()), this, SLOT(listenDeleteKeyTriggered()));

    disconnect(getBookmarksToolboxWidget(), SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));
  }
  else
  {
    // Connection to preflight both pipelines when moving filters between pipelines
    disconnect(model, &PipelineModel::rowsMoved, 0, 0);

    // Connection that marks the document dirty whenever a filter is enabled/disabled
    disconnect(m_PipelineTreeView, &PipelineTreeView::filterEnabledStateChanged, this, &SIMPLView_UI::markDocumentAsDirty);

    // Connection that allows the view to call for a preflight, which gets picked up by the Pipeline Tree controller
    disconnect(m_PipelineTreeView, &PipelineTreeView::needsPreflight, m_SIMPLController, &SIMPLController::preflightPipeline);

    // Connection to update the active pipeline when the user decides to change it
    disconnect(m_PipelineTreeView, &PipelineTreeView::activePipelineChanged, m_SIMPLController, &SIMPLController::updateActivePipeline);

    disconnect(m_PipelineTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, 0, 0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::connectSignalsSlots()
{
  DocRequestManager* docRequester = DocRequestManager::Instance();
  PipelineModel* model = getPipelineModel();

  connect(docRequester, SIGNAL(showFilterDocs(const QString&)), this, SLOT(showFilterHelp(const QString&)));

  connect(docRequester, SIGNAL(showFilterDocUrl(const QUrl&)), this, SLOT(showFilterHelpUrl(const QUrl&)));

  connect(this, SIGNAL(bookmarkNeedsToBeAdded(const QString&, const QModelIndex&)), getBookmarksToolboxWidget(), SLOT(addBookmark(const QString&, const QModelIndex&)));

  connect(m_SIMPLController, &SIMPLController::statusMessageGenerated, [=] (const QString &msg) { statusBar()->showMessage(msg); });
  connect(m_SIMPLController, &SIMPLController::standardOutputMessageGenerated, [=] (const QString &msg) { addStdOutputMessage(msg); });

  connect(m_SIMPLController, &SIMPLController::undoActionGenerated, [=] (QAction* actionUndo) {
    PipelineModel* model = getPipelineModel();
    model->setActionUndo(actionUndo);
  });

  connect(m_SIMPLController, &SIMPLController::redoActionGenerated, [=] (QAction* actionRedo) {
    PipelineModel* model = getPipelineModel();
    model->setActionRedo(actionRedo);
  });

  // Connection that allows the Pipeline Tree controller to display cached issues in the table
  connect(m_SIMPLController, &SIMPLController::displayIssuesTriggered, issuesWidget, &IssuesWidget::displayCachedMessages);

  connect(m_SIMPLController, &SIMPLController::writeSIMPLViewSettingsTriggered, [=] { writeSettings(); });

  // Connection that displays issues in the Issue Table when the preflight is finished
  connect(m_SIMPLController, &SIMPLController::preflightFinished, [=] { issuesWidget->displayCachedMessages(); });

  // Connection that refreshes the Data Browser when the preflight is finished
  connect(m_SIMPLController, &SIMPLController::preflightFinished, [=] { dataBrowserWidget->refreshData(); });

  // Connection that does post-preflight updates on this instance of SIMPLView_UI when the preflight is finished
  connect(m_SIMPLController, &SIMPLController::preflightFinished, this, &SIMPLView_UI::preflightDidFinish);

  // Connection that passes pipeline messages from the controller to this instance of SIMPLView_UI during pipeline execution
  connect(m_SIMPLController, &SIMPLController::pipelineMessageGenerated, this, &SIMPLView_UI::processPipelineMessage);

  connect(m_SIMPLController, &SIMPLController::pipelineFinished, this, &SIMPLView_UI::pipelineDidFinish);

  connect(filterLibraryWidget, SIGNAL(filterItemDoubleClicked(const QString&)), this, SLOT(addFilter(const QString&)));

  connect(filterListWidget, SIGNAL(filterItemDoubleClicked(const QString&)), this, SLOT(addFilter(const QString&)));

  connect(bookmarksWidget->getBookmarksTreeView(), SIGNAL(currentIndexChanged(const QModelIndex&, const QModelIndex&)), this,
          SLOT(listenBookmarkSelectionChanged(const QModelIndex&, const QModelIndex&)));

  connect(bookmarksWidget->getBookmarksTreeView(), &BookmarksTreeView::contextMenuRequested, this,
          &SIMPLView_UI::listenBookmarksContextMenuRequested);

  connect(bookmarksWidget, &BookmarksToolboxWidget::pipelineFileActivated, [=] (const QString& filePath) {
    if (USE_PIPELINE_TREE_WIDGET == true)
    {
      openPipeline(filePath);
    }
    else
    {
      dream3dApp->newInstanceFromFile(filePath);
    }
  });

  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  connect(menuItems->getActionLocateFile(), SIGNAL(triggered()), bookmarksWidget->getBookmarksTreeView(), SLOT(on_actionLocateFile_triggered()));

  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();

    connect(m_ListWidget, &PipelineListWidget::pipelineStarted, m_SIMPLController, &SIMPLController::runPipeline);

    connect(m_ListWidget, &PipelineListWidget::pipelineCanceled, m_SIMPLController, &SIMPLController::cancelPipeline);

    connect(viewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)), this, SLOT(setFilterInputWidget(FilterInputWidget*)));

    connect(viewWidget, SIGNAL(filterInputWidgetNeedsCleared()), this, SLOT(clearFilterInputWidget()));

    connect(viewWidget, SIGNAL(filterInputWidgetEdited()), this, SLOT(markDocumentAsDirty()));

    connect(viewWidget, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));

    connect(viewWidget, SIGNAL(preflightFinished(int)), this, SLOT(preflightDidFinish(int)));

    connect(this, &SIMPLView_UI::preflightFinished, m_ListWidget, &PipelineListWidget::preflightFinished);

    connect(viewWidget, &SVPipelineViewWidget::undoCommandCreated, m_SIMPLController, &SIMPLController::addUndoCommand);

    connect(viewWidget, &SVPipelineViewWidget::undoRequested, m_SIMPLController, &SIMPLController::undo);

    connect(viewWidget, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

    connect(viewWidget, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

    connect(viewWidget, SIGNAL(deleteKeyPressed()), this, SLOT(listenDeleteKeyTriggered()));

    connect(viewWidget, &SVPipelineViewWidget::pipelineDropped, m_SIMPLController, &SIMPLController::addPipelineToModelFromFile);

    connect(getBookmarksToolboxWidget(), SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));

    connect(m_SIMPLController, &SIMPLController::filtersAddedToModel, viewWidget, &SVPipelineViewWidget::addFiltersFromIndices);

    connect(m_SIMPLController, &SIMPLController::pipelineEnteringReadyState, viewWidget, &SVPipelineViewWidget::toReadyState);

    connect(m_SIMPLController, &SIMPLController::pipelineEnteringRunningState, viewWidget, &SVPipelineViewWidget::toRunningState);

    connect(m_SIMPLController, &SIMPLController::pipelineEnteringStoppedState, viewWidget, &SVPipelineViewWidget::toStoppedState);

    connect(m_SIMPLController, &SIMPLController::pipelineFinished, m_ListWidget, &PipelineListWidget::pipelineFinished);
  }
  else
  {
    // Connection to preflight both pipelines when moving filters between pipelines
    connect(model, &PipelineModel::rowsMoved, [=] (const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row) {
      if (model->itemType(parent) == PipelineItem::ItemType::Pipeline && model->itemType(destination) == PipelineItem::ItemType::Pipeline)
      {
        m_SIMPLController->preflightPipeline(parent, model);

        if (destination != parent)
        {
          m_SIMPLController->preflightPipeline(destination, model);
        }
      }
    });

    // Connection that marks the document dirty whenever a filter is enabled/disabled
    connect(m_PipelineTreeView, &PipelineTreeView::filterEnabledStateChanged, this, &SIMPLView_UI::markDocumentAsDirty);

    // Connection that allows the view to call for a preflight, which gets picked up by the Pipeline Tree controller
    connect(m_PipelineTreeView, &PipelineTreeView::needsPreflight, m_SIMPLController, &SIMPLController::preflightPipeline);

    // Connection that allows the Pipeline Tree view to clear the Issues Table
    connect(m_PipelineTreeView, &PipelineTreeView::clearIssuesTriggered, issuesWidget, &IssuesWidget::clearIssues);

    // Connection that enables the pipeline tree view to create a new pipeline
    connect(m_PipelineTreeView, &PipelineTreeView::needsNewPipeline, this, &SIMPLView_UI::addPipeline);

    connect(m_PipelineTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=] {
      QModelIndexList indexList = m_PipelineTreeView->selectionModel()->selectedRows();
      if (indexList.size() == 1)
      {
        QModelIndex index = indexList[0];
        FilterInputWidget* fiw = model->filterInputWidget(index);
        if (fiw != nullptr)
        {
          setFilterInputWidget(fiw);
          return;
        }
      }

      clearFilterInputWidget();
    });
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::addFilter(AbstractFilter::Pointer filter)
{
  setStatusBarMessage(tr("Added \"%1\" filter").arg(filter->getHumanLabel()));
  addStdOutputMessage(tr("Added \"%1\" filter").arg(filter->getHumanLabel()));

  PipelineModel* model = getPipelineModel();
  m_SIMPLController->addFilterToModel(filter, model);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::addPipeline(const QString &pipelineName, bool setAsActive)
{
  PipelineModel* model = getPipelineModel();
  m_SIMPLController->addPipelineToModel(pipelineName, FilterPipeline::New(), model, setAsActive);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SIMPLView_UI::openPipeline(const QString& filePath)
{
  int result = m_SIMPLController->addPipelineToModelFromFile(filePath, getPipelineModel());
  if (result < 0)
  {
    return result;
  }

  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    m_OpenedFilePath = filePath;
    setWindowFilePath(filePath);

    QFileInfo fi(filePath);
    setWindowTitle(QString("[*]") + fi.baseName() + " - " + BrandedStrings::ApplicationName);
    setWindowModified(false);
  }

  return result;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::executePipeline(const QModelIndex &pipelineIndex)
{
  m_SIMPLController->runPipeline(pipelineIndex, getPipelineModel());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setLoadedPlugins(QVector<ISIMPLibPlugin*> plugins)
{
  m_LoadedPlugins = plugins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenNewInstanceTriggered()
{
  SIMPLView_UI* newInstance = dream3dApp->getNewSIMPLViewInstance();
  newInstance->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenDeleteKeyTriggered()
{
  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();

    QList<PipelineFilterObject*> selectedWidgets = viewWidget->getSelectedFilterObjects();
    if(selectedWidgets.size() <= 0)
    {
      return;
    }

    // SIMPL-FIXME: Implement "remove" functionality in the controller
    //    RemoveFilterCommand* removeCmd = new RemoveFilterCommand(selectedWidgets, m_ActiveWindow->getPipelineViewWidget(), "Remove");
    //    m_ActiveWindow->getPipelineViewWidget()->addUndoCommand(removeCmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenOpenPipelineTriggered()
{
  QString proposedDir = m_OpenDialogLastFilePath;
  QString filePath = QFileDialog::getOpenFileName(nullptr, tr("Open Pipeline"), proposedDir, tr("Json File (*.json);;DREAM3D File (*.dream3d);;All Files (*.*)"));
  if(filePath.isEmpty())
  {
    return;
  }

  dream3dApp->newInstanceFromFile(filePath);

  // Cache the last directory on old instance
  m_OpenDialogLastFilePath = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenSavePipelineTriggered()
{
  savePipeline();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenSavePipelineAsTriggered()
{
  savePipelineAs();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenExecuteBookmarkTriggered()
{
  BookmarksModel* model = BookmarksModel::Instance();

  QModelIndexList indexList = bookmarksWidget->getBookmarksTreeView()->selectionModel()->selectedRows(BookmarksItem::Name);

  QModelIndex pathIndex = indexList.at(0);
  QModelIndex actualIndex = model->index(pathIndex.row(), BookmarksItem::Path, pathIndex.parent());

  QString pipelinePath = actualIndex.data().toString();
  if(pipelinePath.isEmpty())
  {
    return; // The user double clicked a folder, so don't do anything
  }
  else if (USE_PIPELINE_TREE_WIDGET == true)
  {
    openPipeline(pipelinePath);

    PipelineModel* model = getPipelineModel();
    int row = model->rowCount() - 1;
    QModelIndex pipelineIndex = model->index(row, PipelineItem::Name);

    executePipeline(pipelineIndex);
  }
  else
  {
    QFileInfo fi(pipelinePath);
    if(fi.exists())
    {
      SIMPLView_UI* ui = dream3dApp->newInstanceFromFile(pipelinePath);
      PipelineModel* model = getPipelineModel();
      QModelIndex pipelineIndex = model->index(0, PipelineItem::Name);

      // Cache the last directory on old instance
      m_OpenDialogLastFilePath = pipelinePath;

      ui->executePipeline(pipelineIndex);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenAddBookmarkTriggered()
{
  QString proposedDir = m_OpenDialogLastFilePath;
  QList<QString> newPrefPaths;

  newPrefPaths =
      QFileDialog::getOpenFileNames(this, tr("Choose Pipeline File(s)"), proposedDir, tr("Json File (*.json);;DREAM3D File (*.dream3d);;Text File (*.txt);;Ini File (*.ini);;All Files (*.*)"));
  if(true == newPrefPaths.isEmpty())
  {
    return;
  }

  QModelIndex parent = bookmarksWidget->getBookmarksTreeView()->currentIndex();

  if(parent.isValid() == false)
  {
    parent = QModelIndex();
  }

  for(int i = 0; i < newPrefPaths.size(); i++)
  {
    QString newPrefPath = newPrefPaths[i];
    newPrefPath = QDir::toNativeSeparators(newPrefPath);
    bookmarksWidget->addBookmark(newPrefPath, parent);
  }

  if(newPrefPaths.size() > 0)
  {
    // Cache the directory from the last path added
    m_OpenDialogLastFilePath = newPrefPaths[newPrefPaths.size() - 1];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenAddBookmarkFolderTriggered()
{
  QModelIndex parent = bookmarksWidget->getBookmarksTreeView()->currentIndex();

  if(parent.isValid() == false)
  {
    parent = QModelIndex();
  }

  QString name = "New Folder";

  BookmarksModel* model = BookmarksModel::Instance();

  bookmarksWidget->addTreeItem(parent, name, QIcon(":/folder_blue.png"), "", model->rowCount(parent), true, true, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenRenameBookmarkTriggered()
{
  BookmarksTreeView* bookmarksTreeView = bookmarksWidget->getBookmarksTreeView();
  QModelIndex index = bookmarksTreeView->currentIndex();
  bookmarksTreeView->edit(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenRemoveBookmarkTriggered()
{
  BookmarksModel* model = BookmarksModel::Instance();

  QModelIndexList indexList = bookmarksWidget->getBookmarksTreeView()->selectionModel()->selectedRows(BookmarksItem::Name);

  indexList = bookmarksWidget->getBookmarksTreeView()->filterOutDescendants(indexList);

  if(indexList.size() <= 0)
  {
    return;
  }

  QList<QPersistentModelIndex> persistentList;
  for(int i = 0; i < indexList.size(); i++)
  {
    persistentList.push_back(indexList[i]);
  }

  QModelIndex singleIndex = model->index(indexList[0].row(), BookmarksItem::Name, indexList[0].parent());

  QMessageBox msgBox;
  if(indexList.size() > 1)
  {
    msgBox.setWindowTitle("Remove Bookmark Items");
    msgBox.setText("Are you sure that you want to remove these bookmark items? The original bookmark files will not be removed.");
  }
  else if(model->flags(singleIndex).testFlag(Qt::ItemIsDropEnabled) == false)
  {
    msgBox.setWindowTitle("Remove Bookmark");
    msgBox.setText("Are you sure that you want to remove the bookmark \"" + singleIndex.data().toString() + "\"? The original file will not be removed.");
  }
  else
  {
    msgBox.setWindowTitle("Remove Folder");
    msgBox.setText("Are you sure that you want to remove the folder \"" + singleIndex.data().toString() + "\"? The folder's contents will also be removed.");
  }
  msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int ret = msgBox.exec();

  if(ret == QMessageBox::Yes)
  {
    for(int i = 0; i < persistentList.size(); i++)
    {
      QModelIndex nameIndex = model->index(persistentList[i].row(), BookmarksItem::Name, persistentList[i].parent());
      QString name = nameIndex.data().toString();

      // Remove bookmark from the SIMPLView interface
      model->removeRow(persistentList[i].row(), persistentList[i].parent());
    }

    // Write these changes out to the preferences file
    emit bookmarksWidget->fireWriteSettings();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenShowBookmarkInFileSystemTriggered()
{
  BookmarksModel* model = BookmarksModel::Instance();
  BookmarksTreeView* bookmarksTreeView = bookmarksWidget->getBookmarksTreeView();

  QModelIndex index = bookmarksTreeView->currentIndex();
  if(index.isValid())
  {
    QString pipelinePath = model->index(index.row(), BookmarksItem::Path, index.parent()).data().toString();

    QFileInfo fi(pipelinePath);
    QString path;
    if(fi.isFile())
    {
      path = fi.absoluteFilePath();
    }
    else
    {
      path = fi.absolutePath();
    }

    QtSFileUtils::ShowPathInGui(bookmarksWidget, path);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenClearCacheTriggered()
{
  QMessageBox msgBox;

  QString title = QString("Clear %1 Cache").arg(BrandedStrings::ApplicationName);
  msgBox.setWindowTitle(title);

  QString text = QString("Clearing the %1 cache will clear the %1 window settings, and will restore %1 back to its default settings on the program's next run.").arg(BrandedStrings::ApplicationName);
  msgBox.setText(text);

  QString infoText = QString("Clear the %1 cache?").arg(BrandedStrings::ApplicationName);
  msgBox.setInformativeText(infoText);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int response = msgBox.exec();

  if(response == QMessageBox::Yes)
  {
    QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

    // Set a flag in the preferences file, so that we know that we are in "Clear Cache" mode
    prefs->setValue("Program Mode", QString("Clear Cache"));

    setStatusBarMessage(QString("The cache has been cleared successfully. Please restart %1.").arg(BrandedStrings::ApplicationName));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenClearBookmarksTriggered()
{
  QMessageBox msgBox;
  QString title = QString("Clear %1 Bookmarks").arg(BrandedStrings::ApplicationName);
  msgBox.setWindowTitle(title);
  msgBox.setText(QString("Are you sure that you want to clear all %1 bookmarks?").arg(BrandedStrings::ApplicationName));

  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int response = msgBox.exec();

  if(response == QMessageBox::Yes)
  {
    BookmarksModel* model = BookmarksModel::Instance();
    if(model->isEmpty() == false)
    {
      model->removeRows(0, model->rowCount(QModelIndex()));
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenCloseWindowTriggered()
{
  close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenShowSIMPLViewHelpTriggered()
{
  QString appPath = QApplication::applicationDirPath();

  QDir helpDir = QDir(appPath);
  QString s("file://");

#if defined(Q_OS_WIN)
  s = s + "/"; // Need the third slash on windows because file paths start with a drive letter
#elif defined(Q_OS_MAC)
  if(helpDir.dirName() == "MacOS")
  {
    helpDir.cdUp();
    // Check if we are running from a .app installation where the Help dir is embeded in the app bundle.
    QFileInfo fi(helpDir.absolutePath() + "/Resources/Help");

    if(fi.exists())
    {
      helpDir.cd("Resources");
    }
    else
    {
      helpDir.cdUp();
      helpDir.cdUp();
    }
  }
#endif

#ifdef SIMPL_USE_MKDOCS
  s = QString("http://%1:%2/index.html").arg(QtSDocServer::GetIPAddress()).arg(QtSDocServer::GetPort());
#endif

#ifdef SIMPL_USE_DISCOUNT
  QString helpFilePath = QString("%1/Help/%2/index.html").arg(helpDir.absolutePath()).arg(QCoreApplication::instance()->applicationName());
  QFileInfo fi(helpFilePath);
  if(fi.exists() == false)
  {
    // The help file does not exist at the default location because we are probably running from Visual Studio or Xcode
    // Try up one more directory
    helpDir.cdUp();
    helpFilePath = QString("%1/Help/%2/index.html").arg(helpDir.absolutePath()).arg(QCoreApplication::instance()->applicationName());
  }

  s = s + helpFilePath;
#endif

  QUrl helpURL(s);
#ifdef SIMPL_USE_QtWebEngine
  SVUserManualDialog::LaunchHelpDialog(helpURL);
#else
  bool didOpen = QDesktopServices::openUrl(helpURL);
  if(false == didOpen)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("Error Opening Help File"));
    msgBox.setInformativeText(QString::fromLatin1("%1 could not open the help file path ").arg(BrandedStrings::ApplicationName) + helpURL.path());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenCheckForUpdatesTriggered()
{

  UpdateCheck::SIMPLVersionData_t data;
  Detail::fillVersionData(data);
  UpdateCheckDialog d(data, nullptr);

  // d.setCurrentVersion(SIMPLib::Version::Complete());
  d.setUpdateWebSite(SIMPLView::UpdateWebsite::UpdateWebSite);
  d.setApplicationName(BrandedStrings::ApplicationName);

  // Read from the QtSSettings Pref file the information that we need
  QtSSettings prefs;
  prefs.beginGroup(SIMPLView::UpdateWebsite::VersionCheckGroupName);
  QDateTime dateTime = prefs.value(SIMPLView::UpdateWebsite::LastVersionCheck, QDateTime::currentDateTime()).toDateTime();
  d.setLastCheckDateTime(dateTime);
  prefs.endGroup();

  // Now display the dialog box
  d.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenPluginInfoTriggered()
{
  AboutPlugins dialog(nullptr);
  dialog.exec();

  // Write cache on exit
  dialog.writePluginCache();

  /* If any of the load checkboxes were changed, display a dialog warning
  * the user that they must restart SIMPLView to see the changes.
  */
  if(dialog.getLoadPreferencesDidChange() == true)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("%1 must be restarted to allow these changes to take effect.").arg(BrandedStrings::ApplicationName));
    msgBox.setInformativeText("Restart?");
    msgBox.setWindowTitle("Restart Needed");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int choice = msgBox.exec();

    if(choice == QMessageBox::Yes)
    {
      QProcess::startDetached(QApplication::applicationFilePath());

      emit applicationExitTriggered();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenAboutSIMPLViewTriggered()
{
  AboutSIMPLView d(nullptr);
  d.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenClearRecentFilesTriggered()
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
void SIMPLView_UI::listenBookmarkSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
  BookmarksModel* model = BookmarksModel::Instance();
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();

  if(model->index(current.row(), BookmarksItem::Path, current.parent()).data().toString().isEmpty() == true)
  {
    actionAddBookmark->setEnabled(true);
    actionNewFolder->setEnabled(true);
  }
  else
  {
    actionAddBookmark->setDisabled(true);
    actionNewFolder->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::listenOpenBookmarkTriggered()
{
  BookmarksModel* model = BookmarksModel::Instance();

  QModelIndexList indexList = bookmarksWidget->getBookmarksTreeView()->selectionModel()->selectedRows(BookmarksItem::Name);

  QModelIndex pathIndex = indexList.at(0);
  QModelIndex actualIndex = model->index(pathIndex.row(), BookmarksItem::Path, pathIndex.parent());

  QString pipelinePath = actualIndex.data().toString();
  if(pipelinePath.isEmpty())
  {
    return; // The user double clicked a folder, so don't do anything
  }
  else
  {
    QFileInfo fi(pipelinePath);
    if(fi.exists())
    {
      dream3dApp->newInstanceFromFile(pipelinePath);
      // Cache the last directory on old instance
      m_OpenDialogLastFilePath = pipelinePath;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::on_pipelineViewWidget_windowNeedsRecheck()
{
  QString fiBase = "Untitled";
  if(false == m_OpenedFilePath.isEmpty())
  {
    fiBase = m_OpenedFilePath;
  }

  QFileInfo fi(fiBase);
  if(!windowFilePath().isEmpty())
  {
    fi = QFileInfo(windowFilePath());
  }

  setWindowTitle(QString("[*]") + fi.baseName() + " - " + BrandedStrings::ApplicationName);
  setWindowModified(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::on_pipelineViewWidget_pipelineIssuesCleared()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::on_pipelineViewWidget_pipelineHasNoErrors()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::toPipelineRunningState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  menuItems->getActionClearPipeline()->setDisabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::toPipelineIdleState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  menuItems->getActionClearPipeline()->setEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::dragEnterEvent(QDragEnterEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastFilePath = parent.dirName();
  QFileInfo fi(file);
  QString ext = fi.suffix();
  if(fi.exists() && fi.isFile() && (ext.compare("mxa") || ext.compare("h5") || ext.compare("hdf5")))
  {
    e->accept();
  }
  else
  {
    e->ignore();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::dropEvent(QDropEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastFilePath = parent.dirName();
  QFileInfo fi(file);
  QString ext = fi.suffix();
  file = QDir::toNativeSeparators(file);
  if(fi.exists() && fi.isFile())
  {
    // TODO: INSERT Drop Event CODE HERE
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMessageBox::StandardButton SIMPLView_UI::checkDirtyDocument()
{

  if(this->isWindowModified() == true)
  {
    int r = QMessageBox::warning(this, BrandedStrings::ApplicationName, tr("The Pipeline has been modified.\nDo you want to save your changes?"), QMessageBox::Save | QMessageBox::Default,
                                 QMessageBox::Discard, QMessageBox::Cancel | QMessageBox::Escape);
    if(r == QMessageBox::Save)
    {
      if(savePipeline() == true)
      {
        return QMessageBox::Save;
      }
      else
      {
        return QMessageBox::Cancel;
      }
    }
    else if(r == QMessageBox::Discard)
    {
      return QMessageBox::Discard;
    }
    else if(r == QMessageBox::Cancel)
    {
      return QMessageBox::Cancel;
    }
  }

  return QMessageBox::Ignore;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::populateMenus(QObject* plugin)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::processPipelineMessage(const PipelineMessage& msg)
{
  if(msg.getType() == PipelineMessage::MessageType::ProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    m_ListWidget->setProgressValue(progValue);
  }
  else if(msg.getType() == PipelineMessage::MessageType::StatusMessageAndProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    m_ListWidget->setProgressValue(progValue);

    if(nullptr != this->statusBar())
    {
      this->statusBar()->showMessage(msg.generateStatusString());
    }
  }
  else if(msg.getType() == PipelineMessage::MessageType::StandardOutputMessage || msg.getType() == PipelineMessage::MessageType::StatusMessage)
  {
    if(msg.getType() == PipelineMessage::MessageType::StatusMessage)
    {
      if(nullptr != this->statusBar())
      {
        this->statusBar()->showMessage(msg.generateStatusString());
      }
    }

    // Allow status messages to open the standard output widget
    if(HideDockSetting::OnStatusAndError == m_HideStdOutput)
    {
      stdOutDockWidget->setVisible(true);
    }

    // Allow status messages to open the issuesDockWidget as well
    if(HideDockSetting::OnStatusAndError == m_HideErrorTable)
    {
      issuesDockWidget->setVisible(true);
    }

    QString text = "<span style=\" color:#000000;\" >";
    text.append(msg.getText());
    text.append("</span>");
    stdOutWidget->appendText(text);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::pipelineDidFinish()
{
  // Re-enable FilterListToolboxWidget signals - resume adding filters
  getFilterListToolboxWidget()->blockSignals(false);

  // Re-enable FilterLibraryToolboxWidget signals - resume adding filters
  getFilterLibraryToolboxWidget()->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::versionCheckReply(UpdateCheckData* dataObj)
{
  UpdateCheck::SIMPLVersionData_t data = dream3dApp->FillVersionData();

  UpdateCheckDialog d(data, this);
  // d->setCurrentVersion(data.complete);
  d.setApplicationName(BrandedStrings::ApplicationName);

  if(dataObj->hasUpdate() && !dataObj->hasError())
  {
    QString message = dataObj->getMessageDescription();
    QLabel* feedbackTextLabel = d.getFeedbackTextLabel();
    d.toSimpleUpdateCheckDialog();
    feedbackTextLabel->setText(message);
    d.getCurrentVersionLabel()->setText(dataObj->getAppString());
    // d->setCurrentVersion( dataObj->getAppString() );
    d.getLatestVersionLabel()->setText(dataObj->getServerString());
    d.exec();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::showFilterHelp(const QString& className)
{
// Launch the dialog
#ifdef SIMPL_USE_QtWebEngine
  SVUserManualDialog::LaunchHelpDialog(className);
#else
  QUrl helpURL = URL_GENERATOR::GenerateHTMLUrl(className);
  bool didOpen = QDesktopServices::openUrl(helpURL);
  if(false == didOpen)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("Error Opening Help File"));
    msgBox.setInformativeText(QString::fromLatin1("SIMPLView could not open the help file path ") + helpURL.path());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::showFilterHelpUrl(const QUrl& helpURL)
{
#ifdef SIMPL_USE_QtWebEngine
  SVUserManualDialog::LaunchHelpDialog(helpURL);
#else
  bool didOpen = QDesktopServices::openUrl(helpURL);
  if(false == didOpen)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("Error Opening Help File"));
    msgBox.setInformativeText(QString::fromLatin1("SIMPLView could not open the help file path ") + helpURL.path());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::cleanupPipeline()
{
  // Clear the filter input widget
  clearFilterInputWidget();

  SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();
  viewWidget->clearFilterWidgets();
  setWindowModified(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::insertDockWidgetActions(QMenu* menu)
{
#if 0
  menu->addAction(issuesDockWidget->toggleViewAction());
  menu->addAction(stdOutDockWidget->toggleViewAction());
  menu->addAction(dataBrowserDockWidget->toggleViewAction());
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QAction*> SIMPLView_UI::getDummyDockWidgetActions()
{
  QList<QAction*> actions;

  QAction* issuesDummyAction = new QAction(issuesDockWidget->toggleViewAction()->text(), macApp);
  issuesDummyAction->setDisabled(true);
  actions.push_back(issuesDummyAction);

  QAction* stdOutDummyAction = new QAction(stdOutDockWidget->toggleViewAction()->text(), macApp);
  stdOutDummyAction->setDisabled(true);
  actions.push_back(stdOutDummyAction);

  QAction* dataBrowserDummyAction = new QAction(dataBrowserDockWidget->toggleViewAction()->text(), macApp);
  dataBrowserDummyAction->setDisabled(true);
  actions.push_back(dataBrowserDummyAction);

  return actions;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::removeDockWidgetActions(QMenu* menu)
{
#if 0
  menu->removeAction(issuesDockWidget->toggleViewAction());
  menu->removeAction(stdOutDockWidget->toggleViewAction());
  menu->removeAction(dataBrowserDockWidget->toggleViewAction());
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
BookmarksToolboxWidget* SIMPLView_UI::getBookmarksToolboxWidget()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  return toolbox->getBookmarksWidget();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterListToolboxWidget* SIMPLView_UI::getFilterListToolboxWidget()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  return toolbox->getFilterListWidget();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterLibraryToolboxWidget* SIMPLView_UI::getFilterLibraryToolboxWidget()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  return toolbox->getFilterLibraryWidget();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataStructureWidget* SIMPLView_UI::getDataStructureWidget()
{
  return dataBrowserWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setOpenedFilePath(const QString& filePath)
{
  m_OpenedFilePath = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setOpenDialogLastFilePath(const QString& path)
{
  m_OpenDialogLastFilePath = path;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setFilterInputWidget(FilterInputWidget* widget)
{
  if(widget == nullptr)
  {
    return;
  }

  // Clear the filter input widget
  clearFilterInputWidget();

  // Set the widget into the frame
  fiwFrameVLayout->addWidget(widget);
  widget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::clearFilterInputWidget()
{
  QLayoutItem* item = fiwFrameVLayout->takeAt(0);
  if(item)
  {
    QWidget* w = item->widget();
    if(w)
    {
      w->hide();
      w->setParent(nullptr);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::markDocumentAsDirty()
{
  setWindowModified(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::issuesTableHasErrors(bool hasErrors, int errCount, int warnCount)
{
  Q_UNUSED(errCount)
  Q_UNUSED(warnCount)
  if(HideDockSetting::OnError == m_HideErrorTable || HideDockSetting::OnStatusAndError == m_HideErrorTable)
  {
    issuesDockWidget->setVisible(hasErrors);
  }

  if(HideDockSetting::OnError == m_HideStdOutput || HideDockSetting::OnStatusAndError == m_HideStdOutput)
  {
    stdOutDockWidget->setVisible(hasErrors);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setStatusBarMessage(const QString& msg)
{
  statusbar->showMessage(msg);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::addStdOutputMessage(const QString& msg)
{
  QString text = "<span style=\" color:#000000;\" >";
  text.append(msg);
  text.append("</span>");
  stdOutWidget->appendText(text);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::changeEvent(QEvent* event)
{
  if(event->type() == QEvent::ActivationChange)
  {
    emit dream3dWindowChangedState(this);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::preflightDidFinish(int err)
{
  PipelineModel* model = getPipelineModel();
  QModelIndex pipelineIndex = m_SIMPLController->getActivePipelineIndex();
  if (pipelineIndex.isValid() == false)
  {
    emit preflightFinished(true);
    return;
  }

  if(err < 0 || model->rowCount(pipelineIndex) <= 0)
  {
    emit preflightFinished(true);

  }
  else
  {
    emit preflightFinished(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* SIMPLView_UI::getPipelineModel()
{
  PipelineModel* model;
  if (USE_PIPELINE_TREE_WIDGET == true)
  {
    model = m_PipelineTreeView->getPipelineModel();
  }
  else
  {
    SVPipelineViewWidget* viewWidget = m_ListWidget->getPipelineViewWidget();
    model = viewWidget->getPipelineModel();
  }
  return model;
}
