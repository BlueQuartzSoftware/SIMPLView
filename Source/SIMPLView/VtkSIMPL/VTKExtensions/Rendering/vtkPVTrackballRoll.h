/*=========================================================================

  Program:   ParaView
  Module:    vtkPVTrackballRoll.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVTrackballRoll
 * @brief   Rolls camera arround a point.
 *
 * vtkPVTrackballRoll allows the user to interactively
 * manipulate the camera, the viewpoint of the scene.
 * Roll tracks the mouse around the center of rotation.
*/

#ifndef vtkPVTrackballRoll_h
#define vtkPVTrackballRoll_h

#include "vtkCameraManipulator.h"

class  vtkPVTrackballRoll : public vtkCameraManipulator
{
public:
  static vtkPVTrackballRoll* New();
  vtkTypeMacro(vtkPVTrackballRoll, vtkCameraManipulator);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //@{
  /**
   * Event bindings controlling the effects of pressing mouse buttons
   * or moving the mouse.
   */
  void OnMouseMove(int x, int y, vtkRenderer* ren, vtkRenderWindowInteractor* rwi) VTK_OVERRIDE;
  void OnButtonDown(int x, int y, vtkRenderer* ren, vtkRenderWindowInteractor* rwi) VTK_OVERRIDE;
  void OnButtonUp(int x, int y, vtkRenderer* ren, vtkRenderWindowInteractor* rwi) VTK_OVERRIDE;
  //@}

protected:
  vtkPVTrackballRoll();
  ~vtkPVTrackballRoll() override;

  vtkPVTrackballRoll(const vtkPVTrackballRoll&) = delete;
  void operator=(const vtkPVTrackballRoll&) = delete;
};

#endif
