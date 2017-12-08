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
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QToolButton>

#include "SIMPLView/SIMPLView.h"

#ifdef SIMPLView_USE_QtWebEngine
#include "Common/SIMPLViewUserManualDialog.h"
#else
#include "SVWidgetsLib/QtSupport/QtSHelpUrlGenerator.h"
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMessageBox>
#endif

//-- SIMPLView Includes
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Plugin/PluginManager.h"

#include "SVWidgetsLib/QtSupport/QtSMacros.h"
#include "SVWidgetsLib/QtSupport/QtSPluginFrame.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSStyles.h"

#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Dialogs/UpdateCheck.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckData.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckDialog.h"
#include "SVWidgetsLib/Widgets/BookmarksModel.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/FilterLibraryToolboxWidget.h"
#include "SVWidgetsLib/Widgets/SIMPLViewMenuItems.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"
#include "SVWidgetsLib/Widgets/PipelineTreeController.h"
#include "SVWidgetsLib/Widgets/PipelineTreeModel.h"
#include "SVWidgetsLib/Widgets/PipelineTreeView.h"
#include "SVWidgetsLib/Widgets/StatusBarWidget.h"

#include "SIMPLView/MacSIMPLViewApplication.h"
#include "SIMPLView/SIMPLViewConstants.h"
#include "SIMPLView/StandardSIMPLViewApplication.h"

#include "BrandedStrings.h"

