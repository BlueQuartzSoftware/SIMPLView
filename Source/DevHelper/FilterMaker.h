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

#include "DevHelper/AddFilterParameter.h"
#include "DevHelper/PMFileGenerator.h"

#include "ui_FilterMaker.h"

class FilterMaker : public QWidget, public Ui::FilterMaker
{
  Q_OBJECT

public:
  enum FPColumnIndex
  {
    VAR_NAME,
    HUMAN_NAME,
    TYPE,
    CATEGORY,
    INIT_VALUE
  };

  enum CodeChooserIndex
  {
    H_INDEX,
    CPP_INDEX,
    DOC_INDEX,
    TEST_INDEX
  };

  FilterMaker(QWidget* parent = nullptr);
  ~FilterMaker() override;

  bool validityCheck();

  QString getFilterName();
  QString getPluginDir();
  bool isPublic();

public Q_SLOTS:
  void addFilterParameterToTable(AddFilterParameter* widget);

protected:
  void setupGui();

  QString getDefaultSetupFPContents();
  QString getDefaultWriteFPContents();
  QString getDefaultDataCheckContents();
  QString getDefaultFPContents();
  QString getDefaultInitListContents();
  QString getDefaultFilterHIncludesContents();
  QString getDefaultFilterCPPIncludesContents();

protected Q_SLOTS:
  void on_selectBtn_clicked();
  void on_codeChooser_currentIndexChanged(int index);
  void on_pluginDir_textChanged(const QString& text);
  void on_filterName_textChanged(const QString& text);
  void on_filterParametersTable_itemChanged(QTableWidgetItem* item);
  void on_generateBtn_clicked();
  void on_addFilterParameterBtn_clicked();
  void on_removeFilterParameterBtn_clicked();
  void on_errorString_linkActivated(const QString& link);
  void generationError(const QString& test);

Q_SIGNALS:
  void generateBtnPressed();
  void updateStatusBar(QString message);

private:
  QString m_OpenDialogLastDirectory;
  QSharedPointer<PMFileGenerator> m_cppGenerator;
  QSharedPointer<PMFileGenerator> m_hGenerator;
  QSharedPointer<PMFileGenerator> m_htmlGenerator;
  QSharedPointer<PMFileGenerator> m_testGenerator;

  void updateFilterFileGenerators();
  void generateFilterFiles();
  void updateSourceList();
  void updateTestLocations();
  void updateTestList();
  QString createNamespaceString();

  QMap<QString, QString> getFunctionContents();

public:
  FilterMaker(const FilterMaker&) = delete;            // Copy Constructor Not Implemented
  FilterMaker(FilterMaker&&) = delete;                 // Move Constructor Not Implemented
  FilterMaker& operator=(const FilterMaker&) = delete; // Copy Assignment Not Implemented
  FilterMaker& operator=(FilterMaker&&) = delete;      // Move Assignment Not Implemented
};
