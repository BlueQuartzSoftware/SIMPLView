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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  PARTICULAR PURPOSE ARE
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

#include "PMFileGenerator.h"

#include <iostream>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QUuid>

#include "SVWidgetsLib/QtSupport/QtSApplicationFileInfo.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PMFileGenerator::PMFileGenerator(QString outputDir, QString pathTemplate, QString fileName, QString codeTemplateResourcePath, QTreeWidgetItem* wi, QObject* parent)
: PMDirGenerator(outputDir, pathTemplate, QString(""), codeTemplateResourcePath, wi, parent)
, setupFPContents("")
, fpContents("")
, dataCheckContents("")
, initListContents("")
, filterHIncludesContents("")
, filterCPPIncludesContents("")
, m_FileName(fileName)
{
  if(nullptr != wi)
  {
    wi->setText(0, fileName);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PMFileGenerator::~PMFileGenerator()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PMFileGenerator::getFileName()
{
  return m_FileName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::pluginNameChanged(const QString& pluginName)
{
  QString pin = cleanName(pluginName);

  if(pin.isEmpty() == true)
  {
    setPluginName("Unknown Plugin Name");
  }
  else
  {
    setPluginName(pin);
  }

  m_FilterName = pin + "Filter";

  if(isNameChangeable() == false)
  {
    return;
  }
  m_FileName = pin + getDisplaySuffix();

  if(nullptr != getTreeWidgetItem())
  {
    getTreeWidgetItem()->setText(0, m_FileName);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::outputDirChanged(const QString& outputDir)
{
  //  qDebug() << "PMFileGenerator::outputDirChanged" << "\n";
  setOutputDir(outputDir);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::generateOutput()
{
  //  qDebug() << "PMFileGenerator::generateOutput" << "\n";
  if(doesGenerateOutput() == false)
  {
    return;
  }

  // Get text feature values from widget
  QString pluginName = getPluginName();
  QString pluginDir = getOutputDir();

  if(pluginName.isEmpty() == true || pluginDir.isEmpty() == true)
  {
    return;
  }

  QString contents = getFileContents(QString());

  if(contents.isEmpty() == false)
  {
    QString parentPath = getOutputDir() + QDir::separator() + getPathTemplate().replace("@PluginName@", getPluginName());
    parentPath = QDir::toNativeSeparators(parentPath);

    QDir dir(parentPath);
    dir.mkpath(parentPath);

    parentPath = parentPath + QDir::separator() + m_FileName;

    // Write to file
    QFile f(parentPath);
    if(f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&f);
      out << contents;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PMFileGenerator::generateFileContents(QString replaceStr)
{
  QString pluginName = getPluginName();

  QString contents = getFileContents(replaceStr);

  return contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PMFileGenerator::getFileContents(const QString& replaceStr)
{
  // Get text feature values from widget
  QString pluginName = getPluginName();
  QString pluginDir = getOutputDir();
  QString filterName = getFilterName();
  QFileInfo fi(m_FileName);
  QString className = fi.baseName();
  QString text = "";

  if(pluginName.isEmpty())
  {
    return text;
  }

  // Open file
  QFile rfile(getCodeTemplateResourcePath());
  if(rfile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QTextStream in(&rfile);
    text = in.readAll();

    text.replace("@SetupFPContents@", setupFPContents);
    text.replace("@FPContents@", fpContents);
    text.replace("@DataCheckContents@", dataCheckContents);
    text.replace("@InitList@", initListContents);
    text.replace("@Filter_H_Includes@", filterHIncludesContents);
    text.replace("@Filter_CPP_Includes@", filterCPPIncludesContents);
    text.replace("@PYBindContents@", pyContents);
    text.replace("@Filter_Parameter_Definitions@", filterCPPFilterParameterDefinitions);
    text.replace("@Filter_Parameter_Declarations@", filterHFilterParameterDeclarations);

    // ****************************************************************
    // The following replacements need to happen after the above
    // replacements, because the above replacements add some additional
    // tokens to the generated file.
    // ****************************************************************
    text.replace("@PluginName@", pluginName);
    text.replace("@PluginNameLowerCase@", pluginName.toLower());
    text.replace("@ClassName@", className);
    text.replace("@FilterName@", filterName);
    text.replace("@MD_FILE_NAME@", m_FileName);
    text.replace("@ClassNameLowerCase@", className.toLower());
    text.replace("@FilterGroup@", pluginName);
    text.replace("@FilterSubgroup@", pluginName);

    uint l = 100;
    ushort w1 = 200;
    ushort w2 = 300;

    uchar b[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int32_t i = 0;
    while(i < 8 && i < pluginName.size())
    {
      b[i] = static_cast<uint8_t>(pluginName.at(i).toLatin1());
      i++;
    }
    QUuid uuid = QUuid(l, w1, w2, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
    QString nameSpace = QString("%1 %2").arg(className).arg(pluginName);
    QUuid p1 = QUuid::createUuidV5(uuid, nameSpace);
    text.replace("@Uuid@", p1.toString());

    if(!replaceStr.isEmpty())
    {
      text.replace("@AddTestText@", replaceStr); // Replace token for Test/CMakeLists.txt file
      text.replace("@Namespaces@", replaceStr);  // Replace token for Test/TestFileLocations.h.in file
    }
    // ****************************************************************

    rfile.close();
  }

  return text;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::generateOutputWithFilterNames(QSet<QString> names)
{
  //  qDebug() << "PMFileGenerator::generateOutput" << "\n";
  if(doesGenerateOutput() == false)
  {
    return;
  }

  // Get text feature values from widget
  QString pluginName = getPluginName();
  QString pluginDir = getOutputDir();

  if(pluginName.isEmpty() || pluginDir.isEmpty())
  {
    return;
  }

  //  QString classNameLowerCase = m_ClassName.toLower();

  // Open file
  QFile rfile(getCodeTemplateResourcePath());
  if(rfile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QTextStream in(&rfile);
    QString text = in.readAll();

    text.replace("@PluginName@", pluginName);
    QFileInfo fi(m_FileName);
    QString className = fi.baseName();
    text.replace("@ClassName@", className);
    text.replace("@MD_FILE_NAME@", m_FileName);
    text.replace("@ClassNameLowerCase@", className.toLower());
    text.replace("@FilterGroup@", pluginName);
    text.replace("@FilterSubgroup@", pluginName);

    if(!names.isEmpty())
    {
      if(getFileName() == "TestFileLocations.h.in")
      {
        QString replaceStr = createReplacementString(TESTFILELOCATIONS, names);
        text.replace("@Namespaces@", replaceStr); // Replace token for Test/TestFileLocations.h.in file
      }
      else if(getFileName() == "CMakeLists.txt")
      {
        QString replaceStr = createReplacementString(CMAKELISTS, names);
        text.replace("@AddTestText@", replaceStr); // Replace token for Test/CMakeLists.txt file
      }
    }
    else
    {
      text.replace("\n  @Namespaces@\n", ""); // Replace token for Test/TestFileLocations.h.in file
      text.replace("\n@AddTestText@\n", "");  // Replace token for Test/CMakeLists.txt file
    }

    QString parentPath = getOutputDir() + QDir::separator() + getPathTemplate().replace("@PluginName@", getPluginName());
    parentPath = QDir::toNativeSeparators(parentPath);

    QDir dir(parentPath);
    dir.mkpath(parentPath);

    parentPath = parentPath + QDir::separator() + m_FileName;
    // Write to file
    QFile f(parentPath);
    if(f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&f);
      out << text;
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PMFileGenerator::createReplacementString(FileType type, QSet<QString> names)
{
  QString pluginName = getPluginName();

  QString replaceStr = "";
  QTextStream rsOut(&replaceStr);
  if(type == CMAKELISTS)
  {
    // Build up the huge string full of namespaces using names
    QSet<QString>::iterator iter = names.begin();
    while(iter != names.end())
    {
      QString name = *iter;

      if(name == "@PluginName@Filter")
      {
        name.replace("@PluginName@", pluginName);
      }
      rsOut << "  " << name << "Test\n";
      ++iter;
    }
  }
  else if(type == TESTFILELOCATIONS)
  {
    // Build up the huge string full of namespaces using names
    QSet<QString>::iterator iter = names.begin();
    while(iter != names.end())
    {
      QString name = *iter;

      if(name == "@PluginName@Filter")
      {
        name.replace("@PluginName@", pluginName);
      }

      replaceStr.append("namespace " + name + "Test\n");
      replaceStr.append("  {\n");
      replaceStr.append("    const QString TestFile1(\"@TEST_TEMP_DIR@/TestFile1.txt\");\n");
      replaceStr.append("    const QString TestFile2(\"@TEST_TEMP_DIR@/TestFile2.txt\");\n");
      replaceStr.append("  }\n");
      if(++iter != names.end())
      {
        replaceStr.append("\n");
      }
    }

    replaceStr.append("@FILTER_NAMESPACE@");
  }

  return replaceStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::setSetupFPContents(const QString& contents)
{
  setupFPContents = contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::setDataCheckContents(const QString& contents)
{
  dataCheckContents = contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::setFPContents(const QString& contents)
{
  fpContents = contents;
}

// -----------------------------------------------------------------------------
void PMFileGenerator::setPyContents(const QString& contents)
{
  pyContents = contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::setInitListContents(const QString& contents)
{
  initListContents = contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PMFileGenerator::setFilterHIncludesContents(const QString& contents)
{
  filterHIncludesContents = contents;
}

// -----------------------------------------------------------------------------
void PMFileGenerator::setFilterCPPIncludesContents(const QString& contents)
{
  filterCPPIncludesContents = contents;
}

// -----------------------------------------------------------------------------
void PMFileGenerator::setFilterParameterDefinitions(const QString& contents)
{
  filterCPPFilterParameterDefinitions = contents;
}

// -----------------------------------------------------------------------------
void PMFileGenerator::setFilterParameterDeclarations(const QString& contents)
{
  filterHFilterParameterDeclarations = contents;
}
