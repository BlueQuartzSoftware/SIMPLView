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

#include <memory>

#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"

// stringised version of line number (must be done in two steps)
#define STRINGISE(N) #N
#define EXPAND_THEN_STRINGISE(N) STRINGISE(N)
#define __LINE_STR__ EXPAND_THEN_STRINGISE(__LINE__)

// MSVC-suitable routines for formatting <#pragma message>
#define __LOC__ __FILE__ "(" __LINE_STR__ ")"
#define __OUTPUT_FORMAT__(type) __LOC__ " : " type " : "

// specific message types for <#pragma message>
#define __WARN__ __OUTPUT_FORMAT__("warning")
#define __ERR__ __OUTPUT_FORMAT__("error")
#define __MSG__ __OUTPUT_FORMAT__("programmer's message")
#define __TODO__ __OUTPUT_FORMAT__("to do")

class FPCodeGenerator
{
public:
  using Self = FPCodeGenerator;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue);

  virtual ~FPCodeGenerator();

  virtual QString generateSetupFilterParameters();

  virtual QString generateDataCheck();

  virtual QString generateFilterAccessorDeclarations();

  virtual QString generateInitializationList();

  virtual QString generatePybindContents();

  virtual QList<QString> generateHIncludes();

  virtual QList<QString> generateCPPIncludes();

  virtual QString generateFilterParameterDefinitions();

  virtual QString generateFilterParameterDeclarations();

  /**
   * @deprecated
   * @brief generateFilterParameters
   * @return
   */
  virtual QString generateFilterParameters();

protected:
  FPCodeGenerator(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue, const QString& varType, bool podType = false);

  QString getPropertyName();
  QString getHumanLabel();
  QString getCategory();
  QString getInitValue();
  QString getVariableType();

private:
  QString m_PropertyName;
  QString m_HumanLabel;
  QString m_Category;
  QString m_InitValue;
  QString m_VariableType = QString("Unknown");
  bool m_PODType = false;

public:
  FPCodeGenerator(const FPCodeGenerator&) = delete;            // Copy Constructor Not Implemented
  FPCodeGenerator(FPCodeGenerator&&) = delete;                 // Move Constructor Not Implemented
  FPCodeGenerator& operator=(const FPCodeGenerator&) = delete; // Copy Assignment Not Implemented
  FPCodeGenerator& operator=(FPCodeGenerator&&) = delete;      // Move Assignment Not Implemented
};
