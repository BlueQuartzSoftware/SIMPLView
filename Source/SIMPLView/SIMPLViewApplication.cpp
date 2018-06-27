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

#include "SIMPLView/SIMPLView.h"
#ifdef SIMPL_USE_QtWebEngine
#include "SVWidgetsLib/Widgets/SVUserManualDialog.h"
#endif
#include "SVWidgetsLib/QtSupport/QtSFileUtils.h"
#include "SVWidgetsLib/Dialogs/AboutPlugins.h"
#include "SVWidgetsLib/Dialogs/UpdateCheck.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckData.h"
#include "SVWidgetsLib/Dialogs/UpdateCheckDialog.h"
#include "SVWidgetsLib/Widgets/BookmarksToolboxWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SIMPLView/AboutSIMPLView.h"
#include "SIMPLView/SIMPLView_UI.h"
#include "SIMPLView/SIMPLViewVersion.h"
#include "SIMPLView/SIMPLViewConstants.h"

#include "BrandedStrings.h"

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
SIMPLViewApplication::SIMPLViewApplication(int& argc, char** argv)
: QApplication(argc, argv)
, m_ActiveWindow(nullptr)
, m_OpenDialogLastFilePath("")
, m_ShowSplash(true)
, m_SplashScreen(nullptr)
, m_minSplashTime(3)
{
  // Automatically check for updates at startup if the user has indicated that preference before
  checkForUpdatesAtStartup();

  // Initialize the Default Stylesheet
  SVStyle* style = SVStyle::Instance();
  QString defaultLoadedThemePath = BrandedStrings::DefaultStyleDirectory + "/" + BrandedStrings::DefaultLoadedTheme + ".json";
  style->loadStyleSheet(defaultLoadedThemePath);

  readSettings();

  // Create the default menu bar
  createDefaultMenuBar();

  // If on Mac, add custom actions to a dock menu
#if defined(Q_OS_MAC)
  createMacDockMenu();
#endif

  // Connection to update the recent files list on all windows when it changes
  QtSRecentFileList* recentsList = QtSRecentFileList::Instance();
  QObject::connect(recentsList, &QtSRecentFileList::fileListChanged, this, &SIMPLViewApplication::updateRecentFileList);

  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());
  QtSRecentFileList::Instance()->readList(prefs.data());
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
      if(fileName.endsWith("_debug.guiplugin", Qt::CaseSensitive))
