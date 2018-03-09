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

#ifndef _VtkMappingData_h_
#define _VtkMappingData_h_

#include "ui_VtkMappingData.h"
#include <QWidget>

#include <vtkSmartPointer.h>





class vtkScalarsToColors;
class vtkColorTransferFunction;
class VtkAbstractFilter;

class ColorPresetsDialog;

/**
 * @class VtkMappingData VtkMappingData.h SIMPLView/VtkSIMPL/QtWidgets/VtkMappingData.h
 * @brief This class handles the mapping controls for a single visualization filter.
 */
class VtkMappingData : public QWidget, public Ui::VtkMappingData
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  */
  VtkMappingData(QWidget* parent = nullptr);

  /**
  * @brief Deconstructor
  */
  ~VtkMappingData();

  /**
  * @brief Sets the current visibility filter
  * @param filter
  */
  void setFilter(VtkAbstractFilter* filter);

public slots:
  /**
  * @brief Applies the default mapping data
  */
  void setDefaults();

protected slots:
  /**
  * @brief Opens the preset color dialog
  */
  void selectPreset();

  /**
  * @brief Loads a preset color set
  * @param preset
  * @param pixmap
  */
  void load(const QJsonObject& preset, const QPixmap& pixmap);

  /**
  * @brief Inverts the current color set
  */
  void invert();

  /**
  * @brief Sets whether or not scalars should be mapped
  * @param map
  */
  void mapScalars(bool map = true);

signals:
  /**
  * @brief Signals that VTK mapping should be updated
  */
  void updateMapping();

protected:
  /**
  * @brief Connects Qt signals and slots
  */
  void connectSlots();
  
  /**
  * @brief Disconnects Qt signals and slots
  */
  void disconnectSlots();

  /**
  * @brief Returns whether or not scalars should be mapped
  */
  bool shouldMapScalars();

  ColorPresetsDialog* m_presetsDialog;

  vtkSmartPointer<vtkColorTransferFunction> m_lookupTable;
  VtkAbstractFilter* m_filter;
};

#endif /* _VtkMappingData_h_ */
