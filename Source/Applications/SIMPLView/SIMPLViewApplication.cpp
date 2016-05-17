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
#include "SIMPLViewApplication.h"

#if ! defined(_MSC_VER)
#include <unistd.h>
#endif

#include <iostream>

#include <QtCore/QPluginLoader>
#include <QtCore/QProcess>

#include <QtWidgets/QFileDialog>

#include <QtGui/QDesktopServices>
#include <QtGui/QBitmap>
#include <QtGui/QClipboard>

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Utilities/QMetaObjectUtilities.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"

#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSHelpUrlGenerator.h"
#include "SVWidgetsLib/QtSupport/QtSApplicationAboutBoxDialog.h"
#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"

#include "Applications/SIMPLView/SIMPLView.h"
#ifdef SIMPLView_USE_QtWebEngine
#include "Applications/Common/SIMPLViewUserManualDialog.h"
#endif
#include "SVWidgetsLib/Dialogs/UpdateCheckDialog.h"
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/Widgets/DSplashScreen.h"

#include "Applications/SIMPLView/SIMPLView_UI.h"
#include "Applications/SIMPLView/AboutSIMPLView.h"
#include "Applications/SIMPLView/SIMPLViewConstants.h"
#include "Applications/SIMPLView/SIMPLViewToolbox.h"
#include "Applications/SIMPLView/SIMPLViewMenuItems.h"

#include "Applications/SIMPLView/SIMPLViewVersion.h"

#include "BrandedStrings.h"

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_SIMPLViewApplication.cpp"

