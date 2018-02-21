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

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtGui/QFontDatabase>

#include "BrandedStrings.h"
#include "MacSIMPLViewApplication.h"
#include "SIMPLView.h"
#include "SIMPLView_UI.h"
#include "StandardSIMPLViewApplication.h"

#ifdef Q_WS_X11
#include <QPlastiqueStyle>
#endif

#include <clocale>

#ifdef SIMPL_USE_QtWebEngine
#include "SVWidgetsLib/QtSupport/QtSDocServer.h"
#endif

int main(int argc, char* argv[])
{
#ifdef Q_OS_X11
  // Using motif style gives us test failures (and its ugly).
  // Using cleanlooks style gives us errors when using valgrind (Trolltech's bug #179200)
  // let's just use plastique for now
  //QApplication::setStyle(new QPlastiqueStyle);
#endif

  QFileInfo fi(argv[0]);
  QString absPathExe = fi.absolutePath();
  QString cwd = QDir::currentPath();
  qDebug() << "argv[0]0 = " << absPathExe;
  qDebug() << "        cwd: " << cwd;

#ifdef Q_OS_WIN
  // Some where Visual Studio wants to set the Current Working Directory (cwd)
  // to the subfolder BUILD/Applications/SIMPLView instead of our true binary
  // directory where everything is built. This wreaks havoc on the prebuilt
  // pipelines not being able to find the data folder. This _should_ fix things
  // for debugging and hopefully NOT effect any type of release

  if (absPathExe != cwd)
  {
    QDir::setCurrent(absPathExe);
    qDebug() << "setting cwd: " << absPathExe;
  }
  cwd = QDir::currentPath();
  qDebug() << "        cwd: " << cwd;
#endif

//#if !defined (Q_OS_MAC)
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
//#endif

#if defined (Q_OS_MAC)
  MacSIMPLViewApplication qtapp(argc, argv);
#else
  StandardSIMPLViewApplication qtapp(argc, argv);
#endif

  QCoreApplication::setOrganizationDomain(BrandedStrings::OrganizationDomain);
  QCoreApplication::setOrganizationName(BrandedStrings::OrganizationName);
  QCoreApplication::setApplicationName(BrandedStrings::ApplicationName);

#if defined (Q_OS_MAC)
  dream3dApp->setQuitOnLastWindowClosed(false);
#endif

  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  setlocale(LC_NUMERIC, "C");
  QFontDatabase::addApplicationFont(":/fonts/FiraSans-Regular.ttf");

  if (!qtapp.initialize(argc, argv))
  {
    return 1;
  }

  // Open pipeline if SIMPLView was opened from a compatible file
  if (argc == 2)
  {
    char* two = argv[1];
    QString filePath = QString::fromLatin1(two);
    if (!filePath.isEmpty())
    {
      qtapp.newInstanceFromFile(filePath, true, true);
    }
  }
  else
  {
    SIMPLView_UI* ui = qtapp.getNewSIMPLViewInstance();
    ui->show();
  }

#if defined (Q_OS_MAC)
  qtapp.initializeDummyDockWidgetActions();
#endif

#ifdef SIMPL_USE_QtWebEngine
  QtSDocServer* docServer = QtSDocServer::Instance();
#endif

  int err = qtapp.exec();
  return err;
}

