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


#pragma once

#include "PMDirGenerator.h"

enum FileType
{
  CMAKELISTS,
  TESTFILELOCATIONS,
  DEFAULT
};

/*
 *
 */
class PMFileGenerator : public PMDirGenerator
{
    Q_OBJECT

  public:
    PMFileGenerator(QString outputDir, QString pathTemplate, QString fileName, QString codeTemplateResourcePath, QTreeWidgetItem* wi, QObject* parent = nullptr);
    ~PMFileGenerator() override;

    QString getFileName();

    void setSetupFPContents(const QString& contents);
    void setDataCheckContents(const QString& contents);
    void setFPContents(const QString& contents);
    void setInitListContents(const QString& contents);
    void setFilterHIncludesContents(const QString& contents);
    void setFilterCPPIncludesContents(const QString& contents);
    void setPyContents(const QString& contents);
    void setFilterParameterDefinitions(const QString& contents);
    void setFilterParameterDeclarations(const QString& contents);

    QString generateFileContents(QString replaceStr = "") override;

    QString createReplacementString(FileType type, QSet<QString> names);

  public slots:
    void generateOutput() override;

  protected slots:
    void pluginNameChanged(const QString& plugname) override;
    void outputDirChanged(const QString& outputDir) override;
    void generateOutputWithFilterNames(QSet<QString> names);

  protected:
    QString setupFPContents;
    QString fpContents;
    QString dataCheckContents;
    QString initListContents;
    QString filterHIncludesContents;
    QString filterCPPIncludesContents;
    QString pyContents;
    QString filterCPPFilterParameterDefinitions;
    QString filterHFilterParameterDeclarations;

  private:
    QString m_FileName;

    QString getFileContents(const QString& replaceStr);
};


