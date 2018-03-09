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

#include "VtkCropWidget.h"

#include <vtkImageData.h>
#include <vtkImplicitFunction.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

#include <QSpinBox>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/VtkWidgets/moc_VtkCropWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkCropWidget::VtkCropWidget(QWidget* parent, vtkSmartPointer<vtkImageData> imageData, vtkRenderWindowInteractor* iren)
: VtkAbstractWidget(parent, imageData->GetBounds(), iren)
{
  setupUi(this);
  int imageExtent[6];
  imageData->GetExtent(imageExtent);

  m_imageData = imageData;

  voi = new int[6];
  sampleRate = new int[3]{1, 1, 1};

  for(int i = 0; i < 6; i++)
  {
    voi[i] = imageExtent[i];
  }

  xMinSpinBox->setMinimum(voi[0]);
  xMaxSpinBox->setMinimum(voi[0]);

  xMinSpinBox->setMaximum(voi[1]);
  xMaxSpinBox->setMaximum(voi[1]);

  yMinSpinBox->setMinimum(voi[2]);
  yMaxSpinBox->setMinimum(voi[2]);

  yMinSpinBox->setMaximum(voi[3]);
  yMaxSpinBox->setMaximum(voi[3]);

  zMinSpinBox->setMinimum(voi[4]);
  zMaxSpinBox->setMinimum(voi[4]);

  zMinSpinBox->setMaximum(voi[5]);
  zMaxSpinBox->setMaximum(voi[5]);

  updateSpinBoxes();

  connect(xMinSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
  connect(xMaxSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));

  connect(yMinSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
  connect(yMaxSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));

  connect(zMinSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
  connect(zMaxSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));

  connect(sampleISpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
  connect(sampleJSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
  connect(sampleKSpinBox, SIGNAL(editingFinished()), this, SIGNAL(modified()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkCropWidget::~VtkCropWidget()
{
  delete voi;
  delete sampleRate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int* VtkCropWidget::getVOI()
{
  return voi;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int* VtkCropWidget::getSampleRate()
{
  return sampleRate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkImplicitFunction> VtkCropWidget::getImplicitFunction()
{
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropWidget::enable()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropWidget::disable()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropWidget::apply()
{
  voi[0] = xMinSpinBox->value();
  voi[1] = xMaxSpinBox->value();
  voi[2] = yMinSpinBox->value();
  voi[3] = yMaxSpinBox->value();
  voi[4] = zMinSpinBox->value();
  voi[5] = zMaxSpinBox->value();

  sampleRate[0] = sampleISpinBox->value();
  sampleRate[1] = sampleJSpinBox->value();
  sampleRate[2] = sampleKSpinBox->value();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropWidget::reset()
{
  updateSpinBoxes();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkCropWidget::updateSpinBoxes()
{
  xMinSpinBox->setValue(voi[0]);
  xMaxSpinBox->setValue(voi[1]);

  yMinSpinBox->setValue(voi[2]);
  yMaxSpinBox->setValue(voi[3]);

  zMinSpinBox->setValue(voi[4]);
  zMaxSpinBox->setValue(voi[5]);

  sampleISpinBox->setValue(sampleRate[0]);
  sampleJSpinBox->setValue(sampleRate[1]);
  sampleKSpinBox->setValue(sampleRate[2]);
}
