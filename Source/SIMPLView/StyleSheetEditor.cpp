/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "BrandedStrings.h"
#include "SVStyle.h"
#include "StyleSheetEditor.h"

#include "ui_StyleSheetEditor.h"

StyleSheetEditor::StyleSheetEditor(QWidget* parent)
: QDialog(parent)
, m_Ui(new Ui::StyleSheetEditor)

{
  m_Ui->setupUi(this);
  this->setSizeGripEnabled(true);

  QString defaultLoadedThemePath = BrandedStrings::DefaultStyleDirectory + "/" + BrandedStrings::DefaultLoadedTheme + ".json";
  QFileInfo fi(defaultLoadedThemePath);

  connect(&m_FileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(qssFileChanged(const QString&)));

  QString styleSheetPath = QString("%1/%2").arg(QDir::homePath(), 1).arg("DREAM3D-Dev/BrandedDREAM3D/DREAM3D/StyleSheets/Midnight.json");
  m_Ui->jsonFilePath->setText(styleSheetPath);
  styleSheetPath = QString("%1/%2").arg(QDir::homePath(), 1).arg("DREAM3D-Dev/BrandedDREAM3D/DREAM3D/StyleSheets/Midnight.css");
  m_Ui->cssFilePath->setText(styleSheetPath);
  on_reloadButton_stateChanged(Qt::Checked);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StyleSheetEditor::on_reloadButton_stateChanged(int state)
{
  if(state == Qt::Checked)
  {
    m_FileWatcher.addPath(m_Ui->jsonFilePath->text());
    m_FileWatcher.addPath(m_Ui->cssFilePath->text());
  }
  else
  {
    m_FileWatcher.removePath(m_Ui->jsonFilePath->text());
    m_FileWatcher.removePath(m_Ui->cssFilePath->text());
  }
}

// -----------------------------------------------------------------------------
// This is called when the file on the file system changes
// -----------------------------------------------------------------------------
void StyleSheetEditor::qssFileChanged(const QString& filePath)
{
  qDebug() << "Changed: " << filePath;
  SVStyle* style = SVStyle::Instance();
  style->loadStyleSheet(m_Ui->jsonFilePath->text());
}

// -----------------------------------------------------------------------------
// This is called when the user hits the "return" key on the keyboard while editing
// QLineEdit
// -----------------------------------------------------------------------------
void StyleSheetEditor::on_jsonFilePath_returnPressed()
{
  qssFileChanged(m_Ui->jsonFilePath->text());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StyleSheetEditor::on_cssFilePath_returnPressed()
{
  qssFileChanged(m_Ui->cssFilePath->text());
}
