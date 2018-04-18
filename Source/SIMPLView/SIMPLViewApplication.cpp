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

#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include <ctime>
#include <iostream>

#include <QtCore/QPluginLoader>
#include <QtCore/QProcess>
#include <QtCore/QThread>

#include <QtGui/QBitmap>
#include <QtGui/QBitmap>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QIcon>
#include <QtGui/QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QSplashScreen>

#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/Filtering/QMetaObjectUtilities.h"
#include "SIMPLib/Plugin/PluginManager.h"
#include "SIMPLib/Plugin/PluginProxy.h"
#include "SIMPLib/SIMPLibVersion.h"

#include "SVWidgetsLib/QtSupport/QtSApplicationAboutBoxDialog.h"
#include "SVWidgetsLib/QtSupport/QtSDocServer.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/Widgets/SVPipelineViewWidget.h"

#include "SIMPLView/SIMPLView.h"
#ifdef SIMPL_USE_QtWebEngine
#include "SVWidgetsLib/Widgets/SVUserManualDialog.h"
#endif
#include "SVWidgetsLib/QtSupport/QtSFileUtils.h"
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/SIMPLViewToolbox.h"
#include "SVWidgetsLib/Widgets/SVPipelineFilterWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"

#include "SIMPLView/AboutSIMPLView.h"
#include "SIMPLView/SIMPLView_UI.h"
#include "SIMPLView/SIMPLViewVersion.h"

