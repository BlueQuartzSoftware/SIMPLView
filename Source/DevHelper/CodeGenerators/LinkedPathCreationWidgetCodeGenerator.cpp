/* ============================================================================
 * Copyright (c) 2019-2019 BlueQuartz Software, LLC
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
 *    United States Air Force Prime Contract FA8650-15-D-5280
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "LinkedPathCreationWidgetCodeGenerator.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LinkedPathCreationWidgetCodeGenerator::LinkedPathCreationWidgetCodeGenerator(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue)
: FPCodeGenerator(humanLabel, propertyName, category, initValue, "DataArrayPath")
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
LinkedPathCreationWidgetCodeGenerator::~LinkedPathCreationWidgetCodeGenerator() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString LinkedPathCreationWidgetCodeGenerator::generateSetupFilterParameters()
{
  QString s;
  QTextStream out(&s);
  //out << "  parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP(\"" << getHumanLabel() << "\", " << getPropertyName() << ", " << getCategory() << ", @FilterName@));";
  out << "#error The correct code for the LinkedPathCreation Filter Parameter needs to be inserted here. Please see the documentation for the filter parameter.";
  return s;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString LinkedPathCreationWidgetCodeGenerator::generateDataCheck()
{
  return "";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString LinkedPathCreationWidgetCodeGenerator::generateFilterParameters()
{
  QString contents;
  QTextStream ss(&contents);
  ss << "    SIMPL_FILTER_PARAMETER(QString, " + getPropertyName() + ")\n";
  ss << "    Q_PROPERTY(QString " + getPropertyName() + " READ get" + getPropertyName() + " WRITE set" + getPropertyName() + ")";

  return contents;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QString> LinkedPathCreationWidgetCodeGenerator::generateCPPIncludes()
{
  QList<QString> list;
  list.push_back("#include \"SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h\"");
  return list;
}
