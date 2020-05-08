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

#include "FPCodeGenerator.h"

#include <QtCore/QTextStream>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FPCodeGenerator::FPCodeGenerator(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue, const QString& varType, bool podType)
: m_PropertyName(propertyName)
, m_HumanLabel(humanLabel)
, m_Category(category)
, m_InitValue(initValue)
, m_VariableType(varType)
, m_PODType(podType)
{
  if(m_Category == "Parameter")
  {
    m_Category = "FilterParameter::Parameter";
  }
  else if(m_Category == "Required Arrays")
  {
    m_Category = "FilterParameter::RequiredArray";
  }
  else if(m_Category == "Created Arrays")
  {
    m_Category = "FilterParameter::CreatedArray";
  }
  else
  {
    m_Category = "FilterParameter::Uncategorized";
  }
}

// -----------------------------------------------------------------------------
FPCodeGenerator::Pointer FPCodeGenerator::New(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue)
{
  Pointer sharedPtr(new FPCodeGenerator(humanLabel, propertyName, category, initValue, "UNKNOWN", false));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FPCodeGenerator::~FPCodeGenerator() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::getPropertyName()
{
  return m_PropertyName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::getHumanLabel()
{
  return m_HumanLabel;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::getCategory()
{
  return m_Category;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::getInitValue()
{
  return m_InitValue;
}

// -----------------------------------------------------------------------------
QString FPCodeGenerator::getVariableType()
{
  return m_VariableType;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateSetupFilterParameters()
{
  return "";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateDataCheck()
{
  return "";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateFilterAccessorDeclarations()
{
  QString contents;
  QTextStream ss(&contents);
  ss << "    /**\n    * @brief Sets the value for Filter Parameter for " << getPropertyName() << "\n    * @param value The new value to use.\n    */\n";
  if(m_PODType)
  {
    ss << "    void set" << getPropertyName() << "(" << getVariableType() << " value);\n";
  }
  else
  {
    ss << "    void set" << getPropertyName() << "(const " << getVariableType() << "& value);\n";
  }
  ss << "    /**\n    * @brief Gets the Filter Parameter value for " << getPropertyName() << "\n    * @return The value for " << getPropertyName() << "\n    */\n";
  ss << "    " << getVariableType() << " get" << getPropertyName() << "() const;\n";
  ss << "    Q_PROPERTY(" << getVariableType() << " " + getPropertyName() + " READ get" + getPropertyName() + " WRITE set" + getPropertyName() + ")";

  return contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::generatePybindContents()
{
  // return "";
  return QString("  PYB11_PROPERTY(" + getVariableType() + " " + getPropertyName() + " READ get" + getPropertyName() + " WRITE set" + getPropertyName() + ")\n");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateInitializationList()
{
  QString contents;
  if(!getInitValue().isEmpty())
  {
    QTextStream ss(&contents);
    ss << ",  m_" + getPropertyName() + "(" + getInitValue() + ")";
  }

  return contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QString> FPCodeGenerator::generateHIncludes()
{
  return QList<QString>();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QString> FPCodeGenerator::generateCPPIncludes()
{
  return QList<QString>();
}

// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateFilterParameterDeclarations()
{
  QString data;
  QTextStream out(&data);
  out << "    " << getVariableType() << "  m_" << getPropertyName() << ";\n";
  return data;
}

// -----------------------------------------------------------------------------
QString FPCodeGenerator::generateFilterParameterDefinitions()
{
  QString data;
  QTextStream out(&data);
  out << "// -----------------------------------------------------------------------------\n";
  if(m_PODType)
  {
    out << "void @ClassName@::set" << getPropertyName() << "(" << getVariableType() << " value)\n";
  }
  else
  {
    out << "void @ClassName@::set" << getPropertyName() << "(const " << getVariableType() << "& value)\n";
  }
  out << "{\n";
  out << "    m_" << getPropertyName() << " = value;\n";
  out << "}\n";
  out << "// -----------------------------------------------------------------------------\n";
  out << getVariableType() << " @ClassName@::get" << getPropertyName() << "() const\n";
  out << "{\n  return m_" << getPropertyName() << ";\n}\n";

  return data;
}

// -----------------------------------------------------------------------------
FPCodeGenerator::Pointer FPCodeGenerator::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

QString FPCodeGenerator::generateFilterParameters()
{
  return {""};
}