#include "BrandedStrings.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLViewApplication::SIMPLViewApplication(int& argc, char** argv)
: QApplication(argc, argv)
, m_ActiveWindow(nullptr)
, m_OpenDialogLastFilePath("")
, m_ShowSplash(true)
, m_SplashScreen(nullptr)
, m_minSplashTime(3)
{
  // Connection to update the recent files list on all windows when it changes
  QtSRecentFileList* recentsList = QtSRecentFileList::instance();
  connect(recentsList, SIGNAL(fileListChanged(const QString&)), this, SLOT(updateRecentFileList(const QString&)));

  // If on Mac, add custom actions to a dock menu
#if defined(Q_OS_MAC)
  m_DockMenu = QSharedPointer<QMenu>(createMacDockMenu());
  m_DockMenu.data()->setAsDockMenu();
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLViewApplication::~SIMPLViewApplication()
{
  delete this->m_SplashScreen;
  this->m_SplashScreen = nullptr;

  for(int i = 0; i < m_PluginLoaders.size(); i++)
  {
    delete m_PluginLoaders[i];
  }

  writeSettings();

  QtSSettings prefs;
  if(prefs.value("Program Mode", QString("")) == "Clear Cache")
  {
    prefs.clear();
    prefs.setValue("Program Mode", QString("Standard"));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void delay(int seconds)
{
  QTime dieTime = QTime::currentTime().addSecs(seconds);
  while(QTime::currentTime() < dieTime)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLViewApplication::initialize(int argc, char* argv[])
{

  Q_UNUSED(argc)
  Q_UNUSED(argv)
  QApplication::setApplicationVersion(SIMPLib::Version::Complete());

  readSettings();

  // Assume we are launching on the main screen.
  float pixelRatio = qApp->screens().at(0)->devicePixelRatio();

  QString name(":/splash/branded_splash");
  if(pixelRatio >= 2)
  {
    name.append("@2x");
  }

  name.append(".png");

  // Create and show the splash screen as the main window is being created.
  QPixmap pixmap(name);

  this->m_SplashScreen = new QSplashScreen(pixmap);
  this->m_SplashScreen->show();

  // start timer;
  std::clock_t startClock = std::clock();

  QDir dir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
  dir.cdUp();
  dir.cd("Plugins");

#elif defined(Q_OS_LINUX)
  if(!dir.cd("Plugins"))
  {
    dir.cdUp();
    dir.cd("Plugins");
  }
#elif defined(Q_OS_WIN)
  dir.cdUp();
  dir.cd("Plugins");
#endif
  QApplication::addLibraryPath(dir.absolutePath());

  QMetaObjectUtilities::RegisterMetaTypes();

  // Load application plugins.
  QVector<ISIMPLibPlugin*> plugins = loadPlugins();

  // give GUI components time to update before the mainwindow is shown
  QApplication::instance()->processEvents();
  if(m_ShowSplash)
  {
    //   delay(1);
    // if official release, enforce the minimum duration for splash screen
    QString releaseType = QString::fromLatin1(SIMPLViewProj_RELEASE_TYPE);
    if(releaseType.compare("Official") == 0)
    {
      double splashDuration = (std::clock() - startClock) / (double)CLOCKS_PER_SEC;
      if(splashDuration < m_minSplashTime)
      {
        QString msg = QObject::tr("");
        this->m_SplashScreen->showMessage(msg, Qt::AlignVCenter | Qt::AlignRight, Qt::white);

        unsigned long extendedDuration = static_cast<unsigned long>((m_minSplashTime - splashDuration) * 1000);
        QThread::msleep(extendedDuration);
      }
    }
    this->m_SplashScreen->finish(nullptr);
  }
  QApplication::instance()->processEvents();

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
  qDebug() << "Loading " << BrandedStrings::ApplicationName << " Plugins....";
  QString thePath;

#if defined(Q_OS_WIN)
  if(aPluginDir.cd("Plugins"))
  {
    thePath = aPluginDir.absolutePath();
    pluginDirs << thePath;
  }
#elif defined(Q_OS_MAC)
  // Look to see if we are inside an .app package or inside the 'tools' directory
  if(aPluginDir.dirName() == "MacOS")
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
  if(aPluginDir.dirName() == "bin")
  {
    aPluginDir.cdUp();
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
  if(aPluginDir.cd("Plugins"))
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
    if(aPluginDir.cd("Plugins"))
    {
      thePath = aPluginDir.absolutePath();
      pluginDirs << thePath;
      aPluginDir.cdUp(); // Move back up a directory level
      int no_error = chdir(aPluginDir.absolutePath().toLatin1().constData());
      if(no_error < 0)
      {
        qDebug() << "Could not set the working directory.";
      }
    }
  }
#endif

  QByteArray pluginEnvPath = qgetenv("SIMPL_PLUGIN_PATH");
  qDebug() << "SIMPL_PLUGIN_PATH:" << pluginEnvPath;

  char sep = ';';
#if defined(Q_OS_WIN)
  sep = ':';
#endif
  QList<QByteArray> envPaths = pluginEnvPath.split(sep);
  foreach(QByteArray envPath, envPaths)
  {
    if(envPath.size() > 0)
    {
      pluginDirs << QString::fromLatin1(envPath);
    }
  }

  int dupes = pluginDirs.removeDuplicates();
  qDebug() << "Removed " << dupes << " duplicate Plugin Paths";
  QStringList pluginFilePaths;

  foreach(QString pluginDirString, pluginDirs)
  {
    qDebug() << "Plugin Directory being Searched: " << pluginDirString;
    aPluginDir = QDir(pluginDirString);
    foreach(QString fileName, aPluginDir.entryList(QDir::Files))
    {
//   qDebug() << "File: " << fileName() << "\n";
#ifdef QT_DEBUG
      if(fileName.endsWith("_debug.plugin", Qt::CaseSensitive))
#else
      if(fileName.endsWith(".plugin", Qt::CaseSensitive)            // We want ONLY Release plugins
         && !fileName.endsWith("_debug.plugin", Qt::CaseSensitive)) // so ignore these plugins
#endif
      {
        pluginFilePaths << aPluginDir.absoluteFilePath(fileName);
        // qWarning(aPluginDir.absoluteFilePath(fileName).toLatin1(), "%s");
        // qDebug() << "Adding " << aPluginDir.absoluteFilePath(fileName)() << "\n";
      }
    }
  }

  FilterManager* filterManager = FilterManager::Instance();
  FilterWidgetManager* fwm = FilterWidgetManager::Instance();

  // THIS IS A VERY IMPORTANT LINE: It will register all the known filters in the dream3d library. This
  // will NOT however get filters from plugins. We are going to have to figure out how to compile filters
  // into their own plugin and load the plugins from a command line.
  filterManager->RegisterKnownFilters(filterManager);

  PluginManager* pluginManager = PluginManager::Instance();
  QList<PluginProxy::Pointer> proxies = AboutPlugins::readPluginCache();
  QMap<QString, bool> loadingMap;
  for(QList<PluginProxy::Pointer>::iterator nameIter = proxies.begin(); nameIter != proxies.end(); nameIter++)
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
    if(plugin)
    {
      ISIMPLibPlugin* ipPlugin = qobject_cast<ISIMPLibPlugin*>(plugin);
      if(ipPlugin)
      {
        QString pluginName = ipPlugin->getPluginFileName();
        if(loadingMap.value(pluginName, true) == true)
        {
          QString msg = QObject::tr("Loading Plugin %1  ").arg(fileName);
          this->m_SplashScreen->showMessage(msg, Qt::AlignVCenter | Qt::AlignRight, Qt::white);
          // ISIMPLibPlugin::Pointer ipPluginPtr(ipPlugin);
          ipPlugin->registerFilterWidgets(fwm);
          ipPlugin->registerFilters(filterManager);
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
      m_SplashScreen->hide();
      QString message("The plugin did not load with the following error\n\n");
      message.append(loader->errorString());
      message.append("\n\n");
      message.append("Possible causes include missing libraries that plugin depends on.");
      QMessageBox box(QMessageBox::Critical, tr("Plugin Load Error"), tr(message.toStdString().c_str()));
      box.setStandardButtons(QMessageBox::Ok | QMessageBox::Default);
      box.setDefaultButton(QMessageBox::Ok);
      box.setWindowFlags(box.windowFlags() | Qt::WindowStaysOnTopHint);
      box.exec();
      m_SplashScreen->show();
      delete loader;
    }
  }

  return pluginManager->getPluginsVector();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::newInstanceFromFile(const QString& filePath)
{
  SIMPLView_UI* ui = getNewSIMPLViewInstance();
  QString nativeFilePath = QDir::toNativeSeparators(filePath);
  QFileInfo fi(filePath);
  if(fi.exists())
  {
    ui->openPipeline(nativeFilePath);

    QtSRecentFileList* list = QtSRecentFileList::instance();
    list->addFile(filePath);
  }
  ui->show();
  return ui;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::getNewSIMPLViewInstance()
{
  PluginManager* pluginManager = PluginManager::Instance();
  QVector<ISIMPLibPlugin*> plugins = pluginManager->getPluginsVector();

  // Create new SIMPLView instance
  SIMPLView_UI* newInstance = new SIMPLView_UI(nullptr);
  newInstance->setLoadedPlugins(plugins);
  newInstance->setAttribute(Qt::WA_DeleteOnClose);
  newInstance->setWindowTitle("[*]Untitled Pipeline - " + BrandedStrings::ApplicationName);

  if (m_ActiveWindow)
  {
    newInstance->move(m_ActiveWindow->x() + 45, m_ActiveWindow->y() + 45);
  }

  m_ActiveWindow = newInstance;

  connect(newInstance, SIGNAL(dream3dWindowChangedState(SIMPLView_UI*)), this, SLOT(dream3dWindowChanged(SIMPLView_UI*)));

  return newInstance;
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
void SIMPLViewApplication::listenExitApplicationTriggered()
{
  bool shouldReallyClose = true;
  for(int i = 0; i < m_SIMPLViewInstances.size(); i++)
  {
    SIMPLView_UI* dream3dWindow = m_SIMPLViewInstances[i];
    if(nullptr != dream3dWindow)
    {
      if(dream3dWindow->close() == false)
      {
        shouldReallyClose = false;
      }
    }
  }

  if(shouldReallyClose == true)
  {
    quit();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::dream3dWindowChanged(SIMPLView_UI* instance)
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
QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> SIMPLViewApplication::getClipboard()
{
  return m_Clipboard;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::setClipboard(QPair<QList<SVPipelineFilterWidget*>, SVPipelineViewWidget*> clipboard)
{
  // Properly delete what is currently stored on the clipboard
  for(int i = 0; i < m_Clipboard.first.size(); i++)
  {
    delete m_Clipboard.first[i];
  }

  // Assign new clipboard values to the clipboard
  m_Clipboard = clipboard;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::writeSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::readSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenuBar* SIMPLViewApplication::createDefaultMenuBar()
{
  m_DefaultMenuBar = new QMenuBar();

  m_ActionAddBookmark->setDisabled(true);
  m_ActionAddBookmarkFolder->setDisabled(true);
  m_ActionSave->setDisabled(true);
  m_ActionSaveAs->setDisabled(true);

  m_ActionClearPipeline->setDisabled(true);
  m_ActionCut->setDisabled(true);
  m_ActionCopy->setDisabled(true);
  m_ActionPaste->setDisabled(true);

  // Create File Menu
  m_DefaultMenuBar->addMenu(m_MenuFile);
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
  m_DefaultMenuBar->addMenu(m_MenuEdit);
  m_MenuEdit->addSeparator();
  m_MenuEdit->addAction(m_ActionCut);
  m_MenuEdit->addAction(m_ActionCopy);
  m_MenuEdit->addAction(m_ActionPaste);

  // Create Bookmarks Menu
  m_DefaultMenuBar->addMenu(m_MenuBookmarks);
  m_MenuBookmarks->addAction(m_ActionAddBookmark);
  m_MenuBookmarks->addSeparator();
  m_MenuBookmarks->addAction(m_ActionAddBookmarkFolder);

  // Create Pipeline Menu
  m_DefaultMenuBar->addMenu(m_MenuPipeline);
  m_MenuPipeline->addAction(m_ActionClearPipeline);

  // Create Help Menu
  m_DefaultMenuBar->addMenu(m_MenuHelp);
  m_MenuHelp->addAction(m_ActionShowSIMPLViewHelp);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addAction(m_ActionCheckForUpdates);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addMenu(m_MenuAdvanced);
  m_MenuAdvanced->addAction(m_ActionClearCache);
  m_MenuAdvanced->addSeparator();
  m_MenuAdvanced->addAction(m_ActionClearBookmarks);
  m_MenuHelp->addSeparator();
  m_MenuHelp->addAction(m_ActionAboutSIMPLView);
  m_MenuHelp->addAction(m_ActionPluginInformation);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenu* SIMPLViewApplication::createMacDockMenu()
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
