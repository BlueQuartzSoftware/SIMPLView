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

#include "DevHelper.h"

#include <iostream>

#include <QtCore/QtDebug>
#include <QtWidgets/QDesktopWidget>

#include "SVWidgetsLib/QtSupport/QtSApplicationAboutBoxDialog.h"

#include "DevHelper/FilterMaker.h"
#include "DevHelper/PluginMaker.h"

#include "SIMPLView/License/DevHelperLicenseFiles.h"

#include "HelpWidget.h"

enum WidgetIndices
{
  PLUGIN_MAKER,
  FILTER_MAKER
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DevHelper::DevHelper(QWidget* parent)
: QMainWindow(parent)
{
  setupUi(this);

  setupGui();

  readSettings();

  // Do a final validity check on both widgets
  validityCheck();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DevHelper::~DevHelper()
{
  if(m_HelpDialog != nullptr)
  {
    delete m_HelpDialog;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::setupGui()
{
  // Allow internal widgets to update the status bar
  connect(filterMaker, SIGNAL(updateStatusBar(QString)), this, SLOT(updateStatusMessage(QString)));
  connect(pluginMaker, SIGNAL(updateStatusBar(QString)), this, SLOT(updateStatusMessage(QString)));

  // Set window to open at the center of the screen
  QDesktopWidget* desktop = QApplication::desktop();

  int screenWidth, width;
  int screenHeight, height;
  int x, y;
  QSize windowSize;

  screenWidth = desktop->width();   // get width of screen
  screenHeight = desktop->height(); // get height of screen

  windowSize = size(); // size of application window
  width = windowSize.width();
  height = windowSize.height();

  x = (screenWidth - width) / 2;
  y = (screenHeight - height) / 2;
  y -= 50;

  // move window to desired coordinates
  move(x, y);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::updateStatusMessage(QString message)
{
  statusbar->showMessage(message);
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void DevHelper::closeEvent(QCloseEvent* event)
{
  writeSettings();
}

// -----------------------------------------------------------------------------
//  Write our Prefs to file
// -----------------------------------------------------------------------------
void DevHelper::writeSettings()
{
  QtSSettings prefs;

  prefs.beginGroup("DevHelper");

  // Write PluginMaker settings
  prefs.beginGroup("PluginMaker");
  // Save the Plugin Name and Output Directory features to the QSettings object
  prefs.setValue("Plugin Name", pluginMaker->m_PluginName->text());
  prefs.setValue("Output Directory", pluginMaker->m_OutputDir->text());
  prefs.endGroup();

  // Write FilterMaker settings
  prefs.beginGroup("FilterMaker");
  // Save the Plugin Name and Output Directory features to the QSettings object
  prefs.setValue("Plugin Directory", filterMaker->pluginDir->text());
  prefs.setValue("Filter Name", filterMaker->filterName->text());
  prefs.beginGroup("FilterParameters");
  prefs.setValue("Count", filterMaker->filterParametersTable->rowCount());
  for(int i = 0; i < filterMaker->filterParametersTable->rowCount(); i++)
  {
    prefs.beginGroup(QString::number(i));
    prefs.setValue("Variable Name", filterMaker->filterParametersTable->item(i, FilterMaker::VAR_NAME)->text());
    prefs.setValue("Human Label", filterMaker->filterParametersTable->item(i, FilterMaker::HUMAN_NAME)->text());
    prefs.setValue("Type", filterMaker->filterParametersTable->item(i, FilterMaker::TYPE)->text());
    prefs.setValue("Category", filterMaker->filterParametersTable->item(i, FilterMaker::CATEGORY)->text());
    prefs.setValue("Initial Value", filterMaker->filterParametersTable->item(i, FilterMaker::INIT_VALUE)->text());
    prefs.endGroup();
  }
  prefs.endGroup();

  writeWindowSettings(prefs);
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::writeWindowSettings(QtSSettings& prefs)
{
  prefs.beginGroup("WindowSettings");
  QByteArray geo_data = saveGeometry();
  QByteArray layout_data = saveState();
  prefs.setValue(QString("Geometry"), geo_data);
  prefs.setValue(QString("Layout"), layout_data);
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::readSettings()
{
  QtSSettings prefs;

  prefs.beginGroup("DevHelper");

  prefs.beginGroup("PluginMaker");
  // Have PluginMaker read its settings
  pluginMaker->m_PluginName->setText(prefs.value("Plugin Name", QString("")).toString());
  pluginMaker->m_OutputDir->setText(prefs.value("Output Directory", QString("")).toString());
  readWindowSettings(prefs);
  prefs.endGroup();

  prefs.beginGroup("FilterMaker");
  // Have FilterMaker read its settings
  filterMaker->pluginDir->setText(prefs.value("Plugin Directory", QString("")).toString());
  filterMaker->filterName->setText(prefs.value("Filter Name", QString("")).toString());

  prefs.beginGroup("FilterParameters");
  int count = prefs.value("Count", QVariant(0)).toInt();
  for(int i = 0; i < count; i++)
  {
    prefs.beginGroup(QString::number(i));
    AddFilterParameter addFilterParameter;
    addFilterParameter.setVariableName(prefs.value("Variable Name", QString("")).toString());
    addFilterParameter.setHumanName(prefs.value("Human Label", QString("")).toString());
    addFilterParameter.setType(prefs.value("Type", QString("")).toString());
    addFilterParameter.setCategory(prefs.value("Category", QString("")).toString());
    addFilterParameter.setInitValue(prefs.value("Initial Value", QString("")).toString());

    filterMaker->addFilterParameterToTable(&addFilterParameter);
    prefs.endGroup();
  }
  prefs.endGroup();

  readWindowSettings(prefs);
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::readWindowSettings(QtSSettings& prefs)
{
  bool ok = false;
  prefs.beginGroup("WindowSettings");
  if(prefs.contains(QString("Geometry")))
  {
    QByteArray geo_data = prefs.value(QString("Geometry"), QByteArray());
    ok = restoreGeometry(geo_data);
    if(!ok)
    {
      qDebug() << "Error Restoring the Window Geometry"
               << "\n";
    }
  }

  if(prefs.contains(QString("Layout")))
  {
    QByteArray layout_data = prefs.value(QString("Layout"), QByteArray());
    restoreState(layout_data);
  }
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::on_actionShowUserManual_triggered()
{
  m_HelpDialog = new HelpWidget(this);
  m_HelpDialog->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DevHelper::on_actionAbout_triggered()
{
  QtSApplicationAboutBoxDialog about(PluginMakerProj::LicenseList, this);
  about.setApplicationInfo("DevHelper", "1.0.0");
  about.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool DevHelper::validityCheck()
{
  return (pluginMaker->validityCheck() && filterMaker->validityCheck());
}
