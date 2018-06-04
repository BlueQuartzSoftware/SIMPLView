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

#include "SVStyle.h"
#include "StyleSheetEditor.h"

#include "SVWidgetsLib/QtSupport/QtSStyles.h"

#include "ui_StyleSheetEditor.h"

StyleSheetEditor::StyleSheetEditor(QWidget* parent)
: QDialog(parent)
, m_Ui(new Ui::StyleSheetEditor)

{
  m_Ui->setupUi(this);
  this->setSizeGripEnabled(true);

  QRegularExpression regExp("^.(.*)\\+?Style$");
  QString defaultStyle = QApplication::style()->metaObject()->className();
  QRegularExpressionMatch match = regExp.match(defaultStyle);

  if(match.hasMatch())
    defaultStyle = match.captured(1);

  m_Ui->styleCombo->addItems(QStyleFactory::keys());
  m_Ui->styleCombo->setCurrentIndex(m_Ui->styleCombo->findText(defaultStyle, Qt::MatchContains));

  m_Ui->styleSheetCombo->addItem("Default");
  m_Ui->styleSheetCombo->addItem("Light");
  m_Ui->styleSheetCombo->addItem("Orange");
  m_Ui->styleSheetCombo->addItem("Watermelon");
  m_Ui->styleSheetCombo->addItem("Green");
  m_Ui->styleSheetCombo->setCurrentIndex(m_Ui->styleSheetCombo->findText("Light"));

  connect(&m_FileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(qssFileChanged(const QString&)));

//  QString styleSheetPath = QString("%1/%2").arg(QDir::homePath(), 1).arg("DREAM3D-Dev/DREAM3D/ExternalProjects/BrandedDREAM3D/DREAM3D/StyleSheets/");
  m_Ui->qssFilePath->setText("/Users/joeykleingers/Workspace/BrandedDREAM3D/DREAM3D/StyleSheets");
}

void StyleSheetEditor::on_styleCombo_activated(const QString& styleName)
{
  //  qApp->setStyle(styleName);
}

void StyleSheetEditor::on_styleSheetCombo_activated(const QString& sheetName)
{
  on_reloadButton_stateChanged(m_Ui->reloadButton->checkState());
  qssFileChanged(QString(""));
}

void StyleSheetEditor::on_reloadButton_stateChanged(int state)
{
  if(state == Qt::Checked)
  {
    m_FileWatcher.addPath(m_Ui->qssFilePath->text() + "/" + m_Ui->styleSheetCombo->currentText() + ".css");
    m_FileWatcher.addPath(m_Ui->qssFilePath->text() + "/" + m_Ui->styleSheetCombo->currentText() + ".json");
  }
  else
  {
    m_FileWatcher.removePath(m_Ui->qssFilePath->text() + "/" + m_Ui->styleSheetCombo->currentText() + ".css");
    m_FileWatcher.removePath(m_Ui->qssFilePath->text() + "/" + m_Ui->styleSheetCombo->currentText() + ".json");
  }
}

void StyleSheetEditor::qssFileChanged(const QString& filePath)
{
  QString jsonFilePath = m_Ui->qssFilePath->text() + "/" + m_Ui->styleSheetCombo->currentText() + ".json";

  SVStyle* style = SVStyle::Instance();
  style->loadStyleSheet(jsonFilePath);
}
