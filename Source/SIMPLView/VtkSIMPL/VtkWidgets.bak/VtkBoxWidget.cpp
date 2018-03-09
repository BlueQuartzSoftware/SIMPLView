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

#include "VtkBoxWidget.h"

#include <vtkBox.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>

#include <vtkSmartPointer.h>

#include <QtWidgets/QDoubleSpinBox>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/VtkWidgets/moc_VtkBoxWidget.cpp"

#define PI 3.14159

class vtkBoxCallback : public vtkCommand
{
public:
  static vtkBoxCallback* New()
  {
    return new vtkBoxCallback();
  }

  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkBoxWidget2* boxWidget = reinterpret_cast<vtkBoxWidget2*>(caller);
    vtkBoxRepresentation* rep = reinterpret_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation());
    rep->GetTransform(transform);

    qtBoxWidget->updateSpinBoxes();
    qtBoxWidget->modified();
  }

  vtkBoxCallback()
  : transform(0)
  , qtBoxWidget(0)
  {
  }
  vtkTransform* transform;
  VtkBoxWidget* qtBoxWidget;
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkBoxWidget::VtkBoxWidget(QWidget* parent, double bounds[6], vtkRenderWindowInteractor* iren)
: VtkAbstractWidget(parent, bounds, iren)
{
  setupUi(this);

  viewTransform = vtkSmartPointer<vtkTransform>::New();
  useTransform = vtkSmartPointer<vtkTransform>::New();

  viewPlanes = vtkSmartPointer<vtkPlanes>::New();

  vtkSmartPointer<vtkBoxCallback> myCallback = vtkSmartPointer<vtkBoxCallback>::New();
  myCallback->transform = viewTransform;
  myCallback->qtBoxWidget = this;

  // Implicit Plane Widget
  boxRep = vtkSmartPointer<vtkBoxRepresentation>::New();
  boxRep->SetPlaceFactor(1.25);
  boxRep->PlaceWidget(bounds);
  boxRep->SetTransform(viewTransform);

  boxWidget = vtkSmartPointer<vtkBoxWidget2>::New();
  boxWidget->SetInteractor(iren);
  boxWidget->SetRepresentation(boxRep);
  boxWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);

  updateSpinBoxes();

  // adjust the vtkWidget when values are changed
  connect(translationXSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(translationYSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(translationZSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));

  connect(rotationXSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(rotationYSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(rotationZSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));

  connect(scaleXSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(scaleYSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(scaleZSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkBoxWidget::~VtkBoxWidget()
{
  boxWidget->EnabledOff();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::setScale(double scale[3])
{
  viewTransform->Scale(scale);

  scaleXSpinBox->setValue(scale[0]);
  scaleYSpinBox->setValue(scale[1]);
  scaleZSpinBox->setValue(scale[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::setScale(double x, double y, double z)
{
  double scale[3] = {x, y, z};

  setScale(scale);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::setOrigin(double origin[3])
{
  VtkAbstractWidget::setOrigin(origin);

  viewTransform->Translate(origin);

  translationXSpinBox->setValue(origin[0]);
  translationYSpinBox->setValue(origin[1]);
  translationZSpinBox->setValue(origin[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::setOrigin(double x, double y, double z)
{
  double position[3] = {x, y, z};
  setOrigin(position);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::updateBounds()
{
  boxWidget->EnabledOff();
  boxWidget->GetRepresentation()->PlaceWidget(bounds);
  boxWidget->EnabledOn();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::updateOrigin()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkImplicitFunction> VtkBoxWidget::getImplicitFunction()
{
  vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
  boxRep->GetPlanes(planes);
  return planes;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::enable()
{
  boxWidget->EnabledOn();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::disable()
{
  boxWidget->EnabledOff();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::apply()
{
  vtkMatrix4x4* matrix = viewTransform->GetMatrix();
  useTransform->SetMatrix(matrix);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::reset()
{
  viewTransform->SetMatrix(useTransform->GetMatrix());

  boxRep->SetTransform(viewTransform);

  updateSpinBoxes();
  m_renderWindowInteractor->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::updateSpinBoxes()
{
  double scale[3];
  double rotation[3];

  getPosition(origin);
  getScale(scale);
  viewTransform->GetOrientation(rotation);

  translationXSpinBox->setValue(origin[0]);
  translationYSpinBox->setValue(origin[1]);
  translationZSpinBox->setValue(origin[2]);

  scaleXSpinBox->setValue(scale[0]);
  scaleYSpinBox->setValue(scale[1]);
  scaleZSpinBox->setValue(scale[2]);

  rotationXSpinBox->setValue(rotation[0]);
  rotationYSpinBox->setValue(rotation[1]);
  rotationZSpinBox->setValue(rotation[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::spinBoxValueChanged()
{
  origin[0] = translationXSpinBox->value();
  origin[1] = translationYSpinBox->value();
  origin[2] = translationZSpinBox->value();

  double scale[3];
  scale[0] = scaleXSpinBox->value();
  scale[1] = scaleYSpinBox->value();
  scale[2] = scaleZSpinBox->value();

  double rotation[3];
  rotation[0] = rotationXSpinBox->value();
  rotation[1] = rotationYSpinBox->value();
  rotation[2] = rotationZSpinBox->value();

  setMatrix(origin, scale, rotation);
  updateBoxWidget();

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::updateBoxWidget()
{
  boxRep->SetTransform(viewTransform);

  m_renderWindowInteractor->Render();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::getPosition(double position[3])
{
  vtkMatrix4x4* matrix = viewTransform->GetMatrix();

  position[0] = matrix->GetElement(0, 3) / 2.0;
  position[1] = matrix->GetElement(1, 3) / 2.0;
  position[2] = matrix->GetElement(2, 3) / 2.0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::getScale(double scale[3])
{
  vtkMatrix4x4* matrix = viewTransform->GetMatrix();

  scale[0] = sqrt(matrix->GetElement(0, 0));
  scale[1] = sqrt(matrix->GetElement(1, 1));
  scale[2] = sqrt(matrix->GetElement(2, 2));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::getRotation(double rotation[3])
{
  viewTransform->GetOrientation(rotation);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkBoxWidget::setMatrix(double position[3], double scale[3], double rotation[3])
{
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  viewTransform->SetMatrix(matrix);

  viewTransform->RotateZ(rotation[2]);
  viewTransform->RotateX(rotation[0]);
  viewTransform->RotateY(rotation[1]);

  viewTransform->Translate(position);
  viewTransform->Scale(scale);
}