#else
      if(fileName.endsWith(".guiplugin", Qt::CaseSensitive)            // We want ONLY Release plugins
         && !fileName.endsWith("_debug.guiplugin", Qt::CaseSensitive)) // so ignore these plugins
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
void SIMPLViewApplication::updateRecentFileList(const QString& file)
{
  Q_UNUSED(file)

  // Clear the Recent Items Menu
  m_MenuRecentFiles->clear();

  // Get the list from the static object
  QStringList filePaths = QtSRecentFileList::Instance()->fileList();
  for (int i = 0; i < filePaths.size(); i++)
  {
    QString filePath = filePaths[i];
    QAction* action = m_MenuRecentFiles->addAction(QtSRecentFileList::Instance()->parentAndFileName(filePath));
//    action->setVisible(true);
    connect(action, &QAction::triggered, [=] {
      dream3dApp->newInstanceFromFile(filePath);
    });
  }

  m_MenuRecentFiles->addSeparator();
  m_MenuRecentFiles->addAction(m_ActionClearRecentFiles);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenNewInstanceTriggered()
{
  SIMPLView_UI* newInstance = getNewSIMPLViewInstance();
  newInstance->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenOpenPipelineTriggered()
{
  QString proposedDir = m_OpenDialogLastFilePath;
  QString filePath = QFileDialog::getOpenFileName(nullptr, tr("Open Pipeline"), proposedDir, tr("Json File (*.json);;DREAM3D File (*.dream3d);;All Files (*.*)"));
  if(filePath.isEmpty())
  {
    return;
  }

  newInstanceFromFile(filePath);

  // Cache the last directory on old instance
  m_OpenDialogLastFilePath = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenClearRecentFilesTriggered()
{
  // Clear the actual list
  QtSRecentFileList* recents = QtSRecentFileList::Instance();
  recents->clear();

  // Write out the empty list
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());
  recents->writeList(prefs.data());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenClearSIMPLViewCacheTriggered()
{
  QMessageBox msgBox;

  QString title = QString("Clear %1 Cache").arg(BrandedStrings::ApplicationName);
  msgBox.setWindowTitle(title);

  QString text = QString("Clearing the %1 cache will clear the %1 window settings, and will restore %1 back to its default settings on the program's next run.").arg(BrandedStrings::ApplicationName);
  msgBox.setText(text);

  QString infoText = QString("Clear the %1 cache?").arg(BrandedStrings::ApplicationName);
  msgBox.setInformativeText(infoText);
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  int response = msgBox.exec();

  if(response == QMessageBox::Yes)
  {
    QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

    // Set a flag in the preferences file, so that we know that we are in "Clear Cache" mode
    prefs->setValue("Program Mode", QString("Clear Cache"));

    QMessageBox cacheClearedBox;
    QString title = QString("The cache has been cleared successfully. Please restart %1 for the changes to take effect.").arg(BrandedStrings::ApplicationName);

    cacheClearedBox.setText(title);
    QPushButton* restartNowBtn = cacheClearedBox.addButton("Restart Now", QMessageBox::YesRole);
    QPushButton* restartLaterBtn = cacheClearedBox.addButton("Restart Later", QMessageBox::NoRole);
    cacheClearedBox.setDefaultButton(restartLaterBtn);
    cacheClearedBox.setIcon(QMessageBox::Information);
    cacheClearedBox.exec();

    if (cacheClearedBox.clickedButton() == restartNowBtn)
    {
      listenExitApplicationTriggered();
      QProcess::startDetached(QApplication::applicationFilePath());
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenShowSIMPLViewHelpTriggered()
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
void SIMPLViewApplication::listenCheckForUpdatesTriggered()
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
void SIMPLViewApplication::checkForUpdatesAtStartup()
{
  UpdateCheck::SIMPLVersionData_t data = dream3dApp->FillVersionData();
  UpdateCheckDialog d(data);
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
void SIMPLViewApplication::versionCheckReply(UpdateCheckData* dataObj)
{
  UpdateCheck::SIMPLVersionData_t data = dream3dApp->FillVersionData();

  UpdateCheckDialog d(data);
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
void SIMPLViewApplication::listenDisplayPluginInfoDialogTriggered()
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
      listenExitApplicationTriggered();
      QProcess::startDetached(QApplication::applicationFilePath());
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::listenDisplayAboutSIMPLViewDialogTriggered()
{
  AboutSIMPLView d(nullptr);
  d.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::newInstanceFromFile(const QString& filePath)
{
  SIMPLView_UI* ui = getNewSIMPLViewInstance();
  ui->show();
  QString nativeFilePath = QDir::toNativeSeparators(filePath);
  QFileInfo fi(filePath);
  if(fi.exists())
  {
    ui->openPipeline(nativeFilePath);
  }
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
  if (instance->isActiveWindow())
  {
    m_ActiveWindow = instance;
  }
  else if (m_SIMPLViewInstances.size() == 1)
  {
    /* If the inactive signal got fired and there are no more windows,
     * this means that the last window has been closed. */
    m_ActiveWindow = nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLView_UI* SIMPLViewApplication::getActiveInstance()
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
  m_SIMPLViewInstances.removeAll(window);

#if defined(Q_OS_MAC)
#else
  if (m_SIMPLViewInstances.size() <= 0)
  {
    quit();
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SIMPLViewApplication::event(QEvent* event)
{
  #if defined(Q_OS_MAC)
  if (event->type() == QEvent::FileOpen)
  {
    QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
    QString filePath = openEvent->file();

    newInstanceFromFile(filePath);
  }
  #endif

  return QApplication::event(event);
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
void SIMPLViewApplication::writeSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  SVStyle* styles = SVStyle::Instance();
  QString themeFilePath = styles->getCurrentThemeFilePath();
  prefs->setValue("Theme File Path", themeFilePath);

  prefs->endGroup();

  BookmarksModel* model = BookmarksModel::Instance();
  model->writeBookmarksToPrefsFile();

  QtSRecentFileList::Instance()->writeList(prefs.data());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::readSettings()
{
  QSharedPointer<QtSSettings> prefs = QSharedPointer<QtSSettings>(new QtSSettings());

  prefs->beginGroup("Application Settings");

  SVStyle* styles = SVStyle::Instance();
  QString themeFilePath = prefs->value("Theme File Path", QString()).toString();
  QFileInfo fi(themeFilePath);
  if (themeFilePath.isEmpty() == false && BrandedStrings::LoadedThemeNames.contains(fi.baseName()))
  {
    styles->loadStyleSheet(themeFilePath);
  }

  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::createDefaultMenuBar()
{
  m_DefaultMenuBar = new QMenuBar();

  m_ActionCut = new QAction("Cut", m_DefaultMenuBar);
  m_ActionCut->setShortcut(QKeySequence::Cut);

  m_ActionCopy = new QAction("Copy", m_DefaultMenuBar);
  m_ActionCopy->setShortcut(QKeySequence::Copy);

  m_ActionPaste = new QAction("Paste", m_DefaultMenuBar);
  m_ActionPaste->setShortcut(QKeySequence::Paste);

  m_ActionClearPipeline = new QAction("Clear Pipeline", m_DefaultMenuBar);
  m_ActionClearPipeline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));

  m_ActionUndo = new QAction("Undo", m_DefaultMenuBar);
  m_ActionUndo->setShortcut(QKeySequence::Undo);

  m_ActionRedo = new QAction("Redo", m_DefaultMenuBar);
  m_ActionRedo->setShortcut(QKeySequence::Redo);

  m_ActionAddBookmark = new QAction("Add Bookmark", m_DefaultMenuBar);
  m_ActionAddBookmark->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));

  m_ActionAddBookmarkFolder = new QAction("Add Bookmark Folder", m_DefaultMenuBar);
  m_ActionAddBookmarkFolder->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));

  m_ActionClearBookmarks = new QAction("Clear Bookmarks", m_DefaultMenuBar);

  m_ActionSave = new QAction("Save", m_DefaultMenuBar);
  m_ActionSave->setShortcut(QKeySequence::Save);

  m_ActionSaveAs = new QAction("Save As...", m_DefaultMenuBar);
  m_ActionSaveAs->setShortcut(QKeySequence::SaveAs);

  m_MenuRecentFiles = new QMenu("Recent Files", m_DefaultMenuBar);
  m_MenuFile = new QMenu("File", m_DefaultMenuBar);
  m_MenuEdit = new QMenu("Edit", m_DefaultMenuBar);
  m_MenuView = new QMenu("View", m_DefaultMenuBar);
  m_MenuBookmarks = new QMenu("Bookmarks", m_DefaultMenuBar);
  m_MenuPipeline = new QMenu("Pipeline", m_DefaultMenuBar);
  m_MenuHelp = new QMenu("Help", m_DefaultMenuBar);
  m_MenuAdvanced = new QMenu("Advanced", m_DefaultMenuBar);

  m_ActionNew = new QAction("New...", m_DefaultMenuBar);
  m_ActionNew->setShortcut(QKeySequence::New);

  m_ActionOpen = new QAction("Open...", m_DefaultMenuBar);
  m_ActionOpen->setShortcut(QKeySequence::Open);

  m_ActionLoadTheme = new QAction("Load Theme", m_DefaultMenuBar);

  m_ActionSaveTheme = new QAction("Save Theme", m_DefaultMenuBar);

  m_ActionClearRecentFiles = new QAction("Clear Recent Files", m_DefaultMenuBar);

  m_ActionExit = new QAction("Exit " + QApplication::applicationName(), m_DefaultMenuBar);
  m_ActionExit->setShortcut(QKeySequence::Quit);

  m_ActionShowSIMPLViewHelp = new QAction(QApplication::applicationName() + " Help", m_DefaultMenuBar);
  m_ActionShowSIMPLViewHelp->setShortcut(QKeySequence::HelpContents);

  m_ActionAboutSIMPLView = new QAction("About " + QApplication::applicationName(), m_DefaultMenuBar);

  m_ActionCheckForUpdates = new QAction("Check For Updates", m_DefaultMenuBar);
  m_ActionCheckForUpdates->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));

  m_ActionPluginInformation = new QAction("Plugin Information", m_DefaultMenuBar);
  m_ActionPluginInformation->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

  m_ActionClearCache = new QAction("Clear Cache", m_DefaultMenuBar);

  m_ActionShowFilterList = new QAction("Filter List", m_DefaultMenuBar);
  m_ActionShowFilterLibrary = new QAction("Filter Library", m_DefaultMenuBar);
  m_ActionShowBookmarks = new QAction("Bookmarks", m_DefaultMenuBar);
  m_ActionShowPipeline = new QAction("Show Pipeline", m_DefaultMenuBar);
  m_ActionShowIssues = new QAction("Pipeline Issues", m_DefaultMenuBar);
  m_ActionShowConsole = new QAction("Pipeline Output", m_DefaultMenuBar);
  m_ActionShowDataBrowser = new QAction("Data Structure", m_DefaultMenuBar);

  m_MenuRecentFiles = new QMenu("Recent Files", m_DefaultMenuBar);

  connect(m_ActionNew, &QAction::triggered, this, &SIMPLViewApplication::listenNewInstanceTriggered);
  connect(m_ActionOpen, &QAction::triggered, this, &SIMPLViewApplication::listenOpenPipelineTriggered);
  connect(m_ActionExit, &QAction::triggered, this, &SIMPLViewApplication::listenExitApplicationTriggered);
  connect(m_ActionClearRecentFiles, &QAction::triggered, this, &SIMPLViewApplication::listenClearRecentFilesTriggered);
  connect(m_ActionAboutSIMPLView, &QAction::triggered, this, &SIMPLViewApplication::listenDisplayAboutSIMPLViewDialogTriggered);
  connect(m_ActionCheckForUpdates, &QAction::triggered, this, &SIMPLViewApplication::listenCheckForUpdatesTriggered);
  connect(m_ActionShowSIMPLViewHelp, &QAction::triggered, this, &SIMPLViewApplication::listenShowSIMPLViewHelpTriggered);
  connect(m_ActionPluginInformation, &QAction::triggered, this, &SIMPLViewApplication::listenDisplayPluginInfoDialogTriggered);
  connect(m_ActionClearCache, &QAction::triggered, this, &SIMPLViewApplication::listenClearSIMPLViewCacheTriggered);

  m_ActionAddBookmark->setDisabled(true);
  m_ActionAddBookmarkFolder->setDisabled(true);
  m_ActionSave->setDisabled(true);
  m_ActionSaveAs->setDisabled(true);

  m_ActionClearPipeline->setDisabled(true);
  m_ActionCut->setDisabled(true);
  m_ActionCopy->setDisabled(true);
  m_ActionPaste->setDisabled(true);
  m_ActionUndo->setDisabled(true);
  m_ActionRedo->setDisabled(true);

  m_ActionShowFilterList->setDisabled(true);
  m_ActionShowFilterLibrary->setDisabled(true);
  m_ActionShowBookmarks->setDisabled(true);
  m_ActionShowPipeline->setDisabled(true);
  m_ActionShowIssues->setDisabled(true);
  m_ActionShowConsole->setDisabled(true);
  m_ActionShowDataBrowser->setDisabled(true);

  // Create File Menu
  m_DefaultMenuBar->addMenu(m_MenuFile);
  m_MenuFile->addAction(m_ActionNew);
  m_MenuFile->addAction(m_ActionOpen);
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_ActionSave);
  m_MenuFile->addAction(m_ActionSaveAs);
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_MenuRecentFiles->menuAction());
  m_MenuFile->addSeparator();
  m_MenuFile->addAction(m_ActionExit);

  // Create Edit Menu
  m_DefaultMenuBar->addMenu(m_MenuEdit);
  m_MenuEdit->addAction(m_ActionUndo);
  m_MenuEdit->addAction(m_ActionRedo);
  m_MenuEdit->addSeparator();
  m_MenuEdit->addAction(m_ActionCut);
  m_MenuEdit->addAction(m_ActionCopy);
  m_MenuEdit->addAction(m_ActionPaste);

  // Create View Menu
  m_DefaultMenuBar->addMenu(m_MenuView);

  QStringList themeNames = BrandedStrings::LoadedThemeNames;
  if (themeNames.size() > 1)  // We are not counting the Default theme when deciding whether or not to add the theme menu
  {
    m_ThemeActionGroup = new QActionGroup(this);
    m_MenuThemes = createThemeMenu(m_ThemeActionGroup, m_DefaultMenuBar);

    m_MenuView->addMenu(m_MenuThemes);

    m_MenuView->addSeparator();
  }

  m_MenuView->addAction(m_ActionShowFilterList);
  m_MenuView->addAction(m_ActionShowFilterLibrary);
  m_MenuView->addAction(m_ActionShowBookmarks);
  m_MenuView->addAction(m_ActionShowPipeline);
  m_MenuView->addAction(m_ActionShowIssues);
  m_MenuView->addAction(m_ActionShowConsole);
  m_MenuView->addAction(m_ActionShowDataBrowser);

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
QMenu* SIMPLViewApplication::createThemeMenu(QActionGroup* actionGroup, QWidget* parent)
{
  SVStyle* style = SVStyle::Instance();

  QMenu* menuThemes = new QMenu("Themes", parent);

  QString themePath = ":/SIMPL/StyleSheets/Default.json";
  QAction* action = menuThemes->addAction("Default", [=] {
    style->loadStyleSheet(themePath);
  });
  action->setCheckable(true);
  if(themePath == style->getCurrentThemeFilePath())
  {
    action->setChecked(true);
  }
  actionGroup->addAction(action);

  QStringList themeNames = BrandedStrings::LoadedThemeNames;
  for (int i = 0; i < themeNames.size(); i++)
  {
    QString themePath = BrandedStrings::DefaultStyleDirectory + QDir::separator() + themeNames[i] + ".json";
    QAction* action = menuThemes->addAction(themeNames[i], [=] {
      style->loadStyleSheet(themePath);
    });
    action->setCheckable(true);
    if(themePath == style->getCurrentThemeFilePath())
    {
      action->setChecked(true);
    }
    actionGroup->addAction(action);
  }

  return menuThemes;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SIMPLViewApplication::createMacDockMenu()
{
#if defined(Q_OS_MAC)
  m_DockMenu = new QMenu();

  m_DockMenu->addAction(m_ActionNew);
  m_DockMenu->addAction(m_ActionOpen);
  m_DockMenu->addSeparator();
  m_DockMenu->addAction(m_ActionShowSIMPLViewHelp);
  m_DockMenu->addSeparator();
  m_DockMenu->addAction(m_ActionCheckForUpdates);
  m_DockMenu->addSeparator();
  m_DockMenu->addAction(m_ActionPluginInformation);

  m_DockMenu->setAsDockMenu();
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMenu* SIMPLViewApplication::getRecentFilesMenu()
{
  return m_MenuRecentFiles;
}
