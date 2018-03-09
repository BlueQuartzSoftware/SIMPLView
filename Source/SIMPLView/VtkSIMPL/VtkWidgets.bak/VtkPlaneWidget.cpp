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

#include "VtkPlaneWidget.h"

#include <vtkCommand.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkPlane.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkSmartPointer.h>

#include <QDoubleSpinBox>

#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"

#include "VtkSIMPL/VtkWidgets/moc_VtkPlaneWidget.cpp"

class vtkPlaneCallback : public vtkCommand
{
public:
  static vtkPlaneCallback* New()
  {
    return new vtkPlaneCallback();
  }

  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkImplicitPlaneWidget2* planeWidget = reinterpret_cast<vtkImplicitPlaneWidget2*>(caller);
    vtkImplicitPlaneRepresentation* rep = reinterpret_cast<vtkImplicitPlaneRepresentation*>(planeWidget->GetRepresentation());
    rep->GetPlane(this->plane);

    rep->DrawPlaneOn();

    qtPlaneWidget->updateSpinBoxes();
    qtPlaneWidget->modified();
  }

  vtkPlaneCallback()
  : plane(0)
  , qtPlaneWidget(0)
  {
  }
  vtkPlane* plane;
  VtkPlaneWidget* qtPlaneWidget;
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkPlaneWidget::VtkPlaneWidget(QWidget* parent, double bounds[6], vtkRenderWindowInteractor* iren)
: VtkAbstractWidget(parent, bounds, iren)
{
  setupUi(this);

  normal[0] = 1.0;
  normal[1] = 0.0;
  normal[2] = 0.0;

  viewPlane = vtkSmartPointer<vtkPlane>::New();
  viewPlane->SetNormal(normal);
  viewPlane->SetOrigin(origin);

  usePlane = vtkSmartPointer<vtkPlane>::New();
  usePlane->SetNormal(normal);
  usePlane->SetOrigin(origin);

  vtkSmartPointer<vtkPlaneCallback> myCallback = vtkSmartPointer<vtkPlaneCallback>::New();
  myCallback->plane = viewPlane;
  myCallback->qtPlaneWidget = this;

  // Implicit Plane Widget
  planeRep = vtkImplicitPlaneRepresentation::New();
  planeRep->SetPlaceFactor(1.25);
  planeRep->PlaceWidget(bounds);
  planeRep->SetNormal(viewPlane->GetNormal());
  planeRep->SetOrigin(viewPlane->GetOrigin());
  planeRep->SetScaleEnabled(0);
  planeRep->SetOutlineTranslation(0);
  planeRep->DrawPlaneOff();
  planeRep->SetInteractionState(vtkImplicitPlaneRepresentation::Pushing);

  planeWidget = vtkImplicitPlaneWidget2::New();
  planeWidget->SetInteractor(iren);
  planeWidget->SetRepresentation(planeRep);
  planeWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);

  updateSpinBoxes();

  // adjust the vtkWidget when values are changed
  connect(normalXSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(normalYSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(normalZSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));

  connect(originXSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(originYSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
  connect(originZSpinBox, SIGNAL(editingFinished()), this, SLOT(spinBoxValueChanged()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VtkPlaneWidget::~VtkPlaneWidget()
{
  planeRep->Delete();
  planeWidget->Delete();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::getNormals(double normals[3])
{
  for(int i = 0; i < 3; i++)
  {
    normals[i] = this->normal[i];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::setNormals(double normals[3])
{
  for(int i = 0; i < 3; i++)
  {
    this->normal[i] = normals[i];
  }

  viewPlane->SetNormal(this->normal);

  normalXSpinBox->setValue(normal[0]);
  normalYSpinBox->setValue(normal[1]);
  normalZSpinBox->setValue(normal[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::setNormals(double x, double y, double z)
{
  normal[0] = x;
  normal[1] = y;
  normal[2] = z;

  viewPlane->SetNormal(this->normal);

  normalXSpinBox->setValue(normal[0]);
  normalYSpinBox->setValue(normal[1]);
  normalZSpinBox->setValue(normal[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::setOrigin(double origin[3])
{
  VtkAbstractWidget::setOrigin(origin);

  viewPlane->SetOrigin(origin);

  originXSpinBox->setValue(origin[0]);
  originYSpinBox->setValue(origin[1]);
  originZSpinBox->setValue(origin[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::setOrigin(double x, double y, double z)
{
  VtkAbstractWidget::setOrigin(x, y, z);

  viewPlane->SetOrigin(origin);

  originXSpinBox->setValue(origin[0]);
  originYSpinBox->setValue(origin[1]);
  originZSpinBox->setValue(origin[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::updateBounds()
{
  planeWidget->EnabledOff();
  planeWidget->GetRepresentation()->PlaceWidget(bounds);
  planeWidget->EnabledOn();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::updateOrigin()
{
  viewPlane->SetOrigin(origin);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkSmartPointer<vtkImplicitFunction> VtkPlaneWidget::getImplicitFunction()
{
  usePlane->SetNormal(normal);
  usePlane->SetOrigin(origin);

  return usePlane;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::enable()
{
  planeWidget->EnabledOn();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::disable()
{
  planeWidget->EnabledOff();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::apply()
{
  origin[0] = originXSpinBox->value();
  origin[1] = originYSpinBox->value();
  origin[2] = originZSpinBox->value();

  normal[0] = normalXSpinBox->value();
  normal[1] = normalYSpinBox->value();
  normal[2] = normalZSpinBox->value();

  planeRep->DrawPlaneOff();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::reset()
{
  viewPlane->SetOrigin(usePlane->GetOrigin());
  viewPlane->SetNormal(usePlane->GetNormal());

  planeRep->DrawPlaneOff();
  planeRep->SetPlane(viewPlane);

  updateSpinBoxes();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::updateSpinBoxes()
{
  planeRep->GetNormal(normal);
  planeRep->GetOrigin(origin);

  normalXSpinBox->setValue(normal[0]);
  normalYSpinBox->setValue(normal[1]);
  normalZSpinBox->setValue(normal[2]);

  originXSpinBox->setValue(origin[0]);
  originYSpinBox->setValue(origin[1]);
  originZSpinBox->setValue(origin[2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::spinBoxValueChanged()
{
  normal[0] = normalXSpinBox->value();
  normal[1] = normalYSpinBox->value();
  normal[2] = normalZSpinBox->value();

  origin[0] = originXSpinBox->value();
  origin[1] = originYSpinBox->value();
  origin[2] = originZSpinBox->value();

  updatePlaneWidget();

  emit modified();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VtkPlaneWidget::updatePlaneWidget()
{
  planeWidget->Off();

  viewPlane->SetNormal(normal);
  viewPlane->SetOrigin(origin);

  planeRep->SetPlane(viewPlane);

  planeWidget->On();

  m_renderWindowInteractor->Render();
}
