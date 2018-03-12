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

#ifndef _VtkCropWidget_h_
#define _VtkCropWidget_h_

#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "ui_VtkCropWidget.h"

#include <vtkSmartPointer.h>

class vtkImageData;

/**
 * @class VtkCropWidget VtkCropWidget.h SIMPLView/VtkSIMPL/VtkWidgets/VtkCropWidget.h
 * @brief This class is the VtkAbstractWidget type used by VtkCropFilter.
 */
class VtkCropWidget : public VtkAbstractWidget, private Ui::VtkCropWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  * @param imageData
  * @param iren
  */
  VtkCropWidget(QWidget* parent, vtkSmartPointer<vtkImageData> imageData, vtkRenderWindowInteractor* iren);

  /**
  * @brief Deconstructor
  */
  ~VtkCropWidget();

  /**
  * @brief Returns the volume of interest
  * @return
  */
  int* getVOI();

  /**
  * @brief Returns the sample rate for generating the output
  * @return
  */
  int* getSampleRate();

  /**
  * @brief Returns the vtkImplicitFunction used
  * @return
  */
  vtkSmartPointer<vtkImplicitFunction> getImplicitFunction() override;

  /**
  * @brief enable
  */
  void enable() override;

  /**
  * @brief disable
  */
  void disable() override;

  /**
  * @brief Applies the widget values to the filter
  */
  void apply() override;

  /**
  * @brief Resets the widget values
  */
  void reset() override;

  /**
  * @brief Updates the QSpinBox values to match the volume of interest and 
  * sample rate.
  */
  void updateSpinBoxes();

private:
  vtkSmartPointer<vtkImageData> m_imageData;
  int* voi;
  int* sampleRate;
};

#endif
