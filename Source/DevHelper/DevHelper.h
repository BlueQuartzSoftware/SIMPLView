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

#pragma once

#include <QtWidgets/QWidget>

#include "SVWidgetsLib/QtSupport/QtSSettings.h"

#include "ui_DevHelper.h"

class HelpWidget;

class DevHelper : public QMainWindow, public Ui::DevHelper
{
  Q_OBJECT

public:
  DevHelper(QWidget* parent = nullptr);
  ~DevHelper();

  bool validityCheck();

protected:
  void setupGui();

protected Q_SLOTS:
  void updateStatusMessage(QString message);

  void on_actionShowUserManual_triggered();
  void on_actionAbout_triggered();

private:
  HelpWidget* m_HelpDialog = nullptr;

  void closeEvent(QCloseEvent* event);
  void writeSettings();
  void readSettings();
  void readWindowSettings(QtSSettings& prefs);
  void writeWindowSettings(QtSSettings& prefs);

public:
  DevHelper(const DevHelper&) = delete;            // Copy Constructor Not Implemented
  DevHelper(DevHelper&&) = delete;                 // Move Constructor Not Implemented
  DevHelper& operator=(const DevHelper&) = delete; // Copy Assignment Not Implemented
  DevHelper& operator=(DevHelper&&) = delete;      // Move Assignment Not Implemented
};
