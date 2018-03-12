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

#ifndef _VtkThresholdWidget_h_
#define _VtkThresholdWidget_h_

#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "ui_VtkThresholdWidget.h"

#include <vtkSmartPointer.h>

/**
 * @class VtkThresholdWidget VtkThresholdWidget.h 
 * SIMPLView/VtkSIMPL/VtkWidgets/VtkThresholdWidget.h
 * @brief This class is used to handle user inputs for thresholding over a 
 * vtkDataArray. It is primarily used by VtkThresholdFilter as its way of 
 * accepting input for the filter.
 */
class VtkThresholdWidget : public VtkAbstractWidget, private Ui::VtkThresholdWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  * @param range
  * @param bounds
  * @param iren
  */
  VtkThresholdWidget(QWidget* parent, double range[2], double bounds[6], vtkRenderWindowInteractor* iren);

  /**
  * @brief Deconstructor
  */
  ~VtkThresholdWidget();

  /**
  * @brief Returns the lower bound for thresholding
  * @return
  */
  double getLowerBound();

  /**
  * @brief Returns the upper bound for thresholding
  * @return
  */
  double getUpperBound();

  /**
  * @brief Sets the lower bound for thresholding
  * @param min
  */
  void setLowerThreshold(double min);

  /**
  * @brief Sets the upper bound for thresholding
  * @param max
  */
  void setUpperThreshold(double max);

  /**
  * @brief Sets the range to threshold over
  * @param min
  * @param max
  */
  void setScalarRange(int min, int max);

  /**
  * @brief enable
  */
  void enable() override;

  /**
  * @brief disable
  */
  void disable() override;

  /**
  * @brief Applies the values for thresholding over
  */
  void apply() override;

  /**
  * @brief Resets the widget to the last applied values
  */
  void reset() override;

  /**
  * @brief Returns the vtkImplicitFunction for the widget
  * @return
  */
  vtkSmartPointer<vtkImplicitFunction> getImplicitFunction() override;

public slots:
  /**
  * @brief Handles any changes in the minimum spin box
  */
  void minSpinBoxValueChanged();

  /**
  * @brief Handles any changes in the maximum spin box
  */
  void maxSpinBoxValueChanged();

  /**
  * @brief Handles any changes in the minimum slider
  */
  void minSliderValueChanged();
  
  /**
  * @brief Handles any changes in the maximum slider
  */
  void maxSliderValueChanged();

protected:
  /**
  * @brief Copies the value from a QSlider to a QDoubleSpinBox
  * @param slider
  * @param spinBox
  */
  void sliderToSpinBox(QSlider* slider, QDoubleSpinBox* spinBox);

  /**
  * @brief Copies the value from a QDoubleSpinBox to a QSlider
  * @param spinBox
  * @param slider
  */
  void spinBoxToSlider(QDoubleSpinBox* spinBox, QSlider* slider);

  /**
  * @brief Checks if the min spin box is greater than the max spin box.
  * If it is, then the max spin box is updated and the method returns true.
  * Otherwise, this function returns false.
  * @return
  */
  bool checkMinSpinBox();

  /**
  * @brief Checks if the max spin box is greater than the min spin box.
  * If it is, then the min spin box is updated and the method returns true.
  * Otherwise, this function returns false.
  * @return
  */
  bool checkMaxSpinBox();

private:
  double lastMinValue;
  double lastMaxValue;
};

#endif
