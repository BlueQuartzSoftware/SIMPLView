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

#ifndef _VtkPlaneWidget_h_
#define _VtkPlaneWidget_h_

#include "VtkSIMPL/VtkWidgets/VtkAbstractWidget.h"
#include "ui_VtkPlaneWidget.h"

#include <vtkSmartPointer.h>

class vtkPlane;
class vtkImplicitPlaneWidget2;
class vtkImplicitPlaneRepresentation;

/**
 * @class VtkPlaneWidget VtkPlaneWidget.h 
 * SIMPLView/VtkSIMPL/VtkWidgets/VtkPlaneWidget.h
 * @brief This class handles the management of vtkImplicitPlaneWidget2 for 
 * VtkAbstractFilters that use plane-type VTK widgets for their execution. 
 * This class inherits from QWidget and displays editable values representing 
 * the normal and origin of the plane. Editing these values also changes the 
 * vtkImplicitPlaneWidget2 orientation.
 */
class VtkPlaneWidget : public VtkAbstractWidget, private Ui::VtkPlaneWidget
{
  Q_OBJECT

public:
  /**
  * @brief Constructor
  * @param parent
  * @param bounds
  * @param iren
  */
  VtkPlaneWidget(QWidget* parent, double bounds[6], vtkRenderWindowInteractor* iren);

  /**
  * @brief Deconstructor
  */
  ~VtkPlaneWidget();

  /**
  * @brief Copies the normals into the double array passed in
  * @param normals
  */
  void getNormals(double normals[3]);

  /**
  * @brief Sets the normals to the given value
  * @param normals
  */
  void setNormals(double normals[3]);

  /**
  * @brief Sets the normals to match the given values
  * @param x
  * @param y
  * @param z
  */
  void setNormals(double x, double y, double z);

  /**
  * @brief Sets the origin to the given value
  * @param origin
  */
  void setOrigin(double origin[3]) override;

  /**
  * @brief Sets the origin to match the given values
  * @param x
  * @param y
  * @param z
  */
  void setOrigin(double x, double y, double z) override;

  /**
  * @brief Returns the vtkImplicitFunction used by the widget
  * @return
  */
  vtkSmartPointer<vtkImplicitFunction> getImplicitFunction() override;

  /**
  * @brief Enables the plane widget
  */
  void enable() override;

  /**
  * @brief Disables the plane widget
  */
  void disable() override;

  /**
  * @brief Copies the origin and normal values to the spin boxes
  */
  void apply() override;

  /**
  * @brief Resets the VTK plane to the last used normal and origin before 
  * resetting the input widgets.
  */
  void reset() override;

  /**
  * @brief Updates the origin and normal values based on the VTK plane widget 
  * before applying those values to the input widgets.
  */
  void updateSpinBoxes();

public slots:
  /**
  * @brief Updates the VTK plane widget with the input widget values.
  */
  void spinBoxValueChanged();

protected:
  /**
  * @brief Updates the bounds representation for the VTK plane widget.
  */
  void updateBounds() override;

  /**
  * @brief Updates the origin for the VTK plane
  */
  void updateOrigin() override;

  /**
  * @brief Updates the VTK plane for the implicit function
  */
  void updatePlaneWidget();

private:
  double normal[3];

  vtkSmartPointer<vtkPlane> usePlane;
  vtkSmartPointer<vtkPlane> viewPlane;
  vtkImplicitPlaneWidget2* planeWidget;
  vtkImplicitPlaneRepresentation* planeRep;
};

#endif
