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

#ifndef _VtkBoxWidget_h_
#define _VtkBoxWidget_h_

#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "ui_VtkBoxWidget.h"

#include <vtkSmartPointer.h>

class vtkTransform;
class vtkBox;
class vtkPlanes;
class vtkBoxWidget2;
class vtkBoxRepresentation;

class VtkBoxWidget : public VtkAbstractWidget, private Ui::VtkBoxWidget
{
  Q_OBJECT

public:
  VtkBoxWidget(QWidget* parent, double bounds[6], vtkRenderWindowInteractor* iren);
  ~VtkBoxWidget();

  void setScale(double scale[3]);
  void setScale(double x, double y, double z);

  void setRotation(double rotation[3]);
  void setRotation(double x, double y, double z);

  void setOrigin(double origin[3]) override;
  void setOrigin(double x, double y, double z) override;

  vtkSmartPointer<vtkImplicitFunction> getImplicitFunction() override;

  void enable() override;
  void disable() override;

  void apply() override;
  void reset() override;

  void updateSpinBoxes();

public slots:

  void spinBoxValueChanged();

protected:
  void updateBounds() override;
  void updateOrigin() override;

  void updateBoxWidget();

  void getPosition(double position[3]);
  void getScale(double scale[3]);
  void getRotation(double rotation[3]);

  void setMatrix(double position[3], double scale[3], double rotation[3]);

private:
  vtkSmartPointer<vtkTransform> viewTransform;
  vtkSmartPointer<vtkTransform> useTransform;

  vtkSmartPointer<vtkPlanes> viewPlanes;

  // vtkSmartPointer<vtkBox> viewBox;
  vtkSmartPointer<vtkBoxWidget2> boxWidget;
  vtkSmartPointer<vtkBoxRepresentation> boxRep;
};

#endif
