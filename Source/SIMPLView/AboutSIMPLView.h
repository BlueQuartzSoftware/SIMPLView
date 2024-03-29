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

//-- UIC generated Header
#include "ui_AboutSIMPLView.h"

class AboutSIMPLView : public QDialog, private Ui::AboutSIMPLView
{
  Q_OBJECT

public:
  AboutSIMPLView(QWidget* parent = nullptr);
  ~AboutSIMPLView() override;

  /**
   * @brief This will set the list of licenses that can be viewed by the
   * about box. The list should contain values such as ":/SomeLicense.license" which
   * are defined in a Qt resource file (*.qrc).
   * @param files The QString list of files to allow the About Box to load.
   */
  void setLicenseFiles(const QStringList& files);

protected Q_SLOTS:
  void on_licenseCombo_currentIndexChanged(int index);

protected:
  /**
   * @brief setupGui
   */
  void setupGui();

  /**
   * @brief Loads a file that is stored as a resource in the applications Qt resource
   * system.
   * @param qresourceFile The source file to load of the form ":/SomeFile.xxx"
   */
  void loadResourceFile(const QString& qresourceFile);

  /**
   * @brief readVersions
   */
  void readVersions();

private:
  QStringList m_licenseFiles;

  QAction* m_CloseAction = nullptr;

public:
  AboutSIMPLView(const AboutSIMPLView&) = delete;            // Copy Constructor Not Implemented
  AboutSIMPLView(AboutSIMPLView&&) = delete;                 // Move Constructor Not Implemented
  AboutSIMPLView& operator=(const AboutSIMPLView&) = delete; // Copy Assignment Not Implemented
  AboutSIMPLView& operator=(AboutSIMPLView&&) = delete;      // Move Assignment Not Implemented
};
