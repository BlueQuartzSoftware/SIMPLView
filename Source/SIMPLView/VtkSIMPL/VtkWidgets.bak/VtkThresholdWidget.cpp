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

#include "VtkThresholdWidget.h"

#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkImplicitFunction.h>
#include <vtkSmartPointer.h>

#include <QDoubleSpinBox>
#include <QSlider>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/VtkWidgets/moc_VtkThresholdWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkThresholdWidget::VtkThresholdWidget(QWidget* parent, double range[2], double bounds[6], vtkRenderWindowInteractor* iren)
: VtkAbstractWidget(parent, bounds, iren)
{
  setupUi(this);

  // updateSpinBoxes();

  lastMinValue = range[0];
  lastMaxValue = range[1];

  // adjust the vtkWidget when values are changed
  connect(minSpinBox, SIGNAL(editingFinished()), this, SLOT(minSpinBoxValueChanged()));
  connect(maxSpinBox, SIGNAL(editingFinished()), this, SLOT(maxSpinBoxValueChanged()));

  connect(minSlider, SIGNAL(valueChanged(int)), this, SLOT(minSliderValueChanged()));
  connect(maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxSliderValueChanged()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkThresholdWidget::~VtkThresholdWidget()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double VtkThresholdWidget::getLowerBound()
{
  return minSpinBox->value();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::setLowerThreshold(double min)
{
  minSpinBox->setValue(min);
  minSpinBoxValueChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::setUpperThreshold(double max)
{
  maxSpinBox->setValue(max);
  maxSpinBoxValueChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double VtkThresholdWidget::getUpperBound()
{
  return maxSpinBox->value();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::setScalarRange(int min, int max)
{
  minSpinBox->setMinimum(min);
  maxSpinBox->setMinimum(min);

  minSpinBox->setMaximum(max);
  maxSpinBox->setMaximum(max);

  // adjust sliders accordingly
  minSpinBoxValueChanged();
  maxSpinBoxValueChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::enable()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::disable()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::apply()
{
  lastMinValue = getLowerBound();
  lastMaxValue = getUpperBound();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::reset()
{
  minSpinBox->setValue(lastMinValue);
  maxSpinBox->setValue(lastMaxValue);

  minSpinBoxValueChanged();
  maxSpinBoxValueChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::minSpinBoxValueChanged()
{
  spinBoxToSlider(minSpinBox, minSlider);

  if(checkMinSpinBox())
  {
    spinBoxToSlider(maxSpinBox, maxSlider);
  }

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::maxSpinBoxValueChanged()
{
  spinBoxToSlider(maxSpinBox, maxSlider);

  if(checkMaxSpinBox())
  {
    spinBoxToSlider(minSpinBox, minSlider);
  }

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::minSliderValueChanged()
{
  sliderToSpinBox(minSlider, minSpinBox);

  if(checkMinSpinBox())
  {
    spinBoxToSlider(maxSpinBox, maxSlider);
  }

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::maxSliderValueChanged()
{
  sliderToSpinBox(maxSlider, maxSpinBox);

  if(checkMaxSpinBox())
  {
    spinBoxToSlider(minSpinBox, minSlider);
  }

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::sliderToSpinBox(QSlider* slider, QDoubleSpinBox* spinBox)
{
  int sliderRange = slider->maximum() - slider->minimum();
  double spinBoxRange = spinBox->maximum() - spinBox->minimum();

  int sliderValue = slider->value() + slider->minimum();

  double percentage = sliderValue / (double)sliderRange;
  spinBox->setValue(percentage * spinBoxRange + spinBox->minimum());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkThresholdWidget::spinBoxToSlider(QDoubleSpinBox* spinBox, QSlider* slider)
{
  int sliderRange = slider->maximum() - slider->minimum();
  double spinBoxRange = spinBox->maximum() - spinBox->minimum();

  double spinBoxValue = spinBox->value() - spinBox->minimum();

  double percentage = spinBoxValue / spinBoxRange;
  slider->setValue(percentage * sliderRange + slider->minimum());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkThresholdWidget::checkMinSpinBox()
{
  if(minSpinBox->value() > maxSpinBox->value())
  {
    maxSpinBox->setValue(minSpinBox->value());
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VtkThresholdWidget::checkMaxSpinBox()
{
  if(minSpinBox->value() > maxSpinBox->value())
  {
    minSpinBox->setValue(maxSpinBox->value());
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkImplicitFunction> VtkThresholdWidget::getImplicitFunction()
{
  return nullptr;
}