// Initialize private static member variable
QString SIMPLView_UI::m_OpenDialogLastFilePath = "";

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI::SIMPLView_UI(QWidget* parent)
: QMainWindow(parent)
, m_TreeController(new PipelineTreeController(this))
, m_WorkerThread(nullptr)
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

  if(m_WorkerThread)
  {
    delete m_WorkerThread;
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
    m_PipelineViewWidget->writePipeline(filePath);

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
  int err = m_PipelineViewWidget->writePipeline(filePath);

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
  int showError = static_cast<int>(HideDockSetting::OnError);
  int hideDockSetting = prefs->value("HideDockSetting", QVariant(showError)).toInt();
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
  prefs->setValue("HideDockSetting", valuei);
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

  if (USE_PIPELINE_TREE_WIDGET)
  {
    setupPipelineTreeView();
  }
  else
  {
    setupPipelineViewWidget();
  }

  // Add the Issues widget as the observer of the controller
  m_TreeController->addPipelineMessageObserver(issuesWidget);

  // Hook up the signals from the various docks to the PipelineViewWidget that will either add a filter
  // or load an entire pipeline into the view
  connectSignalsSlots();

  // This will set the initial list of filters in the FilterListToolboxWidget
  // Tell the Filter Library that we have more Filters (potentially)
  getFilterLibraryToolboxWidget()->refreshFilterGroups();

  // Setup the undo stack.  This must occur after the connectSignalsSlots function.
  m_TreeController->setupUndoStack();

  issuesDockWidget->toggleViewAction()->setText("Show " + issuesDockWidget->toggleViewAction()->text());
  stdOutDockWidget->toggleViewAction()->setText("Show " + stdOutDockWidget->toggleViewAction()->text());
  dataBrowserDockWidget->toggleViewAction()->setText("Show " + dataBrowserDockWidget->toggleViewAction()->text());

  startPipelineBtn->setStyleSheet(getStartPipelineIdleStyle());
  startPipelineBtn->setDisabled(true);

  // Shortcut to close the window
  new QShortcut(QKeySequence(QKeySequence::Close), this, SLOT(close()));

  m_StatusBar = new StatusBarWidget();
  this->statusBar()->insertPermanentWidget(0, m_StatusBar, 0);

  m_StatusBar->setButtonAction(issuesDockWidget, StatusBarWidget::Button::Issues);
  m_StatusBar->setButtonAction(stdOutDockWidget, StatusBarWidget::Button::Console);
  m_StatusBar->setButtonAction(dataBrowserDockWidget, StatusBarWidget::Button::DataStructure);

  connect(issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), m_StatusBar, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(issuesWidget, SIGNAL(tableHasErrors(bool, int, int)), this, SLOT(issuesTableHasErrors(bool, int, int)));
  connect(issuesWidget, SIGNAL(showTable(bool)), issuesDockWidget, SLOT(setVisible(bool)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setupPipelineViewWidget()
{
  // Create the Pipeline View Widget
  m_PipelineViewWidget = new SVPipelineViewWidget();
  m_PipelineViewWidget->setObjectName(QStringLiteral("m_PipelineViewWidget"));
  m_PipelineViewWidget->setGeometry(QRect(0, 0, 755, 516));
  QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
  sizePolicy2.setHorizontalStretch(0);
  sizePolicy2.setVerticalStretch(0);
  sizePolicy2.setHeightForWidth(m_PipelineViewWidget->sizePolicy().hasHeightForWidth());
  m_PipelineViewWidget->setSizePolicy(sizePolicy2);
  m_PipelineViewWidget->setMinimumSize(QSize(250, 0));
  m_PipelineViewWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_PipelineViewWidget->setAcceptDrops(true);
  m_PipelineViewWidget->setStyleSheet(QLatin1String("PipelineViewWidget\n"
"  {\n"
"  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(233, 236, 241, 255), stop:1.0 rgba(211, 216, 224, 255));\n"
"  }"));

  // Create the Pipeline Scroll Area
  QScrollArea* pipelineViewScrollArea = new QScrollArea(this);
  QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(pipelineViewScrollArea->sizePolicy().hasHeightForWidth());
  pipelineViewScrollArea->setSizePolicy(sizePolicy1);
  pipelineViewScrollArea->setMaximumSize(QSize(16777215, 16777215));
  pipelineViewScrollArea->setMouseTracking(false);
  pipelineViewScrollArea->setFocusPolicy(Qt::StrongFocus);
  pipelineViewScrollArea->setAcceptDrops(true);
  pipelineViewScrollArea->setFrameShape(QFrame::Panel);
  pipelineViewScrollArea->setFrameShadow(QFrame::Raised);
  pipelineViewScrollArea->setLineWidth(1);
  pipelineViewScrollArea->setWidgetResizable(true);
  pipelineViewScrollArea->verticalScrollBar()->setSingleStep(5);
  pipelineViewScrollArea->setWidget(m_PipelineViewWidget);

  m_PipelineViewWidget->setScrollArea(pipelineViewScrollArea);
  gridLayout_2->addWidget(pipelineViewScrollArea, 0, 0, 1, 1);

  // Create the model
  PipelineTreeModel* model = new PipelineTreeModel(m_PipelineViewWidget);
  model->setMaxNumberOfPipelines(1);
  m_PipelineViewWidget->setModel(model);

  // Set the Data Browser widget into the Pipeline View widget
  m_PipelineViewWidget->setDataStructureWidget(dataBrowserWidget);

  // Set the IssuesWidget as a PipelineMessageObserver Object.
  m_PipelineViewWidget->addPipelineMessageObserver(issuesWidget);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::setupPipelineTreeView()
{
  m_PipelineTreeView = new PipelineTreeView(pipelineInteralWidget);
  m_PipelineTreeView->setAlternatingRowColors(true);
  gridLayout_2->addWidget(m_PipelineTreeView, 0, 0, 1, 1);

  PipelineTreeModel* model = new PipelineTreeModel(m_PipelineTreeView);
  m_PipelineTreeView->setModel(model);

  m_PipelineTreeView->header()->setSectionResizeMode(PipelineTreeItem::TreeItemData::Name, QHeaderView::Stretch);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::disconnectSignalsSlots()
{
  DocRequestManager* docRequester = DocRequestManager::Instance();
  PipelineTreeModel* model = getPipelineTreeModel();

  disconnect(docRequester, SIGNAL(showFilterDocs(const QString&)), this, SLOT(showFilterHelp(const QString&)));

  disconnect(docRequester, SIGNAL(showFilterDocUrl(const QUrl&)), this, SLOT(showFilterHelpUrl(const QUrl&)));

  disconnect(this, SIGNAL(bookmarkNeedsToBeAdded(const QString&, const QModelIndex&)), getBookmarksToolboxWidget(), SLOT(addBookmark(const QString&, const QModelIndex&)));

  disconnect(m_TreeController, &PipelineTreeController::statusMessageGenerated, 0, 0);
  disconnect(m_TreeController, &PipelineTreeController::standardOutputMessageGenerated, 0, 0);

  disconnect(m_TreeController, &PipelineTreeController::undoActionGenerated, 0, 0);

  disconnect(m_TreeController, &PipelineTreeController::redoActionGenerated, 0, 0);

  // Connection that allows the Pipeline Tree controller to clear the Issues Table
  disconnect(m_TreeController, &PipelineTreeController::pipelineIssuesCleared, issuesWidget, &IssuesWidget::clearIssues);

  // Connection that displays issues in the Issue Table when the preflight is finished
  disconnect(m_TreeController, &PipelineTreeController::preflightFinished, 0, 0);

  // Connection that refreshes the Data Browser when the preflight is finished
  disconnect(m_TreeController, &PipelineTreeController::preflightFinished, 0, 0);

  // Connection that does post-preflight updates on this instance of SIMPLView_UI when the preflight is finished
  disconnect(m_TreeController, &PipelineTreeController::preflightFinished, this, &SIMPLView_UI::preflightDidFinish);

  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    disconnect(m_PipelineViewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)), this, SLOT(setFilterInputWidget(FilterInputWidget*)));

    disconnect(m_PipelineViewWidget, SIGNAL(filterInputWidgetNeedsCleared()), this, SLOT(clearFilterInputWidget()));

    disconnect(m_PipelineViewWidget, SIGNAL(filterInputWidgetEdited()), this, SLOT(markDocumentAsDirty()));

    disconnect(m_PipelineViewWidget, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));

    disconnect(m_PipelineViewWidget, SIGNAL(preflightFinished(int)), this, SLOT(preflightDidFinish(int)));

    disconnect(m_PipelineViewWidget, &SVPipelineViewWidget::undoCommandCreated, m_TreeController, &PipelineTreeController::addUndoCommand);

    disconnect(m_PipelineViewWidget, &SVPipelineViewWidget::undoRequested, m_TreeController, &PipelineTreeController::undo);

    disconnect(m_PipelineViewWidget, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

    disconnect(m_PipelineViewWidget, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

    disconnect(m_PipelineViewWidget, SIGNAL(deleteKeyPressed(SVPipelineViewWidget*)), this, SIGNAL(deleteKeyPressed(SVPipelineViewWidget*)));

    disconnect(getBookmarksToolboxWidget(), SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));
  }
  else
  {
    // Connection to preflight both pipelines when moving filters between pipelines
    disconnect(model, &PipelineTreeModel::rowsMoved, 0, 0);

    // Connection that marks the document dirty whenever a filter is enabled/disabled
    disconnect(m_PipelineTreeView, &PipelineTreeView::filterEnabledStateChanged, this, &SIMPLView_UI::markDocumentAsDirty);

    // Connection that allows the view to call for a preflight, which gets picked up by the Pipeline Tree controller
    disconnect(m_PipelineTreeView, &PipelineTreeView::needsPreflight, m_TreeController, &PipelineTreeController::preflightPipeline);

    // Connection to update the active pipeline when the user decides to change it
    disconnect(m_PipelineTreeView, &PipelineTreeView::activePipelineChanged, m_TreeController, &PipelineTreeController::updateActivePipeline);

    disconnect(m_PipelineTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, 0, 0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::connectSignalsSlots()
{
  DocRequestManager* docRequester = DocRequestManager::Instance();
  PipelineTreeModel* model = getPipelineTreeModel();

  connect(docRequester, SIGNAL(showFilterDocs(const QString&)), this, SLOT(showFilterHelp(const QString&)));

  connect(docRequester, SIGNAL(showFilterDocUrl(const QUrl&)), this, SLOT(showFilterHelpUrl(const QUrl&)));

  connect(this, SIGNAL(bookmarkNeedsToBeAdded(const QString&, const QModelIndex&)), getBookmarksToolboxWidget(), SLOT(addBookmark(const QString&, const QModelIndex&)));

  connect(m_TreeController, &PipelineTreeController::statusMessageGenerated, [=] (const QString &msg) { statusBar()->showMessage(msg); });
  connect(m_TreeController, &PipelineTreeController::standardOutputMessageGenerated, [=] (const QString &msg) { addStdOutputMessage(msg); });

  connect(m_TreeController, &PipelineTreeController::undoActionGenerated, [=] (QAction* actionUndo) {
    PipelineTreeModel* model = getPipelineTreeModel();
    model->setActionUndo(actionUndo);
  });

  connect(m_TreeController, &PipelineTreeController::redoActionGenerated, [=] (QAction* actionRedo) {
    PipelineTreeModel* model = getPipelineTreeModel();
    model->setActionRedo(actionRedo);
  });

  // Connection that allows the Pipeline Tree controller to clear the Issues Table
  connect(m_TreeController, &PipelineTreeController::pipelineIssuesCleared, issuesWidget, &IssuesWidget::clearIssues);

  // Connection that displays issues in the Issue Table when the preflight is finished
  connect(m_TreeController, &PipelineTreeController::preflightFinished, [=] { issuesWidget->displayCachedMessages(); });

  // Connection that refreshes the Data Browser when the preflight is finished
  connect(m_TreeController, &PipelineTreeController::preflightFinished, [=] { dataBrowserWidget->refreshData(); });

  // Connection that does post-preflight updates on this instance of SIMPLView_UI when the preflight is finished
  connect(m_TreeController, &PipelineTreeController::preflightFinished, this, &SIMPLView_UI::preflightDidFinish);

  if (USE_PIPELINE_TREE_WIDGET == false)
  {
    connect(m_PipelineViewWidget, SIGNAL(filterInputWidgetChanged(FilterInputWidget*)), this, SLOT(setFilterInputWidget(FilterInputWidget*)));

    connect(m_PipelineViewWidget, SIGNAL(filterInputWidgetNeedsCleared()), this, SLOT(clearFilterInputWidget()));

    connect(m_PipelineViewWidget, SIGNAL(filterInputWidgetEdited()), this, SLOT(markDocumentAsDirty()));

    connect(m_PipelineViewWidget, SIGNAL(filterEnabledStateChanged()), this, SLOT(markDocumentAsDirty()));

    connect(m_PipelineViewWidget, SIGNAL(preflightFinished(int)), this, SLOT(preflightDidFinish(int)));

    connect(m_PipelineViewWidget, &SVPipelineViewWidget::undoCommandCreated, m_TreeController, &PipelineTreeController::addUndoCommand);

    connect(m_PipelineViewWidget, &SVPipelineViewWidget::undoRequested, m_TreeController, &PipelineTreeController::undo);

    connect(m_PipelineViewWidget, SIGNAL(statusMessage(const QString&)), statusBar(), SLOT(showMessage(const QString&)));

    connect(m_PipelineViewWidget, SIGNAL(stdOutMessage(const QString&)), this, SLOT(addStdOutputMessage(const QString&)));

    connect(m_PipelineViewWidget, SIGNAL(deleteKeyPressed(SVPipelineViewWidget*)), this, SIGNAL(deleteKeyPressed(SVPipelineViewWidget*)));

    connect(m_PipelineViewWidget, &SVPipelineViewWidget::pipelineDropped, m_TreeController, &PipelineTreeController::addPipelineToModelFromFile);

    connect(getBookmarksToolboxWidget(), SIGNAL(updateStatusBar(const QString&)), this, SLOT(setStatusBarMessage(const QString&)));

    connect(m_TreeController, &PipelineTreeController::filtersAddedToModel, m_PipelineViewWidget, &SVPipelineViewWidget::addFiltersFromIndices);
  }
  else
  {
    // Connection to preflight both pipelines when moving filters between pipelines
    connect(model, &PipelineTreeModel::rowsMoved, [=] (const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row) {
      if (model->itemType(parent) == PipelineTreeItem::ItemType::Pipeline && model->itemType(destination) == PipelineTreeItem::ItemType::Pipeline)
      {
        m_TreeController->preflightPipeline(parent, model);

        if (destination != parent)
        {
          m_TreeController->preflightPipeline(destination, model);
        }
      }
    });

    // Connection that marks the document dirty whenever a filter is enabled/disabled
    connect(m_PipelineTreeView, &PipelineTreeView::filterEnabledStateChanged, this, &SIMPLView_UI::markDocumentAsDirty);

    // Connection that allows the view to call for a preflight, which gets picked up by the Pipeline Tree controller
    connect(m_PipelineTreeView, &PipelineTreeView::needsPreflight, m_TreeController, &PipelineTreeController::preflightPipeline);

    // Connection to update the active pipeline when the user decides to change it
    connect(m_PipelineTreeView, &PipelineTreeView::activePipelineChanged, m_TreeController, &PipelineTreeController::updateActivePipeline);

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
  PipelineTreeModel* model = getPipelineTreeModel();
  m_TreeController->addFilterToModel(filter, model);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLView_UI::addPipeline(const QString &pipelineName, bool setAsActive)
{
  PipelineTreeModel* model = getPipelineTreeModel();
  m_TreeController->addPipelineToModel(pipelineName, FilterPipeline::New(), model, setAsActive);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SIMPLView_UI::openPipeline(const QString& filePath)
{
  int result = m_TreeController->addPipelineToModelFromFile(filePath, getPipelineTreeModel());
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
void SIMPLView_UI::setLoadedPlugins(QVector<ISIMPLibPlugin*> plugins)
{
  m_LoadedPlugins = plugins;
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
void SIMPLView_UI::on_startPipelineBtn_clicked()
{
  if(startPipelineBtn->text().compare("Cancel Pipeline") == 0)
  {
    emit pipelineCanceled();

    startPipelineBtn->setText("Canceling...");

    // Enable FilterListToolboxWidget signals - resume adding filters
    getFilterListToolboxWidget()->blockSignals(false);

    // Enable FilterLibraryToolboxWidget signals - resume adding filters
    getFilterLibraryToolboxWidget()->blockSignals(false);

    return;
  }
  else if(startPipelineBtn->text().compare("Canceling...") == 0)
  {
    return;
  }

  startPipelineBtn->setText("Cancel Pipeline");
  startPipelineBtn->setIcon(QIcon(":/media_stop_white.png"));
  update();

  QMap<QWidget*, QTextEdit*>::iterator iter;
  for(iter = m_StdOutputTabMap.begin(); iter != m_StdOutputTabMap.end(); iter++)
  {
    QWidget* widget = iter.key();
    QTextEdit* textEdit = iter.value();
    delete textEdit;
    delete widget;
  }
  m_StdOutputTabMap.clear();

  if(m_WorkerThread != nullptr)
  {
    m_WorkerThread->wait(); // Wait until the thread is complete
    if(m_WorkerThread->isFinished() == true)
    {
      delete m_WorkerThread;
      m_WorkerThread = nullptr;
    }
  }
  m_WorkerThread = new QThread(); // Create a new Thread Resource

  // Clear out the Issues Table
  issuesWidget->clearIssues();

  // Ask the PipelineViewWidget to create a FilterPipeline Object
  // m_PipelineInFlight = m_PipelineViewWidget->getCopyOfFilterPipeline();
  m_PipelineInFlight = m_PipelineViewWidget->getFilterPipeline();

  addStdOutputMessage("<b>Preflight Pipeline.....</b>");
  // Give the pipeline one last chance to preflight and get all the latest values from the GUI
  int err = m_PipelineInFlight->preflightPipeline();
  if(err < 0)
  {
    m_PipelineInFlight = FilterPipeline::NullPointer();
    issuesWidget->displayCachedMessages();
    return;
  }
  addStdOutputMessage("    Preflight Results: 0 Errors");

  // Save each of the DataContainerArrays from each of the filters for when the pipeline is complete
  m_PreflightDataContainerArrays.clear();
  FilterPipeline::FilterContainerType filters = m_PipelineInFlight->getFilterContainer();
  for(FilterPipeline::FilterContainerType::size_type i = 0; i < filters.size(); i++)
  {
    m_PreflightDataContainerArrays.push_back(filters[i]->getDataContainerArray()->deepCopy(true));
  }

  // Save the preferences file NOW in case something happens
  writeSettings();

  // Connect signals and slots between SIMPLView_UI and each PipelineFilterWidget
  for(int i = 0; i < m_PipelineViewWidget->filterCount(); i++)
  {
    SVPipelineFilterWidget* w = dynamic_cast<SVPipelineFilterWidget*>(m_PipelineViewWidget->filterObjectAt(i));

    if(nullptr != w)
    {
      if(PipelineFilterObject::WidgetState::Disabled != w->getWidgetState())
      {
        w->toReadyState();
      }
      w->toRunningState();

      connect(m_WorkerThread, SIGNAL(finished()), w, SLOT(toStoppedState()));
    }
  }

  // Connect signals and slots between SIMPLView_UI and PipelineViewWidget
  connect(this, SIGNAL(pipelineStarted()), m_PipelineViewWidget, SLOT(toRunningState()));
  connect(this, SIGNAL(pipelineCanceled()), m_PipelineViewWidget, SLOT(toIdleState()));
  connect(this, SIGNAL(pipelineFinished()), m_PipelineViewWidget, SLOT(toIdleState()));

  // Connect signals and slots between SIMPLView_UI and SIMPLViewApplication
  connect(this, SIGNAL(pipelineStarted()), dream3dApp, SLOT(toPipelineRunningState()));
  connect(this, SIGNAL(pipelineCanceled()), dream3dApp, SLOT(toPipelineIdleState()));
  connect(this, SIGNAL(pipelineFinished()), dream3dApp, SLOT(toPipelineIdleState()));

  // Block FilterListToolboxWidget signals, so that we can't add filters to the view while running the pipeline
  getFilterListToolboxWidget()->blockSignals(true);

  // Block FilterLibraryToolboxWidget signals, so that we can't add filters to the view while running the pipeline
  getFilterLibraryToolboxWidget()->blockSignals(true);

  // Move the FilterPipeline object into the thread that we just created.
  m_PipelineInFlight->moveToThread(m_WorkerThread);

  // Allow the GUI to receive messages - We are only interested in the progress messages
  m_PipelineInFlight->addMessageReceiver(this);

  // Clear the Error table
  issuesWidget->clearIssues();

  /* Connect the signal 'started()' from the QThread to the 'run' slot of the
   * PipelineBuilder object. Since the PipelineBuilder object has been moved to another
   * thread of execution and the actual QThread lives in *this* thread then the
   * type of connection will be a Queued connection.
   */
  // When the thread starts its event loop, start the PipelineBuilder going
  connect(m_WorkerThread, SIGNAL(started()), m_PipelineInFlight.get(), SLOT(run()));

  // When the PipelineBuilder ends then tell the QThread to stop its event loop
  connect(m_PipelineInFlight.get(), SIGNAL(pipelineFinished()), m_WorkerThread, SLOT(quit()));

  // When the QThread finishes, tell this object that it has finished.
  connect(m_WorkerThread, SIGNAL(finished()), this, SLOT(pipelineDidFinish()));

  // Add in a connection to clear the Error/Warnings table when the thread starts
  //  connect(m_WorkerThread, SIGNAL(started()),
  //          issuesWidget, SLOT( clearIssues() ) );

  // Tell the Error/Warnings Table that we are finished and to display any cached messages
  connect(m_WorkerThread, SIGNAL(finished()), issuesWidget, SLOT(displayCachedMessages()));

  // If the use clicks on the "Cancel" button send a message to the PipelineBuilder object
  connect(this, SIGNAL(pipelineCanceled()), m_PipelineInFlight.get(), SLOT(cancelPipeline()), Qt::DirectConnection);

  emit pipelineStarted();
  m_WorkerThread->start();
  addStdOutputMessage("");
  addStdOutputMessage("<b>*************** PIPELINE STARTED ***************</b>");
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
    startPipelineBtn->setStyleSheet(getStartPipelineInProgressStyle(progValue));
  }
  else if(msg.getType() == PipelineMessage::MessageType::StatusMessageAndProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    startPipelineBtn->setStyleSheet(getStartPipelineInProgressStyle(progValue));

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

    if(stdOutDockWidget->isVisible() == false)
    {
      stdOutDockWidget->toggleViewAction()->toggle();
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
  if(m_PipelineInFlight->getCancel() == true)
  {
    addStdOutputMessage("<b>*************** PIPELINE CANCELED ***************</b>");
  }
  else
  {
    addStdOutputMessage("<b>*************** PIPELINE FINISHED ***************</b>");
  }
  addStdOutputMessage("");

  // Put back the DataContainerArray for each filter at the conclusion of running
  // the pipeline. this keeps the data browser current and up to date.
  FilterPipeline::FilterContainerType filters = m_PipelineInFlight->getFilterContainer();
  for(FilterPipeline::FilterContainerType::size_type i = 0; i < filters.size(); i++)
  {
    filters[i]->setDataContainerArray(m_PreflightDataContainerArrays[i]);
  }

  m_PipelineInFlight = FilterPipeline::NullPointer(); // This _should_ remove all the filters and deallocate them
  startPipelineBtn->setText("Start Pipeline");
  startPipelineBtn->setIcon(QIcon(":/media_play_white.png"));
  startPipelineBtn->setStyleSheet(getStartPipelineIdleStyle());

  // Re-enable FilterListToolboxWidget signals - resume adding filters
  getFilterListToolboxWidget()->blockSignals(false);

  // Re-enable FilterLibraryToolboxWidget signals - resume adding filters
  getFilterLibraryToolboxWidget()->blockSignals(false);

  QList<PipelineFilterObject*> selectedFilters = m_PipelineViewWidget->getSelectedFilterObjects();
  foreach(PipelineFilterObject* selectedFilter, selectedFilters)
  {
    m_PipelineViewWidget->setSelectedFilterObject(selectedFilter, Qt::ControlModifier);
  }

  emit pipelineFinished();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SIMPLView_UI::getStartPipelineIdleStyle()
{

  QFont font = QtSStyles::GetHumanLabelFont();
  QString fontString;
  QTextStream in(&fontString);
  in << "font: " << font.weight() << " " <<
#if defined(Q_OS_MAC)
      font.pointSize() - 2
#elif defined(Q_OS_WIN)
      font.pointSize() - 3
#else
      font.pointSize()
#endif
     << "pt \"" << font.family() << "\";"
     << "font-weight: bold;";

  QString cssStr;
  QTextStream ss(&cssStr);
  ss << "QPushButton:enabled { ";
  ss << "background-color: rgb(0, 118, 6);\n";
  ss << "color: white;\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;
  ss << "}\n\n";

  ss << "QPushButton:disabled { ";
  ss << "background-color: rgb(150, 150, 150);\n";
  ss << "color: rgb(190, 190, 190);\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;
  ss << "}";

  return cssStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SIMPLView_UI::getStartPipelineInProgressStyle(float percent)
{
  QFont font = QtSStyles::GetHumanLabelFont();
  QString fontString;
  QTextStream in(&fontString);
  in << "font: " << font.weight() << " " <<
#if defined(Q_OS_MAC)
      font.pointSize() - 2
#elif defined(Q_OS_WIN)
      font.pointSize() - 3
#else
      font.pointSize()
#endif
     << "pt \"" << font.family() << "\";"
     << "font-weight: bold;";

  QString cssStr;
  QTextStream ss(&cssStr);
  ss << "QPushButton { ";
  ss << QString("background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgb(29, 168, 29), stop:%1 rgb(29, 168, 29), stop:%2 rgb(0, 118, 6), stop:1 rgb(0, 118, 6));\n")
            .arg(percent)
            .arg(percent + 0.001);
  ss << "color: white;\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;

  ss << "}";

  return cssStr;
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
#ifdef SIMPLView_USE_QtWebEngine
  SIMPLViewUserManualDialog::LaunchHelpDialog(className);
#else
  QUrl helpURL = QtSHelpUrlGenerator::generateHTMLUrl(className.toLower());

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
#ifdef SIMPLView_USE_QtWebEngine
  SIMPLViewUserManualDialog::LaunchHelpDialog(helpURL);
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

  m_PipelineViewWidget->clearFilterWidgets();
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
  if(HideDockSetting::OnError == m_HideErrorTable)
  {
    issuesDockWidget->setHidden(!hasErrors);
  }

  if(HideDockSetting::OnError == m_HideStdOutput)
  {
    stdOutDockWidget->setHidden(!hasErrors);
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
  PipelineTreeModel* model = getPipelineTreeModel();
  QModelIndex pipelineIndex = m_TreeController->getActivePipelineIndex();
  if (pipelineIndex.isValid() == false)
  {
    startPipelineBtn->setDisabled(true);
    return;
  }

  if(err < 0 || model->rowCount(pipelineIndex) <= 0)
  {
    startPipelineBtn->setDisabled(true);
  }
  else
  {
    startPipelineBtn->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineTreeModel* SIMPLView_UI::getPipelineTreeModel()
{
  PipelineTreeModel* model;
  if (USE_PIPELINE_TREE_WIDGET == true)
  {
    model = m_PipelineTreeView->getPipelineTreeModel();
  }
  else
  {
    model = m_PipelineViewWidget->getPipelineTreeModel();
  }
  return model;
}