namespace Detail {

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void fillVersionData(UpdateCheck::SIMPLVersionData_t &data)
  {
        data.complete = SIMPLView::Version::Complete();
        data.major = SIMPLView::Version::Major();
        data.minor = SIMPLView::Version::Minor();
        data.patch = SIMPLView::Version::Patch();
        data.package = SIMPLView::Version::Package();
        data.revision = SIMPLView::Version::Revision();
        data.packageComplete = SIMPLView::Version::PackageComplete();
        data.buildDate = SIMPLView::Version::BuildDate();
  }
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLViewApplication::SIMPLViewApplication(int& argc, char** argv) :
  QApplication(argc, argv),
  m_ActiveWindow(NULL),
  m_PreviousActiveWindow(NULL),
  m_OpenDialogLastDirectory(""),
  show_splash(true),
  Splash(NULL),
  m_ContextMenu(new QMenu(NULL))
{
  // Create the toolbox
  m_Toolbox = SIMPLViewToolbox::Instance();
  m_Toolbox->setWindowTitle(BrandedStrings::ApplicationName + " Toolbox");

  connect(m_Toolbox, SIGNAL(toolboxChangedState()), this, SLOT(toolboxWindowChanged()));

  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  connect(menuItems, SIGNAL(clipboardHasChanged(bool)), this, SLOT(updatePasteState(bool)));

  // Connection to update the recent files list on all windows when it changes
  QtSRecentFileList* recentsList = QtSRecentFileList::instance();
  connect(recentsList, SIGNAL(fileListChanged(const QString&)),
          this, SLOT(updateRecentFileList(const QString&)));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLViewApplication::~SIMPLViewApplication()
{
  delete this->Splash;
  this->Splash = NULL;

  writeSettings();

  QtSSettings prefs;
  if (prefs.value("Program Mode", QString("")) == "Clear Cache")
  {
    prefs.clear();
    prefs.setValue("First Run", QVariant(false));

    prefs.setValue("Program Mode", QString("Standard"));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void delay(int seconds)
{
  QTime dieTime = QTime::currentTime().addSecs(seconds);
  while( QTime::currentTime() < dieTime )
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLViewApplication::initialize(int argc, char* argv[])
{
  QApplication::setApplicationVersion(SIMPLib::Version::Complete());

  readSettings();

  // Create and show the splash screen as the main window is being created.
  QPixmap pixmap(QLatin1String(":/splash/branded_splash.png"));
  this->Splash = new DSplashScreen(pixmap);
  this->Splash->setMask(pixmap.createMaskFromColor(QColor(Qt::transparent)));
  this->Splash->show();

  QDir dir(QApplication::applicationDirPath());

#if defined (Q_OS_MAC)
  dir.cdUp();
  dir.cd("Plugins");

#elif defined (Q_OS_LINUX)
  if (! dir.cd("Plugins"))
  {
    dir.cdUp();
    dir.cd("Plugins");
  }
#elif defined (Q_OS_WIN)
  dir.cdUp();
  dir.cd("Plugins");
#endif
  QApplication::addLibraryPath(dir.absolutePath());

  QMetaObjectUtilities::RegisterMetaTypes();

  // Load application plugins.
  QVector<ISIMPLibPlugin*> plugins = loadPlugins();

  // give GUI components time to update before the mainwindow is shown
  QApplication::instance()->processEvents();
  if (show_splash)
  {
//   delay(1);
    this->Splash->finish(NULL);
  }
  QApplication::instance()->processEvents();

  // Read the toolbox settings and update the filter list
  m_Toolbox->readSettings();
  m_Toolbox->getFilterListWidget()->updateFilterList(true);

  // Set the "Show Toolbox" action to the correct state
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  menuItems->getActionShowToolbox()->setChecked(m_Toolbox->isVisible());

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<ISIMPLibPlugin*> SIMPLViewApplication::loadPlugins()
{
  QStringList pluginDirs;
  pluginDirs << applicationDirPath();

  QDir aPluginDir = QDir(applicationDirPath());
  qDebug() << "Loading SIMPLView Plugins....";
  QString thePath;

#if defined(Q_OS_WIN)
  if (aPluginDir.cd("Plugins") )
  {
    thePath = aPluginDir.absolutePath();
    pluginDirs << thePath;
  }
#elif defined(Q_OS_MAC)
  // Look to see if we are inside an .app package or inside the 'tools' directory
  if (aPluginDir.dirName() == "MacOS")
  {
    aPluginDir.cdUp();
    thePath = aPluginDir.absolutePath() + "/Plugins";
    qDebug() << "  Adding Path " << thePath;
    pluginDirs << thePath;
    aPluginDir.cdUp();
    aPluginDir.cdUp();
    // We need this because Apple (in their infinite wisdom) changed how the current working directory is set in OS X 10.9 and above. Thanks Apple.
    chdir(aPluginDir.absolutePath().toLatin1().constData());
  }
  if (aPluginDir.dirName() == "tools")
  {
    aPluginDir.cdUp();
    // thePath = aPluginDir.absolutePath() + "/Plugins";
    // qDebug() << "  Adding Path " << thePath;
    // m_PluginDirs << thePath;
    // We need this because Apple (in their infinite wisdom) changed how the current working directory is set in OS X 10.9 and above. Thanks Apple.
    chdir(aPluginDir.absolutePath().toLatin1().constData());
  }
  // aPluginDir.cd("Plugins");
  thePath = aPluginDir.absolutePath() + "/Plugins";
  qDebug() << "  Adding Path " << thePath;
  pluginDirs << thePath;

  // This is here for Xcode compatibility
#ifdef CMAKE_INTDIR
  aPluginDir.cdUp();
  thePath = aPluginDir.absolutePath() + "/Plugins/" + CMAKE_INTDIR;
  pluginDirs << thePath;
#endif
#else
  // We are on Linux - I think
  // Try the current location of where the application was launched from which is
  // typically the case when debugging from a build tree
  if (aPluginDir.cd("Plugins"))
  {
    thePath = aPluginDir.absolutePath();
    pluginDirs << thePath;
    aPluginDir.cdUp(); // Move back up a directory level
  }

  if(thePath.isEmpty())
  {
    // Now try moving up a directory which is what should happen when running from a
    // proper distribution of SIMPLView
    aPluginDir.cdUp();
    if (aPluginDir.cd("Plugins"))
    {
      thePath = aPluginDir.absolutePath();
      pluginDirs << thePath;
      aPluginDir.cdUp(); // Move back up a directory level
      int no_error = chdir(aPluginDir.absolutePath().toLatin1().constData());
      if( no_error < 0)
      {
        qDebug() << "Could not set the working directory.";
      }
    }
  }
#endif

  int dupes = pluginDirs.removeDuplicates();
  qDebug() << "Removed " << dupes << " duplicate Plugin Paths";
  QStringList pluginFilePaths;

  foreach (QString pluginDirString, pluginDirs)
  {
    qDebug() << "Plugin Directory being Searched: " << pluginDirString;
    aPluginDir = QDir(pluginDirString);
    foreach (QString fileName, aPluginDir.entryList(QDir::Files))
    {
      //   qDebug() << "File: " << fileName() << "\n";
#ifdef QT_DEBUG
      if (fileName.endsWith("_debug.plugin", Qt::CaseSensitive))
#else
      if (fileName.endsWith( ".plugin", Qt::CaseSensitive) // We want ONLY Release plugins
          && ! fileName.endsWith("_debug.plugin", Qt::CaseSensitive)) // so ignore these plugins
#endif
      {
        pluginFilePaths << aPluginDir.absoluteFilePath(fileName);
        //qWarning(aPluginDir.absoluteFilePath(fileName).toLatin1(), "%s");
        //qDebug() << "Adding " << aPluginDir.absoluteFilePath(fileName)() << "\n";
      }
    }
  }

  FilterManager* fm = FilterManager::Instance();
  FilterWidgetManager*  fwm = FilterWidgetManager::Instance();

  // THIS IS A VERY IMPORTANT LINE: It will register all the known filters in the dream3d library. This
  // will NOT however get filters from plugins. We are going to have to figure out how to compile filters
  // into their own plugin and load the plugins from a command line.
  fm->RegisterKnownFilters(fm);

  PluginManager* pluginManager = PluginManager::Instance();
  QList<PluginProxy::Pointer> proxies = AboutPlugins::readPluginCache();
  QMap<QString, bool> loadingMap;
  for (QList<PluginProxy::Pointer>::iterator nameIter = proxies.begin(); nameIter != proxies.end(); nameIter++)
  {
    PluginProxy::Pointer proxy = *nameIter;
    loadingMap.insert(proxy->getPluginName(), proxy->getEnabled());
  }

  // Now that we have a sorted list of plugins, go ahead and load them all from the
  // file system and add each to the toolbar and menu
  foreach(QString path, pluginFilePaths)
  {
    qDebug() << "Plugin Being Loaded:" << path;
    QApplication::instance()->processEvents();
    QPluginLoader* loader = new QPluginLoader(path);
    QFileInfo fi(path);
    QString fileName = fi.fileName();
    QObject* plugin = loader->instance();
    qDebug() << "    Pointer: " << plugin << "\n";
    if (plugin)
    {
      ISIMPLibPlugin* ipPlugin = qobject_cast<ISIMPLibPlugin*>(plugin);
      if (ipPlugin)
      {
        QString pluginName = ipPlugin->getPluginName();
        if (loadingMap.value(pluginName, true) == true)
        {
          QString msg = QObject::tr("Loading Plugin %1").arg(fileName);
          this->Splash->showMessage(msg);
          //ISIMPLibPlugin::Pointer ipPluginPtr(ipPlugin);
          ipPlugin->registerFilterWidgets(fwm);
          ipPlugin->registerFilters(fm);
          ipPlugin->setDidLoad(true);
        }
        else
        {
          ipPlugin->setDidLoad(false);
        }

        ipPlugin->setLocation(path);
        pluginManager->addPlugin(ipPlugin);
      }
      m_PluginLoaders.push_back(loader);
    }
    else
    {
      Splash->hide();
      QString message("The plugin did not load with the following error\n");
      message.append(loader->errorString());
      QMessageBox box(QMessageBox::Critical, tr("SIMPLView Plugin Load Error"), tr(message.toStdString().c_str()));
      box.setStandardButtons(QMessageBox::Ok | QMessageBox::Default);
      box.setDefaultButton(QMessageBox::Ok);
      box.setWindowFlags(box.windowFlags() | Qt::WindowStaysOnTopHint);
      box.exec();
      Splash->show();
      delete loader;
    }
  }

  return pluginManager->getPluginsVector();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLViewApplication::event(QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
    QString filePath = openEvent->file();

    newInstanceFromFile(filePath, true, true);

    return true;
  }

  return QApplication::event(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::updateRecentFileList(const QString& file)
{
  // This should never be executed
  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::openRecentFile()
{
  QAction* action = qobject_cast<QAction*>(sender());

  if (action)
  {
    QString filePath = action->data().toString();

    newInstanceFromFile(filePath, true, true);

    // Add file path to the recent files list for both instances
    QtSRecentFileList* list = QtSRecentFileList::instance();
    list->addFile(filePath);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionClearRecentFiles_triggered()
{
  // This should never be executed
  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::addFilter(const QString &className, int index)
{
  if (NULL != m_PreviousActiveWindow)
  {
    m_PreviousActiveWindow->getPipelineViewWidget()->addFilter(className, index, true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionCloseToolbox_triggered()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  toolbox->close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionNew_triggered()
{
  SIMPLView_UI* newInstance = getNewSIMPLViewInstance();
  newInstance->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionOpen_triggered()
{
  QString proposedDir = m_OpenDialogLastDirectory;
  QString filePath = QFileDialog::getOpenFileName(NULL, tr("Open Pipeline"),
    proposedDir, tr("Json File (*.json);;SIMPLView File (*.dream3d);;Text File (*.txt);;Ini File (*.ini);;All Files (*.*)"));
  if (filePath.isEmpty())
  {
    return;
  }

  newInstanceFromFile(filePath, true, true);

  // Cache the last directory on old instance
  m_OpenDialogLastDirectory = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionSave_triggered()
{
  if (NULL != m_ActiveWindow)
  {
    m_ActiveWindow->savePipeline();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionSaveAs_triggered()
{
  if (NULL != m_ActiveWindow)
  {
    m_ActiveWindow->savePipelineAs();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionAddBookmark_triggered()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();

  toolbox->setCurrentTab(SIMPLViewToolbox::Bookmarks);

  BookmarksToolboxWidget* bookmarksToolboxWidget = toolbox->getBookmarksWidget();

  if (NULL != bookmarksToolboxWidget)
  {
    QString proposedDir = m_OpenDialogLastDirectory;
    QList<QString> newPrefPaths;

    newPrefPaths = QFileDialog::getOpenFileNames(toolbox, tr("Choose Pipeline File(s)"),
      proposedDir, tr("Json File (*.json);;SIMPLView File (*.dream3d);;Text File (*.txt);;Ini File (*.ini);;All Files (*.*)"));
    if (true == newPrefPaths.isEmpty()) { return; }

    QModelIndex parent = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->currentIndex();

    if (parent.isValid() == false)
    {
      parent = QModelIndex();
    }

    for (int i = 0; i < newPrefPaths.size(); i++)
    {
      QString newPrefPath = newPrefPaths[i];
      newPrefPath = QDir::toNativeSeparators(newPrefPath);
      bookmarksToolboxWidget->addBookmark(newPrefPath, parent);
    }

    if (newPrefPaths.size() > 0)
    {
      // Cache the directory from the last path added
      m_OpenDialogLastDirectory = newPrefPaths[newPrefPaths.size() - 1];
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionNewFolder_triggered()
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();

  toolbox->setCurrentTab(SIMPLViewToolbox::Bookmarks);

  BookmarksModel* model = BookmarksModel::Instance();
  BookmarksToolboxWidget* bookmarksToolboxWidget = toolbox->getBookmarksWidget();

  QModelIndex parent = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->currentIndex();

  if (parent.isValid() == false)
  {
    parent = QModelIndex();
  }

  QString name = "New Folder";

  bookmarksToolboxWidget->addTreeItem(parent, name, QIcon(":/folder_blue.png"), "", model->rowCount(parent), true, true, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionShowToolbox_triggered(bool visible)
{
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  QAction* actionShowToolbox = menuItems->getActionShowToolbox();

  actionShowToolbox->blockSignals(true);
  toolbox->blockSignals(true);

  actionShowToolbox->setChecked(visible);
  toolbox->setVisible(visible);

  actionShowToolbox->blockSignals(false);
  toolbox->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionShowIssues_triggered(bool visible)
{
  if (NULL != m_ActiveWindow)
  {
    QAction* actionShowIssues = qobject_cast<QAction*>(sender());
    IssuesDockWidget* issuesDockWidget = m_ActiveWindow->getIssuesDockWidget();

    if (NULL != actionShowIssues && NULL != issuesDockWidget)
    {
      m_ActiveWindow->updateAndSyncDockWidget(actionShowIssues, issuesDockWidget, visible);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionShowStdOutput_triggered(bool visible)
{
  if (NULL != m_ActiveWindow)
  {
    QAction* actionShowStdOutput = qobject_cast<QAction*>(sender());
    StandardOutputDockWidget* stdOutputDockWidget = m_ActiveWindow->getStandardOutputDockWidget();

    if (NULL != actionShowStdOutput && NULL != stdOutputDockWidget)
    {
      m_ActiveWindow->updateAndSyncDockWidget(actionShowStdOutput, stdOutputDockWidget, visible);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionShowSIMPLViewHelp_triggered()
{
  // Generate help page
  QUrl helpURL = QtSHelpUrlGenerator::generateHTMLUrl("index");
#ifdef SIMPLView_USE_QtWebEngine
  SIMPLViewUserManualDialog::LaunchHelpDialog(helpURL);
#else
  bool didOpen = QDesktopServices::openUrl(helpURL);
  if (false == didOpen)
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
void SIMPLViewApplication::on_actionAboutSIMPLView_triggered()
{
  AboutSIMPLView d(NULL);
  d.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UpdateCheck::SIMPLVersionData_t SIMPLViewApplication::FillVersionData()
{
  UpdateCheck::SIMPLVersionData_t data;
  data.complete = SIMPLView::Version::Complete();
  data.major = SIMPLView::Version::Major();
  data.minor = SIMPLView::Version::Minor();
  data.patch = SIMPLView::Version::Patch();
  data.package = SIMPLView::Version::Package();
  data.revision = SIMPLView::Version::Revision();
  data.packageComplete = SIMPLView::Version::PackageComplete();
  data.buildDate = SIMPLView::Version::BuildDate();
  return data;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionCheckForUpdates_triggered()
{

  UpdateCheck::SIMPLVersionData_t data;
  Detail::fillVersionData(data);
  UpdateCheckDialog d(data, NULL);

  //d.setCurrentVersion(SIMPLib::Version::Complete());
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
void SIMPLViewApplication::on_actionPluginInformation_triggered()
{
  AboutPlugins dialog(NULL);
  dialog.exec();

  // Write cache on exit
  dialog.writePluginCache();

  /* If any of the load checkboxes were changed, display a dialog warning
  * the user that they must restart SIMPLView to see the changes.
  */
  if (dialog.getLoadPreferencesDidChange() == true)
  {
    QMessageBox msgBox;
    msgBox.setText("SIMPLView must be restarted to allow these changes to take effect.");
    msgBox.setInformativeText("Restart?");
    msgBox.setWindowTitle("Restart Needed");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int choice = msgBox.exec();

    if (choice == QMessageBox::Yes)
    {
      for (int i = 0; i < m_SIMPLViewInstances.size(); i++)
      {
        SIMPLView_UI* dream3d = m_SIMPLViewInstances[i];
        dream3d->close();
      }

      // Restart SIMPLView
      QProcess::startDetached(QApplication::applicationFilePath());
      dream3dApp->quit();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionRenameBookmark_triggered()
{
    BookmarksTreeView* bookmarksTreeView = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView();
    QModelIndex index = bookmarksTreeView->currentIndex();
    bookmarksTreeView->edit(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionRemoveBookmark_triggered()
{
  BookmarksToolboxWidget* bookmarksToolboxWidget = m_Toolbox->getBookmarksWidget();

  BookmarksModel* model = BookmarksModel::Instance();

  QModelIndexList indexList = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->selectionModel()->selectedRows(BookmarksItem::Name);

  indexList = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->filterOutDescendants(indexList);

  if (indexList.size() <= 0)
  {
    return;
  }

  QList<QPersistentModelIndex> persistentList;
  for (int i = 0; i < indexList.size(); i++)
  {
    persistentList.push_back(indexList[i]);
  }

  QModelIndex singleIndex = model->index(indexList[0].row(), BookmarksItem::Name, indexList[0].parent());

  QMessageBox msgBox;
  if (indexList.size() > 1)
  {
    msgBox.setWindowTitle("Remove Bookmark Items");
    msgBox.setText("Are you sure that you want to remove these bookmark items? The original bookmark files will not be removed.");
  }
  else if (model->flags(singleIndex).testFlag(Qt::ItemIsDropEnabled) == false)
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

  if (ret == QMessageBox::Yes)
  {
    for (int i = 0; i < persistentList.size(); i++)
    {
      QModelIndex nameIndex = model->index(persistentList[i].row(), BookmarksItem::Name, persistentList[i].parent());
      QString name = nameIndex.data().toString();

      //Remove bookmark from the SIMPLView interface
      model->removeRow(persistentList[i].row(), persistentList[i].parent());
    }

    // Write these changes out to the preferences file
    emit bookmarksToolboxWidget->fireWriteSettings();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionShowBookmarkInFileSystem_triggered()
{
  BookmarksModel* model = BookmarksModel::Instance();
  BookmarksToolboxWidget* bookmarksToolboxWidget = m_Toolbox->getBookmarksWidget();
  BookmarksTreeView* bookmarksTreeView = bookmarksToolboxWidget->getBookmarksTreeView();

  QModelIndex index = bookmarksTreeView->currentIndex();
  if (index.isValid())
  {
    QString pipelinePath = model->index(index.row(), BookmarksItem::Path, index.parent()).data().toString();

    QFileInfo pipelinePathInfo(pipelinePath);
    QString pipelinePathDir = pipelinePathInfo.path();

    QString s("file://");
#if defined(Q_OS_WIN)
    s = s + "/"; // Need the third slash on windows because file paths start with a drive letter
#elif defined(Q_OS_MAC)

#else
    // We are on Linux - I think

#endif
    s = s + pipelinePathDir;
    QDesktopServices::openUrl(s);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionCut_triggered()
{
  if (NULL != m_ActiveWindow)
  {
    PipelineViewWidget* widget = m_ActiveWindow->getPipelineViewWidget();
    QList<PipelineFilterWidget*> filterWidgets = widget->getSelectedFilterWidgets();

    FilterPipeline::Pointer pipeline = FilterPipeline::New();
    for (int i = 0; i < filterWidgets.size(); i++)
    {
      pipeline->pushBack(filterWidgets[i]->getFilter());
    }

    QString jsonString = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Pipeline");

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(jsonString);

    widget->cutFilterWidgets(filterWidgets);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionCopy_triggered()
{
  if (NULL != m_ActiveWindow)
  {
    FilterPipeline::Pointer pipeline = FilterPipeline::New();
    QList<PipelineFilterWidget*> filterWidgets = m_ActiveWindow->getPipelineViewWidget()->getSelectedFilterWidgets();
    for (int i = 0; i < filterWidgets.size(); i++)
    {
      pipeline->pushBack(filterWidgets[i]->getFilter());
    }

    QString json = JsonFilterParametersWriter::WritePipelineToString(pipeline, "Copy - Pipeline");
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(json);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionPaste_triggered()
{
  <<<<<<<<<<
  if (NULL != m_ActiveWindow)
  {
    PipelineViewWidget* viewWidget = m_ActiveWindow->getPipelineViewWidget();

    QClipboard* clipboard = QApplication::clipboard();
    QString jsonString = clipboard->text();

    FilterPipeline::Pointer pipeline = JsonFilterParametersReader::ReadPipelineFromString(jsonString);
    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();

    viewWidget->pasteFilters(container);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_pipelineViewWidget_deleteKeyPressed(PipelineViewWidget* widget)
{
  if (m_ActiveWindow)
  {
    QList<PipelineFilterWidget*> selectedWidgets = widget->getSelectedFilterWidgets();
    if (selectedWidgets.size() > 0)
    {
      if (m_ShowFilterWidgetDeleteDialog == true)
      {
        QMessageBox msgBox;
        msgBox.setParent(m_ActiveWindow);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("Are you sure that you want to delete these filters?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        QCheckBox* chkBox = new QCheckBox("Do not show me this again");
        msgBox.setCheckBox(chkBox);
        int ret = msgBox.exec();

        m_ShowFilterWidgetDeleteDialog = !chkBox->isChecked();

        if (ret == QMessageBox::Yes)
        {
          widget->removeFilterWidgets(selectedWidgets, true);
        }
      }
      else
      {
        widget->removeFilterWidgets(selectedWidgets, true);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_pipelineFilterWidget_contextMenuRequested(const QPoint& pos)
{
  PipelineFilterWidget* filterWidget = dynamic_cast<PipelineFilterWidget*>(sender());

  if (NULL != filterWidget && NULL != filterWidget->getFilter())
  {
    m_ContextMenu->clear();

    SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

    QAction* actionCut = menuItems->getActionCut();
    QAction* actionCopy = menuItems->getActionCopy();
    QAction* actionPaste = menuItems->getActionPaste();

    m_ContextMenu->addAction(actionCut);
    m_ContextMenu->addAction(actionCopy);
    m_ContextMenu->addAction(actionPaste);

    m_ContextMenu->addSeparator();

    QAction* actionLaunchHelp = new QAction(m_ContextMenu);
    actionLaunchHelp->setObjectName(QString::fromUtf8("actionLaunchHelp"));
    actionLaunchHelp->setText(QApplication::translate("SIMPLView_UI", "Filter Help", 0));
    connect(actionLaunchHelp, SIGNAL(triggered()),
      filterWidget, SLOT(launchHelpForItem()));

    m_ContextMenu->addAction(actionLaunchHelp);
    m_ContextMenu->exec(QCursor::pos());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_bookmarksDockContextMenuRequested(const QPoint& pos)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  BookmarksTreeView* bookmarksTreeView = m_PreviousActiveWindow->getBookmarksToolboxWidget()->getBookmarksTreeView();

  QModelIndex index = bookmarksTreeView->indexAt(pos);

  QPoint mapped;
  if (index.isValid())
  {
    // Note: We must map the point to global from the viewport to
    // account for the header.
    mapped = bookmarksTreeView->viewport()->mapToGlobal(pos);
  }
  else
  {
    index = QModelIndex();
    mapped = bookmarksTreeView->mapToGlobal(pos);
  }

  BookmarksModel* model = BookmarksModel::Instance();

  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();
  QAction* actionRenameBookmark = menuItems->getActionRenameBookmark();
  QAction* actionRemoveBookmark = menuItems->getActionRemoveBookmark();
  QAction* actionLocateFile = menuItems->getActionLocateFile();
  QAction* actionShowBookmarkInFileSystem = menuItems->getActionShowBookmarkInFileSystem();

  QModelIndexList indexList = m_Toolbox->getBookmarksWidget()->getBookmarksTreeView()->selectionModel()->selectedRows(BookmarksItem::Name);

  QMenu menu;
  if (index.isValid() == false)
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
    QModelIndex actualIndex = model->index(index.row(), BookmarksItem::Path, index.parent());
    QString path = actualIndex.data().toString();
    if (indexList.size() > 1)
    {
      actionRemoveBookmark->setText("Remove Items");
      menu.addAction(actionRemoveBookmark);
    }
    else if (path.isEmpty() == false)
    {
      bool itemHasErrors = model->data(actualIndex, Qt::UserRole).value<bool>();
      if (itemHasErrors == true)
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
        actionRenameBookmark->setText("Rename Bookmark");
        menu.addAction(actionRenameBookmark);

        {
          QAction* separator = new QAction(this);
          separator->setSeparator(true);
          menu.addAction(separator);
        }

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
    else if (path.isEmpty())
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

  menu.exec(mapped);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionClearBookmarks_triggered()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Clear SIMPLView Bookmarks");
  msgBox.setText("Are you sure that you want to clear all SIMPLView bookmarks?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int response = msgBox.exec();

  if (response == QMessageBox::Yes)
  {
    BookmarksModel* model = BookmarksModel::Instance();
    if (model->isEmpty() == false)
    {
      model->removeRows(0, model->rowCount(QModelIndex()));
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionClearCache_triggered()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("Clear SIMPLView Cache");
  msgBox.setText("Clearing the SIMPLView cache will clear the SIMPLView window settings, and will restore SIMPLView back to its default settings on the program's next run.");
  msgBox.setInformativeText("Clear the SIMPLView cache?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int response = msgBox.exec();

  if (response == QMessageBox::Yes)
  {
    QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

    // Set a flag in the preferences file, so that we know that we are in "Clear Cache" mode
    prefs->setValue("Program Mode", QString("Clear Cache"));

    if (NULL != m_ActiveWindow)
    {
      m_ActiveWindow->getPipelineViewWidget()->getStatusBar()->showMessage("The cache has been cleared successfully.  Please restart SIMPLView.");
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionCloseWindow_triggered()
{
  m_ActiveWindow->close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::on_actionExit_triggered()
{
  // Write Toolbox Settings
  SIMPLViewToolbox* toolbox = SIMPLViewToolbox::Instance();
  toolbox->writeSettings();

  bool shouldReallyClose = true;
  for (int i = 0; i<m_SIMPLViewInstances.size(); i++)
  {
    SIMPLView_UI* dream3dWindow = m_SIMPLViewInstances[i];
    if (NULL != dream3dWindow)
    {
      if (dream3dWindow->close() == false)
      {
        shouldReallyClose = false;
      }
    }
  }

  if (shouldReallyClose == true)
  {
    quit();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::bookmarkSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
  BookmarksModel* model = BookmarksModel::Instance();
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  QAction* actionAddBookmark = menuItems->getActionAddBookmark();
  QAction* actionNewFolder = menuItems->getActionNewFolder();

  if (model->index(current.row(), BookmarksItem::Path, current.parent()).data().toString().isEmpty() == true)
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
void SIMPLViewApplication::dream3dWindowChanged(SIMPLView_UI *instance)
{
  // This should never be executed
  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::toolboxWindowChanged()
{
  // This should never be executed
  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::getActiveWindow()
{
  return m_ActiveWindow;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<SIMPLView_UI*> SIMPLViewApplication::getSIMPLViewInstances()
{
  return m_SIMPLViewInstances;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::registerSIMPLViewWindow(SIMPLView_UI* window)
{
  m_SIMPLViewInstances.push_back(window);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::unregisterSIMPLViewWindow(SIMPLView_UI* window)
{
  // This should never be executed
  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::toPipelineRunningState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  menuItems->getActionClearPipeline()->setDisabled(true);

  SIMPLView_UI* runningInstance = qobject_cast<SIMPLView_UI*>(sender());
  if (NULL != runningInstance)
  {
    m_CurrentlyRunningInstances.insert(runningInstance);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::toPipelineIdleState()
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();
  menuItems->getActionClearPipeline()->setEnabled(true);

  SIMPLView_UI* runningInstance = qobject_cast<SIMPLView_UI*>(sender());
  if (NULL != runningInstance)
  {
    m_CurrentlyRunningInstances.remove(runningInstance);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::newInstanceFromFile(const QString& filePath, const bool& setOpenedFilePath, const bool& addToRecentFiles)
{
  SIMPLView_UI* ui = getNewSIMPLViewInstance();
  QString nativeFilePath = QDir::toNativeSeparators(filePath);

  ui->getPipelineViewWidget()->openPipeline(nativeFilePath, 0, setOpenedFilePath, true);

  QtSRecentFileList* list = QtSRecentFileList::instance();
  if (addToRecentFiles == true)
  {
    // Add file to the recent files list
    list->addFile(filePath);
  }
  ui->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::getNewSIMPLViewInstance()
{
  PluginManager* pluginManager = PluginManager::Instance();
  QVector<ISIMPLibPlugin*> plugins = pluginManager->getPluginsVector();

  // Create new SIMPLView instance
  SIMPLView_UI* newInstance = new SIMPLView_UI(NULL);
  newInstance->setLoadedPlugins(plugins);
  newInstance->setAttribute(Qt::WA_DeleteOnClose);
  newInstance->setWindowTitle("[*]Untitled Pipeline - " + BrandedStrings::ApplicationName);

  if (NULL != m_ActiveWindow)
  {
    newInstance->move(m_ActiveWindow->x() + 45, m_ActiveWindow->y() + 45);
  }
  else if (NULL != m_PreviousActiveWindow)
  {
    newInstance->move(m_PreviousActiveWindow->x() + 45, m_PreviousActiveWindow->y() + 45);
  }

  m_ActiveWindow = newInstance;

  connect(newInstance, SIGNAL(dream3dWindowChangedState(SIMPLView_UI*)), this, SLOT(dream3dWindowChanged(SIMPLView_UI*)));

  // Check if this is the first run of SIMPLView
  newInstance->checkFirstRun();

  return newInstance;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::setActiveWindow(SIMPLView_UI* instance)
{
  m_ActiveWindow = instance;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLViewApplication::isCurrentlyRunning(SIMPLView_UI* instance)
{
  return m_CurrentlyRunningInstances.contains(instance);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPair<QList<PipelineFilterWidget*>, PipelineViewWidget*> SIMPLViewApplication::getClipboard()
{
  return m_Clipboard;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::setClipboard(QPair<QList<PipelineFilterWidget*>, PipelineViewWidget*> clipboard)
{
  // Properly delete what is currently stored on the clipboard
  for (int i = 0; i < m_Clipboard.first.size(); i++)
  {
    delete m_Clipboard.first[i];
  }

  // Assign new clipboard values to the clipboard
  m_Clipboard = clipboard;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::updatePasteState(bool canPaste)
{
  SIMPLViewMenuItems* menuItems = SIMPLViewMenuItems::Instance();

  if (NULL != m_ActiveWindow)
  {
    menuItems->getActionPaste()->setEnabled(canPaste);
  }
  else
  {
    menuItems->getActionPaste()->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::writeSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  prefs->setValue("Show 'Delete Filter Widgets' Dialog", m_ShowFilterWidgetDeleteDialog);

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::readSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  m_ShowFilterWidgetDeleteDialog = prefs->value("Show 'Delete Filter Widgets' Dialog", QVariant(true)).toBool();

  prefs->endGroup();
}






