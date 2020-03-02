/* ============================================================================
 * Copyright (c) 2019 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"

#include "DevHelper/CodeGenerators/FPCodeGenerator.h"

class DataContainerGridSelectionWidgetCodeGenerator : public FPCodeGenerator
{
public:
  using Self = DataContainerGridSelectionWidgetCodeGenerator;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue)
  {
    Pointer sharedPtr(new DataContainerGridSelectionWidgetCodeGenerator(humanLabel, propertyName, category, initValue));
    return sharedPtr;
  }

  virtual ~DataContainerGridSelectionWidgetCodeGenerator();

  QString generateSetupFilterParameters() override;

  QString generateDataCheck() override;

  QString generateFilterParameters() override;

  QList<QString> generateCPPIncludes() override;

protected:
  DataContainerGridSelectionWidgetCodeGenerator(const QString& humanLabel, const QString& propertyName, const QString& category, const QString& initValue);

public:
  DataContainerGridSelectionWidgetCodeGenerator(const DataContainerGridSelectionWidgetCodeGenerator&) = delete;            // Copy Constructor Not Implemented
  DataContainerGridSelectionWidgetCodeGenerator(DataContainerGridSelectionWidgetCodeGenerator&&) = delete;                 // Move Constructor Not Implemented
  DataContainerGridSelectionWidgetCodeGenerator& operator=(const DataContainerGridSelectionWidgetCodeGenerator&) = delete; // Copy Assignment Not Implemented
  DataContainerGridSelectionWidgetCodeGenerator& operator=(DataContainerGridSelectionWidgetCodeGenerator&&) = delete;      // Move Assignment Not Implemented

private:
};
