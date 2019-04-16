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
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Plugin/PluginManager.h"
#include "SIMPLib/Utilities/SIMPLDataPathValidator.h"

#include "SVWidgetsLib/Animations/PipelineItemBorderSizeAnimation.h"
#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/QtSupport/QtSMacros.h"
#include "SVWidgetsLib/QtSupport/QtSPluginFrame.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSFileUtils.h"
#include "SVWidgetsLib/Widgets/BookmarksModel.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/BookmarksTreeView.h"
#include "SVWidgetsLib/Widgets/FilterLibraryToolboxWidget.h"
#include "SVWidgetsLib/Widgets/PipelineItemDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineListWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"
#include "SVWidgetsLib/Widgets/StatusBarWidget.h"
#include "SVWidgetsLib/Widgets/util/AddFilterCommand.h"
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

#include "SIMPLView/AboutSIMPLView.h"
#include "SIMPLView/SIMPLView.h"
#include "SIMPLView/SIMPLViewApplication.h"
#include "SIMPLView/SIMPLViewConstants.h"
#include "SIMPLView/SIMPLViewVersion.h"
#include "SIMPLView/SIMPLViewUIMessageHandler.h"

#include "BrandedStrings.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI::SIMPLView_UI(QWidget* parent)
: QMainWindow(parent)
, m_Ui(new Ui::SIMPLView_UI)
, m_LastOpenedFilePath(QDir::homePath())
{
  // Register all of the Filters we know about - the rest will be loaded through plugins
  //  which all should have been loaded by now.
  m_FilterManager = FilterManager::Instance();
  // m_FilterManager->RegisterKnownFilters(m_FilterManager);

  // Register all the known filterWidgets
  m_FilterWidgetManager = FilterWidgetManager::Instance();
  FilterWidgetManager::RegisterKnownFilterWidgets();

  // Calls the Parent Class to do all the Widget Initialization that were created
  // using the QDesigner program
  m_Ui->setupUi(this);

  dream3dApp->registerSIMPLViewWindow(this);

  // Do our own widget initializations
  setupGui();

  this->setAcceptDrops(true);

  // Read various settings
  readSettings();
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnError == IssuesWidget::GetHideDockSetting())
  {
    m_Ui->issuesDockWidget->setHidden(true);
  }
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnError == StandardOutputWidget::GetHideDockSetting())
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
  writeSettings();

  dream3dApp->unregisterSIMPLViewWindow(this);

  if(dream3dApp->activeWindow() == this)
  {
    dream3dApp->setActiveWindow(nullptr);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::resizeEvent(QResizeEvent* event)
{
  QMainWindow::resizeEvent(event);

  emit parentResized();

  // We need to write the window settings so that any new windows will open with these window settings
  writeWindowSettings();
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
  QString filePath;
  if(windowFilePath().isEmpty())
  {
    // When the file hasn't been saved before, the same functionality as a "Save As" occurs...
    return savePipelineAs();
  }

    filePath = windowFilePath();

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
  QtSRecentFileList* list = QtSRecentFileList::Instance();
  list->addFile(filePath);

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
  QString proposedFile = m_LastOpenedFilePath + QDir::separator() + "Untitled.json";
  QString filePath = QFileDialog::getSaveFileName(this, tr("Save Pipeline To File"), proposedFile, tr("Json File (*.json);;SIMPLView File (*.dream3d);;All Files (*.*)"));
  if(filePath.isEmpty())
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
    QtSRecentFileList* list = QtSRecentFileList::Instance();
    list->addFile(filePath);

    setWindowFilePath(filePath);
  }
  else
  {
    return false;
  }

  // Cache the last directory
  m_LastOpenedFilePath = filePath;

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
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::activateBookmark(const QString& filePath, bool execute)
{
  SIMPLView_UI* instance = dream3dApp->getActiveInstance();
  if(instance != nullptr && !instance->isWindowModified() && instance->getPipelineModel()->isEmpty())
  {
    instance->openPipeline(filePath);
  }
  else
  {
    instance = dream3dApp->newInstanceFromFile(filePath);
  }

  QtSRecentFileList* list = QtSRecentFileList::Instance();
  list->addFile(filePath);

  if(execute)
  {
    instance->executePipeline();
  }

  instance->raise();
  QApplication::setActiveWindow(instance);
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void SIMPLView_UI::closeEvent(QCloseEvent* event)
{
  if(m_Ui->pipelineListWidget->getPipelineView()->isPipelineCurrentlyRunning())
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

  // Status Bar Widget needs to write out its settings BEFORE the main window is closed
  //  m_StatusBar->writeSettings();

  event->accept();
}

// -----------------------------------------------------------------------------
//  Read our settings from a file
// -----------------------------------------------------------------------------
void SIMPLView_UI::readSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  // Have the pipeline builder read its settings from the prefs file
  readWindowSettings();
  readVersionCheckSettings();

  // Read dock widget settings
  prefs->beginGroup(SIMPLView::DockWidgetSettings::GroupName);

  prefs->beginGroup(SIMPLView::DockWidgetSettings::IssuesDockGroupName);
  readDockWidgetSettings(prefs.data(), m_Ui->issuesDockWidget);
  prefs->endGroup();

  prefs->beginGroup(SIMPLView::DockWidgetSettings::StandardOutputGroupName);
  readDockWidgetSettings(prefs.data(), m_Ui->stdOutDockWidget);
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
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::readWindowSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

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
void SIMPLView_UI::readVersionCheckSettings()
{
}

// -----------------------------------------------------------------------------
//  Write our Prefs to file
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeSettings()
{
  // Have the pipeline builder write its settings to the prefs file
  writeWindowSettings();

  // Have the version check widet write its preferences.
  writeVersionCheckSettings();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeVersionCheckSettings()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::writeWindowSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

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
void SIMPLView_UI::setupGui()
{
  setTabPosition(Qt::DockWidgetArea::TopDockWidgetArea, QTabWidget::TabPosition::North); 
  setTabPosition(Qt::DockWidgetArea::RightDockWidgetArea, QTabWidget::TabPosition::North); 
  setTabPosition(Qt::DockWidgetArea::BottomDockWidgetArea, QTabWidget::TabPosition::North); 
  setTabPosition(Qt::DockWidgetArea::LeftDockWidgetArea, QTabWidget::TabPosition::North); 

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
  m_Ui->filterLibraryWidget->refreshFilterGroups();

  // Read the toolbox settings and update the filter list
  m_Ui->filterListWidget->loadFilterList();

  tabifyDockWidget(m_Ui->filterListDockWidget, m_Ui->filterLibraryDockWidget);
  tabifyDockWidget(m_Ui->filterLibraryDockWidget, m_Ui->bookmarksDockWidget);

  m_Ui->filterListDockWidget->raise();

  // Shortcut to close the window
  new QShortcut(QKeySequence(QKeySequence::Close), this, SLOT(close()));

  //  m_StatusBar = new StatusBarWidget();
  //  this->statusBar()->insertPermanentWidget(0, m_StatusBar, 0);

  //  m_StatusBar->setButtonAction(m_Ui->filterListDockWidget, StatusBarWidget::Button::FilterList);
  //  m_StatusBar->setButtonAction(m_Ui->filterLibraryDockWidget, StatusBarWidget::Button::FilterLibrary);
  //  m_StatusBar->setButtonAction(m_Ui->bookmarksDockWidget, StatusBarWidget::Button::Bookmarks);
  //  m_StatusBar->setButtonAction(m_Ui->pipelineDockWidget, StatusBarWidget::Button::Pipeline);
  //  m_StatusBar->setButtonAction(m_Ui->issuesDockWidget, StatusBarWidget::Button::Issues);
  //  m_StatusBar->setButtonAction(m_Ui->stdOutDockWidget, StatusBarWidget::Button::Console);
  //  m_StatusBar->setButtonAction(m_Ui->dataBrowserDockWidget, StatusBarWidget::Button::DataStructure);

  //  m_StatusBar->readSettings();

  //  connect(m_Ui->issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), m_StatusBar, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(m_Ui->issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), this, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(m_Ui->issuesWidget, SIGNAL(showTable(bool)), m_Ui->issuesDockWidget, SLOT(setVisible(bool)));

  connectDockWidgetSignalsSlots(m_Ui->bookmarksDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->dataBrowserDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->filterLibraryDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->filterListDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->issuesDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->pipelineDockWidget);
  connectDockWidgetSignalsSlots(m_Ui->stdOutDockWidget);

  m_Ui->bookmarksDockWidget->installEventFilter(this);
  m_Ui->dataBrowserDockWidget->installEventFilter(this);
  m_Ui->filterLibraryDockWidget->installEventFilter(this);
  m_Ui->filterListDockWidget->installEventFilter(this);
  m_Ui->issuesDockWidget->installEventFilter(this);
  m_Ui->pipelineDockWidget->installEventFilter(this);
  m_Ui->stdOutDockWidget->installEventFilter(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLView_UI::eventFilter(QObject* watched, QEvent* event)
{
  if(static_cast<QDockWidget*>(watched) != nullptr)
  {
    // Writes the window settings when dock widgets are resized or when the tabs are rearranged.  ChildRemoved and ChildAdded
    // are the only signals emitted when changing the order of the tabs, and there doesn't seem to be a better way to detect that.
    if(event->type() == QEvent::Resize || event->type() == QEvent::ChildRemoved || event->type() == QEvent::ChildAdded)
    {
      writeWindowSettings();
    }
  }

  return QMainWindow::eventFilter(watched, event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::createSIMPLViewMenuSystem()
{
  m_SIMPLViewMenu = new QMenuBar(this);

  m_MenuFile = new QMenu("File", this);
  m_MenuEdit = new QMenu("Edit", this);
  m_MenuView = new QMenu("View", this);
  m_MenuBookmarks = new QMenu("Bookmarks", this);
  m_MenuPipeline = new QMenu("Pipeline", this);
  m_MenuHelp = new QMenu("Help", this);
  m_MenuAdvanced = new QMenu("Advanced", this);
  m_MenuThemes = new QMenu("Themes", this);
  QMenu* menuRecentFiles = dream3dApp->getRecentFilesMenu();

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
  m_ActionClearCache = new QAction("Reset Preferences", this);

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
  m_ActionSaveAs->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
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
  m_MenuFile->addAction(menuRecentFiles->menuAction());
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

  // Create View Menu
  m_SIMPLViewMenu->addMenu(m_MenuView);

  QStringList themeNames = BrandedStrings::LoadedThemeNames;
  if(!themeNames.empty()) // We are not counting the Default theme when deciding whether or not to add the theme menu
  {
    m_ThemeActionGroup = new QActionGroup(this);
    m_MenuThemes = dream3dApp->createThemeMenu(m_ThemeActionGroup, m_SIMPLViewMenu);

    m_MenuView->addMenu(m_MenuThemes);

    m_MenuView->addSeparator();
  }

  m_MenuView->addAction(m_Ui->filterListDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->filterLibraryDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->bookmarksDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->pipelineDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->issuesDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->stdOutDockWidget->toggleViewAction());
  m_MenuView->addAction(m_Ui->dataBrowserDockWidget->toggleViewAction());

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

  #if defined SIMPL_RELATIVE_PATH_CHECK

  m_MenuDataDirectory = new QMenu("Data Directory", this);
  m_ActionSetDataFolder = new QAction("Set Location...", this);
  m_ActionShowDataFolder = new QAction("Show Location", this);

  connect(m_ActionSetDataFolder, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenSetDataFolderTriggered);
  connect(m_ActionShowDataFolder, &QAction::triggered, dream3dApp, &SIMPLViewApplication::listenShowDataFolderTriggered);

  m_MenuHelp->addSeparator();
  m_MenuHelp->addMenu(m_MenuDataDirectory);
  m_MenuDataDirectory->addAction(m_ActionSetDataFolder);
  m_MenuDataDirectory->addAction(m_ActionShowDataFolder);
  #endif

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
  connect(m_Ui->bookmarksWidget, &BookmarksToolboxWidget::bookmarkActivated, this, &SIMPLView_UI::activateBookmark);
  connect(m_Ui->bookmarksWidget, SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));

  connect(m_Ui->bookmarksWidget, &BookmarksToolboxWidget::raiseBookmarksDockWidget, [=] { showDockWidget(m_Ui->bookmarksDockWidget); });

  /* Pipeline List Widget Connections */
  connect(m_Ui->pipelineListWidget, &PipelineListWidget::pipelineCanceled, pipelineView, &SVPipelineView::cancelPipeline);

  /* Pipeline View Connections */
  connect(pipelineView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SIMPLView_UI::filterSelectionChanged);
  connect(pipelineView, &SVPipelineView::filterParametersChanged, [=] (AbstractFilter::Pointer filter) {
    m_Ui->dataBrowserWidget->filterActivated(filter);
    markDocumentAsDirty();
  });
  connect(pipelineView, &SVPipelineView::clearDataStructureWidgetTriggered, [=] { m_Ui->dataBrowserWidget->filterActivated(AbstractFilter::NullPointer()); });
  connect(pipelineView, &SVPipelineView::filterInputWidgetNeedsCleared, this, &SIMPLView_UI::clearFilterInputWidget);
  connect(pipelineView, &SVPipelineView::displayIssuesTriggered, m_Ui->issuesWidget, &IssuesWidget::displayCachedMessages);
  connect(pipelineView, &SVPipelineView::clearIssuesTriggered, m_Ui->issuesWidget, &IssuesWidget::clearIssues);
  connect(pipelineView, &SVPipelineView::writeSIMPLViewSettingsTriggered, [=] { writeSettings(); });

  // Connection that displays issues in the Issue Table when the preflight is finished
  connect(pipelineView, &SVPipelineView::preflightFinished, [=](int32_t pipelineFilterCount, int err) {
    m_Ui->dataBrowserWidget->refreshData();
    m_Ui->issuesWidget->displayCachedMessages();
    m_Ui->pipelineListWidget->preflightFinished(pipelineFilterCount, err);
  });

  connect(pipelineView, &SVPipelineView::pipelineHasMessage, this, &SIMPLView_UI::processPipelineMessage);
  connect(pipelineView, &SVPipelineView::pipelineFinished, this, &SIMPLView_UI::pipelineDidFinish);
  connect(pipelineView, &SVPipelineView::pipelineFilePathUpdated, this, &SIMPLView_UI::setWindowFilePath);

  connect(pipelineView, &SVPipelineView::pipelineChanged, this, &SIMPLView_UI::handlePipelineChanges);
  connect(pipelineView, &SVPipelineView::filePathOpened, [=](const QString& filePath) { m_LastOpenedFilePath = filePath; });

  connect(pipelineView, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));
  connect(pipelineView, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));
  connect(pipelineView, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

  /* Pipeline Model Connections */
  connect(pipelineModel, &PipelineModel::statusMessageGenerated, [=](const QString& msg) { statusBar()->showMessage(msg); });
  connect(pipelineModel, &PipelineModel::standardOutputMessageGenerated, [=](const QString& msg) { addStdOutputMessage(msg); });

  connect(pipelineModel, &PipelineModel::pipelineDataChanged, [=] {});
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::connectDockWidgetSignalsSlots(QDockWidget* dockWidget)
{
  connect(dockWidget, &QDockWidget::dockLocationChanged, [=] { writeWindowSettings(); });
  connect(dockWidget, &QDockWidget::topLevelChanged, [=] { writeWindowSettings(); });
  connect(dockWidget, &QDockWidget::visibilityChanged, [=] { writeWindowSettings(); });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::showDockWidget(QDockWidget* dockWidget)
{
  if(tabifiedDockWidgets(dockWidget).isEmpty() && !dockWidget->toggleViewAction()->isChecked())
  {
    dockWidget->toggleViewAction()->trigger();
  }

  dockWidget->raise();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SIMPLView_UI::openPipeline(const QString& filePath)
{
  SVPipelineView* pipelineView = m_Ui->pipelineListWidget->getPipelineView();
  int err = pipelineView->openPipeline(filePath);
  if (err >= 0)
  {
    PipelineModel* model = pipelineView->getPipelineModel();
    if (model->rowCount() > 0)
    {
      QModelIndex index = model->index(0, PipelineItem::PipelineItemData::Contents);
      pipelineView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    }
  }

  QFileInfo fi(filePath);
  setWindowTitle(QString("[*]") + fi.baseName() + " - " + QApplication::applicationName());
  setWindowFilePath(filePath);
  setWindowModified(false);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::handlePipelineChanges()
{
  markDocumentAsDirty();

  SVPipelineView* pipelineView = m_Ui->pipelineListWidget->getPipelineView();
  QModelIndexList selectedIndexes = pipelineView->selectionModel()->selectedRows();
  qSort(selectedIndexes);

  if(selectedIndexes.size() == 1)
  {
    QModelIndex selectedIndex = selectedIndexes[0];
    PipelineModel* model = getPipelineModel();

    AbstractFilter::Pointer filter = model->filter(selectedIndex);
    m_Ui->dataBrowserWidget->filterActivated(filter);
  }
  else
  {
    m_Ui->dataBrowserWidget->filterActivated(AbstractFilter::NullPointer());
  }
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
QMessageBox::StandardButton SIMPLView_UI::checkDirtyDocument()
{

  if(this->isWindowModified())
  {
    int r = QMessageBox::warning(this, BrandedStrings::ApplicationName, tr("The Pipeline has been modified.\nDo you want to save your changes?"), QMessageBox::Save | QMessageBox::Default,
                                 QMessageBox::Discard, QMessageBox::Cancel | QMessageBox::Escape);
    if(r == QMessageBox::Save)
    {
      if(savePipeline())
      {
        return QMessageBox::Save;
      }

        return QMessageBox::Cancel;
    }
    if(r == QMessageBox::Discard)
    {
      return QMessageBox::Discard;
    }
    if(r == QMessageBox::Cancel)
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
void SIMPLView_UI::processPipelineMessage(const AbstractMessage::Pointer& msg)
{
  SIMPLViewUIMessageHandler msgHandler(this);
  msg->visit(&msgHandler);
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

  QModelIndexList selectedIndexes = m_Ui->pipelineListWidget->getPipelineView()->selectionModel()->selectedRows();
  qSort(selectedIndexes);

  if(selectedIndexes.size() == 1)
  {
    QModelIndex selectedIndex = selectedIndexes[0];
    PipelineModel* model = getPipelineModel();

    AbstractFilter::Pointer filter = model->filter(selectedIndex);
    m_Ui->dataBrowserWidget->filterActivated(filter);
  }
  else
  {
    m_Ui->dataBrowserWidget->filterActivated(AbstractFilter::NullPointer());
  }

  m_Ui->pipelineListWidget->pipelineFinished();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::showFilterHelp(const QString& className)
{
// Launch the dialog
#ifdef SIMPL_USE_QtWebEngine
  SVUserManualDialog::LaunchHelpDialog(className);
#elif(defined(SIMPL_USE_MKDOCS) || defined(SIMPL_USE_DISCOUNT))
  QUrl helpURL = URL_GENERATOR::GenerateHTMLUrl(className);
  bool didOpen = QDesktopServices::openUrl(helpURL);
  if(!didOpen)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("Error Opening Help File"));
    msgBox.setInformativeText(QString::fromLatin1("SIMPLView could not open the help file path ") + helpURL.path());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
  }
#else
  QMessageBox::StandardButton reply;
  reply = QMessageBox::critical(this, tr("Documentation Error"), "Documentation was disabled in this build.", QMessageBox::Ok);
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
  if(!didOpen)
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
DataStructureWidget* SIMPLView_UI::getDataStructureWidget()
{
  return m_Ui->dataBrowserWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::filterSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  SVPipelineView* pipelineView = m_Ui->pipelineListWidget->getPipelineView();
  PipelineModel* pipelineModel = pipelineView->getPipelineModel();

  QModelIndexList selectedIndexes = pipelineView->selectionModel()->selectedRows();
  qSort(selectedIndexes);

  // Animate a selection border for selected indexes
  for(const QModelIndex& index : selected.indexes())
  {
    new PipelineItemBorderSizeAnimation(pipelineModel, QPersistentModelIndex(index));
  }

  // Remove selection border from deselected indexes
  for(const QModelIndex& index : deselected.indexes())
  {
    pipelineModel->setData(index, -1, PipelineModel::Roles::BorderSizeRole);
  }

  if(selectedIndexes.size() == 1)
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

  if(m_FilterInputWidget != nullptr)
  {
    emit m_FilterInputWidget->endPathFiltering();
    emit m_FilterInputWidget->endViewPaths();
    emit m_FilterInputWidget->endDataStructureFiltering();
  }

  // Clear the filter input widget
  clearFilterInputWidget();

  // Alert to DataArrayPath requirements
  connect(widget, SIGNAL(viewPathsMatchingReqs(DataContainerSelectionFilterParameter::RequirementType)), getDataStructureWidget(),
          SLOT(setViewReqs(DataContainerSelectionFilterParameter::RequirementType)), Qt::ConnectionType::UniqueConnection);
  connect(widget, SIGNAL(viewPathsMatchingReqs(AttributeMatrixSelectionFilterParameter::RequirementType)), getDataStructureWidget(),
          SLOT(setViewReqs(AttributeMatrixSelectionFilterParameter::RequirementType)), Qt::ConnectionType::UniqueConnection);
  connect(widget, SIGNAL(viewPathsMatchingReqs(DataArraySelectionFilterParameter::RequirementType)), getDataStructureWidget(), SLOT(setViewReqs(DataArraySelectionFilterParameter::RequirementType)),
          Qt::ConnectionType::UniqueConnection);
  connect(widget, SIGNAL(endViewPaths()), getDataStructureWidget(), SLOT(clearViewRequirements()), Qt::ConnectionType::UniqueConnection);
  connect(getDataStructureWidget(), SIGNAL(filterPath(DataArrayPath)), widget, SIGNAL(filterPath(DataArrayPath)), Qt::ConnectionType::UniqueConnection);
  connect(getDataStructureWidget(), SIGNAL(endDataStructureFiltering()), widget, SIGNAL(endDataStructureFiltering()), Qt::ConnectionType::UniqueConnection);
  connect(getDataStructureWidget(), SIGNAL(applyPathToFilteringParameter(DataArrayPath)), widget, SIGNAL(applyPathToFilteringParameter(DataArrayPath)));

  emit widget->endPathFiltering();

  // Set the widget into the frame
  m_Ui->fiwFrameVLayout->addWidget(widget);
  m_FilterInputWidget = widget;
  widget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::clearFilterInputWidget()
{
  QLayoutItem* item = m_Ui->fiwFrameVLayout->takeAt(0);
  if(item != nullptr)
  {
    QWidget* w = item->widget();
    if(w != nullptr)
    {
      w->hide();
      w->setParent(nullptr);
    }
  }

  m_FilterInputWidget = nullptr;
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

  SIMPLView::DockWidgetSettings::HideDockSetting errorTableSetting = IssuesWidget::GetHideDockSetting();
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnError == errorTableSetting 
     || SIMPLView::DockWidgetSettings::HideDockSetting::OnStatusAndError == errorTableSetting)
  {
    m_Ui->issuesDockWidget->setVisible(hasErrors);
  }

  SIMPLView::DockWidgetSettings::HideDockSetting stdOutSetting = StandardOutputWidget::GetHideDockSetting();
  if(SIMPLView::DockWidgetSettings::HideDockSetting::OnError == stdOutSetting 
     || SIMPLView::DockWidgetSettings::HideDockSetting::OnStatusAndError == stdOutSetting)
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
  m_Ui->stdOutWidget->appendText(msg);
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
