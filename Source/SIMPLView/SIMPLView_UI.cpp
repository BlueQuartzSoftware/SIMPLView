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
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/Widgets/util/AddFilterCommand.h"
#include "SVWidgetsLib/Widgets/BookmarksModel.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/BookmarksTreeView.h"
#include "SVWidgetsLib/Widgets/FilterLibraryToolboxWidget.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineItemDelegate.h"
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI::SIMPLView_UI(QWidget* parent)
: QMainWindow(parent)
, m_Ui(new Ui::SIMPLView_UI)
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
  m_Ui->setupUi(this);

  dream3dApp->registerSIMPLViewWindow(this);

  // Do our own widget initializations
  setupGui();

  this->setAcceptDrops(true);

  // Read various settings
  readSettings();
  if(HideDockSetting::OnError == m_HideErrorTable)
  {
    m_Ui->issuesDockWidget->setHidden(true);
  }
  if(HideDockSetting::OnError == m_HideStdOutput)
  {
    m_Ui->stdOutDockWidget->setHidden(true);
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

  writeSettings();

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
void SIMPLView_UI::listenSavePipelineTriggered()
{
  savePipeline();
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
      return savePipelineAs();
    }
    else
    {
      filePath = m_OpenedFilePath;
    }

    // Fix the separators
    filePath = QDir::toNativeSeparators(filePath);

    // Write the pipeline
    SVPipelineView* viewWidget = m_Ui->pipelineListWidget->getPipelineView();
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
void SIMPLView_UI::listenSavePipelineAsTriggered()
{
  savePipelineAs();
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
  SVPipelineView* viewWidget = m_Ui->pipelineListWidget->getPipelineView();
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
    m_Ui->bookmarksWidget->getBookmarksTreeView()->addBookmark(filePath, QModelIndex());
  }

  return true;
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void SIMPLView_UI::closeEvent(QCloseEvent* event)
{
  if(m_Ui->pipelineListWidget->getPipelineView()->isPipelineCurrentlyRunning() == true)
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
  readDockWidgetSettings(prefs.data(), m_Ui->issuesDockWidget);
  readHideDockSettings(prefs.data(), m_HideErrorTable);
  prefs->endGroup();

  prefs->beginGroup("Standard Output Dock Widget");
  readDockWidgetSettings(prefs.data(), m_Ui->stdOutDockWidget);
  readHideDockSettings(prefs.data(), m_HideStdOutput);
  prefs->endGroup();

  prefs->endGroup();

  prefs->beginGroup("ToolboxSettings");

  // Read dock widget settings
  prefs->beginGroup("Bookmarks Widget");
  m_Ui->bookmarksWidget->readSettings(prefs.data());
  prefs->endGroup();

  prefs->beginGroup("Filter List Widget");
  m_Ui->filterListWidget->readSettings(prefs.data());
  prefs->endGroup();

  prefs->beginGroup("Filter Library Widget");
  m_Ui->filterLibraryWidget->readSettings(prefs.data());
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
  writeDockWidgetSettings(prefs.data(), m_Ui->issuesDockWidget);
  writeHideDockSettings(prefs.data(), m_HideErrorTable);
  prefs->endGroup();

  prefs->beginGroup("Standard Output Dock Widget");
  writeDockWidgetSettings(prefs.data(), m_Ui->stdOutDockWidget);
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
void SIMPLView_UI::setupGui()
{
  SVPipelineView* viewWidget = m_Ui->pipelineListWidget->getPipelineView();

  PipelineItemDelegate* delegate = new PipelineItemDelegate(viewWidget);
  viewWidget->setItemDelegate(delegate);

  // Create the model
  PipelineModel* model = new PipelineModel(this);
  model->setMaxNumberOfPipelines(1);

  viewWidget->setModel(model);

  // Set the IssuesWidget as a PipelineMessageObserver Object.
  viewWidget->addPipelineMessageObserver(m_Ui->issuesWidget);

  createSIMPLViewMenuSystem();

  // Hook up the signals from the various docks to the PipelineViewWidget that will either add a filter
  // or load an entire pipeline into the view
  connectSignalsSlots();

  // This will set the initial list of filters in the FilterListToolboxWidget
  // Tell the Filter Library that we have more Filters (potentially)
  getFilterLibraryToolboxWidget()->refreshFilterGroups();

  // Read the toolbox settings and update the filter list
  m_Ui->filterListWidget->updateFilterList(true);

  tabifyDockWidget(m_Ui->filterListDockWidget, m_Ui->filterLibraryDockWidget);
  tabifyDockWidget(m_Ui->filterLibraryDockWidget, m_Ui->bookmarksDockWidget);

  m_Ui->filterListDockWidget->raise();

  m_Ui->issuesDockWidget->toggleViewAction()->setText("Show " + m_Ui->issuesDockWidget->toggleViewAction()->text());
  m_Ui->stdOutDockWidget->toggleViewAction()->setText("Show " + m_Ui->stdOutDockWidget->toggleViewAction()->text());
  m_Ui->dataBrowserDockWidget->toggleViewAction()->setText("Show " + m_Ui->dataBrowserDockWidget->toggleViewAction()->text());
  m_Ui->pipelineDockWidget->toggleViewAction()->setText("Show " + m_Ui->pipelineDockWidget->toggleViewAction()->text());

  // Shortcut to close the window
  new QShortcut(QKeySequence(QKeySequence::Close), this, SLOT(close()));

  m_StatusBar = new StatusBarWidget();
  this->statusBar()->insertPermanentWidget(0, m_StatusBar, 0);

  m_StatusBar->setButtonAction(m_Ui->issuesDockWidget, StatusBarWidget::Button::Issues);
  m_StatusBar->setButtonAction(m_Ui->stdOutDockWidget, StatusBarWidget::Button::Console);
  m_StatusBar->setButtonAction(m_Ui->dataBrowserDockWidget, StatusBarWidget::Button::DataStructure);
  m_StatusBar->setButtonAction(m_Ui->pipelineDockWidget, StatusBarWidget::Button::Pipeline);

  connect(m_Ui->issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), m_StatusBar, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(m_Ui->issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), this, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(m_Ui->issuesWidget, SIGNAL(showTable(bool)), m_Ui->issuesDockWidget, SLOT(setVisible(bool)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::createSIMPLViewMenuSystem()
{
  m_SIMPLViewMenu = new QMenuBar(this);

  m_MenuRecentFiles = new QMenu("Recent Files", this);
  m_MenuFile = new QMenu("File", this);
  m_MenuEdit = new QMenu("Edit", this);
  m_MenuView = new QMenu("View", this);
  m_MenuBookmarks = new QMenu("Bookmarks", this);
  m_MenuPipeline = new QMenu("Pipeline", this);
  m_MenuHelp = new QMenu("Help", this);
  m_MenuAdvanced = new QMenu("Advanced", this);

  m_ActionNew = new QAction("New...", this);
  m_ActionOpen = new QAction("Open...", this);
  m_ActionSave = new QAction("Save", this);
  m_ActionSaveAs = new QAction("Save As...", this);
  m_ActionLoadTheme = new QAction("Load Theme", this);
  m_ActionSaveTheme = new QAction("Save Theme", this);
  m_ActionClearRecentFiles = new QAction("Clear Recent Files", this);
  m_ActionExit = new QAction("Exit " + QApplication::applicationName(), this);
  m_ActionShowSIMPLViewHelp = new QAction(QApplication::applicationName() + " Help", this);
  m_ActionAboutSIMPLView = new QAction("About " + QApplication::applicationName(), this);
  m_ActionCheckForUpdates = new QAction("Check For Updates", this);
  m_ActionPluginInformation = new QAction("Plugin Information", this);
  m_ActionClearCache = new QAction("Clear Cache", this);

  // SIMPLView_UI Actions
  connect(m_ActionNew, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenNewInstanceTriggered);
  connect(m_ActionOpen, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenOpenPipelineTriggered);
  connect(m_ActionSave, &QAction::triggered, this, &SIMPLView_UI::listenSavePipelineTriggered);
  connect(m_ActionSaveAs, &QAction::triggered, this, &SIMPLView_UI::listenSavePipelineAsTriggered);
  connect(m_ActionExit, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenExitApplicationTriggered);
  connect(m_ActionClearRecentFiles, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenClearRecentFilesTriggered);
  connect(m_ActionAboutSIMPLView, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenDisplayAboutSIMPLViewDialogTriggered);
  connect(m_ActionCheckForUpdates, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenCheckForUpdatesTriggered);
  connect(m_ActionShowSIMPLViewHelp, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenShowSIMPLViewHelpTriggered);
  connect(m_ActionPluginInformation, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenDisplayPluginInfoDialogTriggered);
  connect(m_ActionClearCache, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenClearSIMPLViewCacheTriggered);

  m_ActionNew->setShortcut(QKeySequence::New);
  m_ActionOpen->setShortcut(QKeySequence::Open);
  m_ActionSave->setShortcut(QKeySequence::Save);
  m_ActionSaveAs->setShortcut(QKeySequence::SaveAs);
  m_ActionExit->setShortcut(QKeySequence::Quit);
  m_ActionCheckForUpdates->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
  m_ActionShowSIMPLViewHelp->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
  m_ActionPluginInformation->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

  // Pipeline View Actions
  SVPipelineView* viewWidget = m_Ui->pipelineListWidget->getPipelineView();
  QAction* actionCut = viewWidget->getActionCut();
  QAction* actionCopy = viewWidget->getActionCopy();
  QAction* actionPaste = viewWidget->getActionPaste();
  QAction* actionClearPipeline = viewWidget->getActionClearPipeline();
  QAction* actionUndo = viewWidget->getActionUndo();
  QAction* actionRedo = viewWidget->getActionRedo();

  // Bookmarks Actions
  BookmarksTreeView* bookmarksView = m_Ui->bookmarksWidget->getBookmarksTreeView();
  QAction* actionAddBookmark = bookmarksView->getActionAddBookmark();
  QAction* actionNewFolder = bookmarksView->getActionAddBookmarkFolder();
  QAction* actionClearBookmarks = bookmarksView->getActionClearBookmarks();

  // Create File Menu
  m_SIMPLViewMenu->addMenu(m_MenuFile);
  m_MenuFile->addAction(m_ActionNew);
  m_MenuFile->addAction(m_ActionOpen);
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_ActionSave);
  m_MenuFile->addAction(m_ActionSaveAs);
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_MenuRecentFiles->menuAction());
  m_MenuRecentFiles->addSeparator();
  m_MenuRecentFiles->addAction(m_ActionClearRecentFiles);
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_ActionExit);

  // Create Edit Menu
  m_SIMPLViewMenu->addMenu(m_MenuEdit);
  m_MenuEdit->addAction(actionUndo);
  m_MenuEdit->addAction(actionRedo);
  m_MenuEdit->addSeparator();
  m_MenuEdit->addAction(actionCut);
  m_MenuEdit->addAction(actionCopy);
  m_MenuEdit->addAction(actionPaste);

  // Create Bookmarks Menu
  m_SIMPLViewMenu->addMenu(m_MenuBookmarks);
  m_MenuBookmarks->addAction(actionAddBookmark);
  m_MenuBookmarks->addSeparator();
  m_MenuBookmarks->addAction(actionNewFolder);

  // Create Pipeline Menu
  m_SIMPLViewMenu->addMenu(m_MenuPipeline);
  m_MenuPipeline->addAction(actionClearPipeline);

  // Create Help Menu
  m_SIMPLViewMenu->addMenu(m_MenuHelp);
  m_MenuHelp->addAction(m_ActionShowSIMPLViewHelp);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addAction(m_ActionCheckForUpdates);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addMenu(m_MenuAdvanced);
  m_MenuAdvanced->addAction(m_ActionClearCache);
  m_MenuAdvanced->addSeparator();
  m_MenuAdvanced->addAction(actionClearBookmarks);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addAction(m_ActionAboutSIMPLView);
  m_MenuHelp->addAction(m_ActionPluginInformation);

  setMenuBar(m_SIMPLViewMenu);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::connectSignalsSlots()
{
  SVPipelineView* pipelineView = m_Ui->pipelineListWidget->getPipelineView();
  PipelineModel* pipelineModel = pipelineView->getPipelineModel();

  /* Documentation Requester connections */
  DocRequestManager* docRequester = DocRequestManager::Instance();

  connect(docRequester, SIGNAL(showFilterDocs(const QString&)), this, SLOT(showFilterHelp(const QString&)));

  connect(docRequester, SIGNAL(showFilterDocUrl(const QUrl&)), this, SLOT(showFilterHelpUrl(const QUrl&)));

  /* Filter Library Widget Connections */
  connect(m_Ui->filterLibraryWidget, &FilterLibraryToolboxWidget::filterItemDoubleClicked, pipelineView, &SVPipelineView::addFilterFromClassName);

  /* Filter List Widget Connections */
  connect(m_Ui->filterListWidget, &FilterListToolboxWidget::filterItemDoubleClicked, pipelineView, &SVPipelineView::addFilterFromClassName);

  /* Bookmarks Widget Connections */
  connect(m_Ui->bookmarksWidget, &BookmarksToolboxWidget::newSIMPLViewInstanceTriggered, [=] (const QString& filePath, bool execute) {
    SIMPLView_UI* newInstance = dream3dApp->newInstanceFromFile(filePath);
    if (execute)
    {
      newInstance->executePipeline();
    }
  });

  connect(m_Ui->bookmarksWidget, SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));

  /* Pipeline List Widget Connections */
  connect(m_Ui->pipelineListWidget, &PipelineListWidget::pipelineCanceled, pipelineView, &SVPipelineView::cancelPipeline);

  /* Pipeline View Connections */
  connect(pipelineView->selectionModel(), &QItemSelectionModel::selectionChanged, [=] {

    QModelIndexList selectedIndexes = pipelineView->selectionModel()->selectedRows();
    if (selectedIndexes.size() == 1)
    {
      QModelIndex selectedIndex = selectedIndexes[0];

      PipelineModel* model = getPipelineModel();
      FilterInputWidget* fiw = model->filterInputWidget(selectedIndex);
      setFilterInputWidget(fiw);

      AbstractFilter::Pointer filter = model->filter(selectedIndex);
      m_Ui->dataBrowserWidget->filterActivated(filter);
    }
    else
    {
      clearFilterInputWidget();
      m_Ui->dataBrowserWidget->filterActivated(AbstractFilter::NullPointer());
    }
  });

  connect(pipelineView, &SVPipelineView::clearDataStructureWidgetTriggered, [=] {
    m_Ui->dataBrowserWidget->filterActivated(AbstractFilter::NullPointer());
  });

  connect(pipelineView, &SVPipelineView::filterInputWidgetNeedsCleared, this, &SIMPLView_UI::clearFilterInputWidget);

  connect(pipelineView, &SVPipelineView::displayIssuesTriggered, m_Ui->issuesWidget, &IssuesWidget::displayCachedMessages);

  connect(pipelineView, &SVPipelineView::clearIssuesTriggered, m_Ui->issuesWidget, &IssuesWidget::clearIssues);

  connect(pipelineView, &SVPipelineView::writeSIMPLViewSettingsTriggered, [=] { writeSettings(); });

  // Connection that displays issues in the Issue Table when the preflight is finished
  connect(pipelineView, &SVPipelineView::preflightFinished, [=] (FilterPipeline::Pointer pipeline, int err) {
    m_Ui->dataBrowserWidget->refreshData();
    m_Ui->issuesWidget->displayCachedMessages();
    m_Ui->pipelineListWidget->preflightFinished(pipeline, err);
  });

  connect(pipelineView, &SVPipelineView::pipelineStarted, this, &SIMPLView_UI::pipelineDidFinish);

  connect(pipelineView, &SVPipelineView::pipelineHasMessage, this, &SIMPLView_UI::processPipelineMessage);

  connect(pipelineView, &SVPipelineView::pipelineFinished, this, &SIMPLView_UI::pipelineDidFinish);

  connect(pipelineView, &SVPipelineView::pipelineFinished, [=] {
    m_Ui->pipelineListWidget->pipelineFinished();
  });

  connect(pipelineView, &SVPipelineView::pipelineFilePathUpdated, this, &SIMPLView_UI::setWindowFilePath);

  connect(pipelineView, &SVPipelineView::pipelineChanged, this, &SIMPLView_UI::markDocumentAsDirty);

  connect(pipelineView, &SVPipelineView::filePathOpened, [=] (const QString &filePath) { m_OpenDialogLastFilePath = filePath; });

  connect(pipelineView, SIGNAL(filterInputWidgetEdited()), this, SLOT(markDocumentAsDirty()));

  connect(pipelineView, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));

  connect(pipelineView, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

  connect(pipelineView, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

//  connect(pipelineView, &SVPipelineView::pipelineDropped, pipelineModel, &PipelineModel::addPipeline);

  /* Pipeline Model Connections */
  connect(pipelineModel, &PipelineModel::statusMessageGenerated, [=] (const QString &msg) { statusBar()->showMessage(msg); });
  connect(pipelineModel, &PipelineModel::standardOutputMessageGenerated, [=] (const QString &msg) { addStdOutputMessage(msg); });

  connect(pipelineModel, &PipelineModel::pipelineDataChanged, [=] {  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SIMPLView_UI::openPipeline(const QString& filePath)
{
  int err = m_Ui->pipelineListWidget->getPipelineView()->openPipeline(filePath);

  QFileInfo fi(filePath);
  setWindowTitle(QString("[*]") + fi.baseName() + " - " + QApplication::applicationName());
  setWindowModified(false);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::executePipeline()
{
  m_Ui->pipelineListWidget->getPipelineView()->executePipeline();
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
void SIMPLView_UI::updateRecentFileList(const QString& file)
{
  // Clear the Recent Items Menu
  m_MenuRecentFiles->clear();

  // Get the list from the static object
  QStringList files = QtSRecentFileList::instance()->fileList();
  foreach(QString file, files)
  {
    QAction* action = m_MenuRecentFiles->addAction(QtSRecentFileList::instance()->parentAndFileName(file));
    action->setData(file);
    action->setVisible(true);
    connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
  }

  m_MenuRecentFiles->addSeparator();
  m_MenuRecentFiles->addAction(m_ActionClearRecentFiles);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::openRecentFile()
{
  QAction* action = qobject_cast<QAction*>(sender());

  if(action)
  {
    QString filePath = action->data().toString();

    dream3dApp->newInstanceFromFile(filePath);

    // Add file path to the recent files list for both instances
    QtSRecentFileList* list = QtSRecentFileList::instance();
    list->addFile(filePath);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::refreshWindowTitle()
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
    m_Ui->pipelineListWidget->setProgressValue(progValue);
  }
  else if(msg.getType() == PipelineMessage::MessageType::StatusMessageAndProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    m_Ui->pipelineListWidget->setProgressValue(progValue);

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
      m_Ui->stdOutDockWidget->setVisible(true);
    }

    // Allow status messages to open the issuesDockWidget as well
    if(HideDockSetting::OnStatusAndError == m_HideErrorTable)
    {
      m_Ui->issuesDockWidget->setVisible(true);
    }

    QString text = "<span style=\" color:#000000;\" >";
    text.append(msg.getText());
    text.append("</span>");
    m_Ui->stdOutWidget->appendText(text);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::pipelineDidFinish()
{
  // Re-enable FilterListToolboxWidget signals - resume adding filters
  m_Ui->filterListWidget->blockSignals(false);

  // Re-enable FilterLibraryToolboxWidget signals - resume adding filters
  m_Ui->filterLibraryWidget->blockSignals(false);
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

  QAction* issuesDummyAction = new QAction(m_Ui->issuesDockWidget->toggleViewAction()->text(), macApp);
  issuesDummyAction->setDisabled(true);
  actions.push_back(issuesDummyAction);

  QAction* stdOutDummyAction = new QAction(m_Ui->stdOutDockWidget->toggleViewAction()->text(), macApp);
  stdOutDummyAction->setDisabled(true);
  actions.push_back(stdOutDummyAction);

  QAction* dataBrowserDummyAction = new QAction(m_Ui->dataBrowserDockWidget->toggleViewAction()->text(), macApp);
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
  return m_Ui->dataBrowserWidget;
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
  m_Ui->fiwFrameVLayout->addWidget(widget);
  widget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::clearFilterInputWidget()
{
  QLayoutItem* item = m_Ui->fiwFrameVLayout->takeAt(0);
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
    m_Ui->issuesDockWidget->setVisible(hasErrors);
  }

  if(HideDockSetting::OnError == m_HideStdOutput || HideDockSetting::OnStatusAndError == m_HideStdOutput)
  {
    m_Ui->stdOutDockWidget->setVisible(hasErrors);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setStatusBarMessage(const QString& msg)
{
  m_Ui->statusbar->showMessage(msg);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::addStdOutputMessage(const QString& msg)
{
  QString text = "<span style=\" color:#000000;\" >";
  text.append(msg);
  text.append("</span>");
  m_Ui->stdOutWidget->appendText(text);
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
PipelineModel* SIMPLView_UI::getPipelineModel()
{
  SVPipelineView* pipelineView = m_Ui->pipelineListWidget->getPipelineView();
  return pipelineView->getPipelineModel();
}
