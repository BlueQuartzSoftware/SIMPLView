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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QOperatingSystemVersion>

#include <QtGui/QFontDatabase>

#include <QtWidgets/QFileDialog>

#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/SVWidgetsLib.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

#include "SIMPLView.h"
#include "SIMPLViewApplication.h"
#include "SIMPLView_UI.h"
#include "StyleSheetEditor.h"

#include "BrandedStrings.h"

#ifdef Q_WS_X11
#include <QPlastiqueStyle>
#endif

#include <clocale>

#ifdef SIMPL_USE_MKDOCS
#include "SVWidgetsLib/QtSupport/QtSDocServer.h"
#endif

#ifdef SIMPL_EMBED_PYTHON
#include "SIMPLib/Python/PythonLoader.h"
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void InitFonts(const QStringList& fontList)
{
  int fontID(-1);

  for(QStringList::const_iterator constIterator = fontList.constBegin(); constIterator != fontList.constEnd(); ++constIterator)
  {
    QFile res(*constIterator);
    // qDebug() << "font path: " << res.fileName();
    if(!res.open(QIODevice::ReadOnly))
    {
      qDebug() << "ERROR opening font resource: " << res.fileName();
    }

    else
    {
      fontID = QFontDatabase::addApplicationFontFromData(res.readAll());
      // qDebug() << "loading font Id " << fontID;
      if(fontID == -1)
      {
        qDebug() << "ERROR loading font id: " << fontID;
      }
      res.close();
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void InitStyleSheetEditor()
{
  // SVStyle* style = SVStyle::Instance();

  StyleSheetEditor* styleSheetEditor = new StyleSheetEditor(nullptr);
  styleSheetEditor->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#if defined(__APPLE__)
  if( (QOperatingSystemVersion::current().majorVersion() == 10 && QOperatingSystemVersion::current().minorVersion() == 16) 
        || QOperatingSystemVersion::current().majorVersion() > 10)
  {
    qputenv("QT_MAC_WANTS_LAYER","1");
  }

#endif

#ifdef DREAM3D_ANACONDA
  {
    constexpr const char k_QT_PLUGIN_PATH[] = "QT_PLUGIN_PATH";
    constexpr const char k_PYTHONHOME[] = "PYTHONHOME";
    QString qtPluginPath = qgetenv(k_QT_PLUGIN_PATH);
    QString condaPrefix = qgetenv("CONDA_PREFIX");
    if(qtPluginPath.isEmpty() && !condaPrefix.isEmpty())
    {
#ifdef _WIN32
      QString absoluteQtPluginPath = QString("%1/Library/Plugins").arg(condaPrefix);
#else
      QString absoluteQtPluginPath = QString("%1/plugins").arg(condaPrefix);
#endif
      if(QDir(absoluteQtPluginPath).exists())
      {
        qputenv(k_QT_PLUGIN_PATH, absoluteQtPluginPath.toLocal8Bit());
      }
    }

    QString pythonHome = qgetenv(k_PYTHONHOME);
    if(pythonHome.isEmpty() && !condaPrefix.isEmpty())
    {
      qputenv(k_PYTHONHOME, condaPrefix.toLocal8Bit());
    }
  }
#endif

#ifdef SIMPL_EMBED_PYTHON
  qputenv("DREAM3D_PLUGINS_LOADED", "1");
#endif

#ifdef Q_OS_X11
  // Using motif style gives us test failures (and its ugly).
  // Using cleanlooks style gives us errors when using valgrind (Trolltech's bug #179200)
  // let's just use plastique for now
  // QApplication::setStyle(new QPlastiqueStyle);
#endif

  QFileInfo fi(argv[0]);
  QString absPathExe = fi.absolutePath();
  QString cwd = QDir::currentPath();
  qDebug() << "argv[0]: " << absPathExe;
  qDebug() << "    cwd: " << cwd;

#ifdef Q_OS_WIN
  // Somewhere Visual Studio wants to set the Current Working Directory (cwd)
  // to the subfolder BUILD/Applications/SIMPLView instead of our true binary
  // directory where everything is built. This wreaks havoc on the prebuilt
  // pipelines not being able to find the data folder. This _should_ fix things
  // for debugging and hopefully NOT effect any type of release

  if(absPathExe != cwd)
  {
    QDir::setCurrent(absPathExe);
    qDebug() << "setting cwd: " << absPathExe;
  }
  cwd = QDir::currentPath();
  qDebug() << "        cwd: " << cwd;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QCoreApplication::setOrganizationDomain(BrandedStrings::OrganizationDomain);
  QCoreApplication::setOrganizationName(BrandedStrings::OrganizationName);
  QCoreApplication::setApplicationName(BrandedStrings::ApplicationName);

  SIMPLViewApplication qtapp(argc, argv);

#ifdef SIMPL_EMBED_PYTHON
  bool hasPythonHome = PythonLoader::checkPythonHome();
  bool enablePython = hasPythonHome;

  if(!hasPythonHome)
  {
    QMessageBox::StandardButton result =
        QMessageBox::warning(nullptr, "Warning", "\"PYTHONHOME\" not set. This environment variable must be set for embedded Python to work. Would you like to set it now?",
                             QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);

    if(result == QMessageBox::StandardButton::Yes)
    {
      QString pythonHome = QFileDialog::getExistingDirectory(nullptr, "Set PYTHONHOME");

      if(pythonHome.isEmpty() && !PythonLoader::setPythonHome(pythonHome.toStdString()))
      {
        QMessageBox::critical(nullptr, "Error", "Failed to set \"PYTHONHOME\".");
      }
      else
      {
        enablePython = true;
      }
    }
  }

  if(!enablePython)
  {
    QMessageBox::warning(nullptr, "Warning", "Python filters disabled.");
  }

  // Python interpreter must be created before calling SIMPLViewApplication::initialize since it will try to load Python filters
  PythonLoader::ScopedInterpreter interpreter_guard{enablePython};

  // Release Python GIL to allow the main and worker threads to lock as needed
  PythonLoader::GILScopedRelease gil_release_guard{enablePython};
#endif

  if(!qtapp.initialize(argc, argv))
  {
    return 1;
  }

#ifdef SIMPL_EMBED_PYTHON
  qtapp.setPythonGUIEnabled(enablePython);
  if(enablePython)
  {
    PythonLoader::loadPluginFilters();
    qtapp.reloadPythonFilters();
    PythonLoader::addToPythonPath(PythonLoader::defaultSIMPLPythonLibPath());
  }
#endif

#if defined(Q_OS_MAC)
  dream3dApp->setQuitOnLastWindowClosed(false);
#endif

  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  setlocale(LC_NUMERIC, "C");

  // Load the default font from SIMPL
  QStringList fontList;
  fontList << QString(":/SIMPL/fonts/FiraSans-Regular.ttf") << QString(":/SIMPL/fonts/Lato-Regular.ttf") << QString(":/SIMPL/fonts/Lato-Black.ttf") << QString(":/SIMPL/fonts/Lato-BlackItalic.ttf")
           << QString(":/SIMPL/fonts/Lato-Bold.ttf") << QString(":/SIMPL/fonts/Lato-BoldItalic.ttf") << QString(":/SIMPL/fonts/Lato-Hairline.ttf") << QString(":/SIMPL/fonts/Lato-HairlineItalic.ttf")
           << QString(":/SIMPL/fonts/Lato-Italic.ttf") << QString(":/SIMPL/fonts/Lato-Light.ttf") << QString(":/SIMPL/fonts/Lato-LightItalic.ttf");

  InitFonts(fontList);

  // Init any extra fonts that are needed by specialized versions of SIMPLView
  InitFonts(BrandedStrings::ExtraFonts);

#ifdef SIMPLView_USE_STYLESHEETEDITOR
  InitStyleSheetEditor();
#endif

  // Open pipeline if SIMPLView was opened from a compatible file
  if(argc == 2)
  {
    char* two = argv[1];
    QString filePath = QString::fromLatin1(two);
    if(!filePath.isEmpty())
    {
      qtapp.newInstanceFromFile(filePath);
    }
  }
  else
  {
    SIMPLView_UI* ui = qtapp.getNewSIMPLViewInstance();
    ui->show();
  }

#ifdef SIMPL_USE_MKDOCS
  QtSDocServer::Instance();
#endif

  int err = SIMPLViewApplication::exec();
  return err;
}
