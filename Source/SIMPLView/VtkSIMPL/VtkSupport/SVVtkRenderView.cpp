/* ============================================================================
 * Copyright (c) 2017 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "SVVtkRenderView.h"

#include <vtkObjectBase.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkBorderRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkInformation.h>
#include <vtkInformationKey.h>
#include <vtkInformationObjectBaseKey.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <QVTKInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkDataSet.h>
#include <vtkOutlineSource.h>
#include <vtkDataSetMapper.h>

//#include "VtkSIMPL/VtkFilters/VtkAbstractFilter.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVCenterAxesActor.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVAxesActor.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVInteractorStyle.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballMultiRotate.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballZoomToMouse.h"
#include "VtkSIMPL/VTKExtensions/Rendering/vtkTrackballPan.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h"
#include "VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
vtkStandardNewMacro(SVVtkRenderView);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkRenderView::SVVtkRenderView() :
  vtkRenderViewBase ()
{
  this->CenterAxes = vtkPVCenterAxesActor::New();
  this->CenterAxes->SetComputeNormals(0);
  this->CenterAxes->SetPickable(0);
  this->CenterAxes->SetScale(0.25, 0.25, 0.25); 
  this->GetRenderer()->AddActor(this->CenterAxes);
  
  
  this->InteractionMode = INTERACTION_MODE_UNINTIALIZED;
  this->UseHiddenLineRemoval = false;
  this->GetRenderer()->SetUseDepthPeeling(1);
  
  this->SetInteractionMode(INTERACTION_MODE_3D);
  
  this->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
  
  this->InteractorStyle = // Default one will be the 3D
      this->ThreeDInteractorStyle = vtkPVInteractorStyle::New();
  this->TwoDInteractorStyle = vtkPVInteractorStyle::New();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVVtkRenderView::~SVVtkRenderView()
{
  if(this->CenterAxes) {
    this->CenterAxes->Delete();
  }
  if(this->OrientationAxes)
  {
    this->OrientationAxes->Delete();
  }
  if(this->OrientationWidget) {
    this->OrientationWidget->Delete();
  }
  
  if (this->InteractorStyle)
  {
    // Don't want to delete it as it is only pointing to either
    // [TwoDInteractorStyle, ThreeDInteractorStyle]
    this->InteractorStyle = 0;
  }
  if (this->TwoDInteractorStyle)
  {
    this->TwoDInteractorStyle->Delete();
    this->TwoDInteractorStyle = 0;
  }
  if (this->ThreeDInteractorStyle)
  {
    this->ThreeDInteractorStyle->Delete();
    this->ThreeDInteractorStyle = 0;
  }
  
}

//----------------------------------------------------------------------------
void SVVtkRenderView::InitializeRenderView(vtkGenericOpenGLRenderWindow *renWin)
{
  SetBackground(0.3, 0.3, 0.35);

  this->SetRenderWindow(renWin);
  
  QVTKInteractor* qVtkInterator = QVTKInteractor::New();
  vtkInteractorStyle* style = vtkInteractorStyleTrackballCamera::New();
  qVtkInterator->SetInteractorStyle(style);
  qVtkInterator->Initialize(); // This generally does nothing since we don't use a 3DConnexion Device.
  renWin->SetInteractor(qVtkInterator);
  
  // These next pair of "Delete" calls are because the classes that the objects
  // are passed into are avoiding an infinite destructive recursive call by directly
  // using the vtkSmartPointers "Register(...)" call to increment the reference
  // count of the object. By explicitly calling Delete() on this side we keep
  // the reference count where it should be. If we DO NOT call Delete() then a 
  // resource leak will occur. This may be NON STANDARD and the VTK source codes
  // and APIs should be looked at for each of the VTK classes that we use.
  qVtkInterator->Delete();
  style->Delete(); 
  
  this->OrientationAxes = vtkAxesActor::New();
  this->OrientationAxes->GetYAxisShaftProperty()->SetColor(1.0,1.0,0.0);
  this->OrientationAxes->GetYAxisTipProperty()->SetColor(1.0,1.0,0.0);
  this->OrientationAxes->GetZAxisShaftProperty()->SetColor(0.0,1.0,0.0);
  this->OrientationAxes->GetZAxisTipProperty()->SetColor(0.0,1.0,0.0);

  this->OrientationWidget = vtkOrientationMarkerWidget::New();
  this->OrientationWidget->SetOutlineColor(0.93, 0.57, 0.13);
  this->OrientationWidget->SetOrientationMarker(this->OrientationAxes);
  this->OrientationWidget->SetInteractor(renWin->GetInteractor());
  this->OrientationWidget->SetEnabled(1);
  this->OrientationWidget->InteractiveOff();  
  
}

//----------------------------------------------------------------------------
void SVVtkRenderView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "" << endl;
}


//----------------------------------------------------------------------------
void SVVtkRenderView::SetActiveCamera(vtkCamera* camera)
{
  this->GetRenderer()->SetActiveCamera(camera);
  //this->GetNonCompositedRenderer()->SetActiveCamera(camera);
  if (camera)
  {
    camera->SetParallelProjection(this->ParallelProjection);
  }
}

//----------------------------------------------------------------------------
vtkCamera* SVVtkRenderView::GetActiveCamera()
{
  return this->GetRenderer()->GetActiveCamera();
}


//----------------------------------------------------------------------------
void SVVtkRenderView::SetInteractionMode(int mode)
{
  if (this->InteractionMode != mode)
  {
    this->InteractionMode = mode;
    this->Modified();
    vtkRenderWindowInteractor* interator = GetInteractor();
    
    // If we're in a situation where we don't have an interactor (e.g. pvbatch or Catalyst)
    // we still want to set the other properties on the camera.
    switch (this->InteractionMode)
    {
      case INTERACTION_MODE_3D:
        if (interator)
        {
          interator->SetInteractorStyle(this->ThreeDInteractorStyle);
        }
        // Get back to the previous state
        this->GetActiveCamera()->SetParallelProjection(this->ParallelProjection);
        break;
      case INTERACTION_MODE_2D:
        if (interator)
        {
          interator->SetInteractorStyle(this->TwoDInteractorStyle);
        }
        this->GetActiveCamera()->SetParallelProjection(1);
        break;
        
      case INTERACTION_MODE_SELECTION:
        if (interator)
        {
          //  interator->SetInteractorStyle(this->RubberBandStyle);
        }
        break;
        
      case INTERACTION_MODE_POLYGON:
        if (interator)
        {
          //   interator->SetInteractorStyle(this->PolygonStyle);
        }
        break;
        
      case INTERACTION_MODE_ZOOM:
        if (interator)
        {
          //   interator->SetInteractorStyle(this->RubberBandZoom);
        }
        break;
    }
  }
}


//*****************************************************************
// Forwarded to orientation axes widget.

//----------------------------------------------------------------------------
void SVVtkRenderView::SetOrientationAxesInteractivity(bool v)
{
  this->OrientationWidget->SetEnabled(v);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetOrientationAxesVisibility(bool v)
{
  this->OrientationWidget->SetEnabled(v);
  GetRenderWindow()->Render();  
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetOrientationAxesLabelColor(double r, double g, double b)
{
  //this->OrientationWidget->SetAxisLabelColor(r, g, b);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetOrientationAxesOutlineColor(double r, double g, double b)
{
  this->OrientationWidget->SetOutlineColor(r, g, b);
}

//*****************************************************************
// Forwarded to center axes.
//----------------------------------------------------------------------------
void SVVtkRenderView::SetCenterAxesVisibility(bool v)
{
  this->CenterAxes->SetVisibility(v);
  if(v) 
  {
    this->GetRenderer()->AddActor(this->CenterAxes);
  }
  else
  {
    this->GetRenderer()->RemoveActor(this->CenterAxes);
  }
  GetRenderWindow()->Render();  
}

//*****************************************************************
// Forward to vtkPVInteractorStyle instances.
//----------------------------------------------------------------------------
void SVVtkRenderView::SetCenterOfRotation(double x, double y, double z)
{ 
  this->CenterAxes->SetPosition(x, y, z);
  if (this->TwoDInteractorStyle)
  {
    this->TwoDInteractorStyle->SetCenterOfRotation(x, y, z);
  }
  if (this->ThreeDInteractorStyle)
  {
    this->ThreeDInteractorStyle->SetCenterOfRotation(x, y, z);
  }
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetRotationFactor(double factor)
{
  if (this->TwoDInteractorStyle)
  {
    this->TwoDInteractorStyle->SetRotationFactor(factor);
  }
  if (this->ThreeDInteractorStyle)
  {
    this->ThreeDInteractorStyle->SetRotationFactor(factor);
  }
}


//*****************************************************************
// Forward to 3D renderer.
//----------------------------------------------------------------------------
void SVVtkRenderView::SetUseDepthPeeling(int val)
{
  this->GetRenderer()->SetUseDepthPeeling(val);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetMaximumNumberOfPeels(int val)
{
  this->GetRenderer()->SetMaximumNumberOfPeels(val);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetBackground(double r, double g, double b)
{
  this->GetRenderer()->SetBackground(r, g, b);
}
//----------------------------------------------------------------------------
void SVVtkRenderView::SetBackground2(double r, double g, double b)
{
  this->GetRenderer()->SetBackground2(r, g, b);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetBackgroundTexture(vtkTexture* val)
{
  this->GetRenderer()->SetBackgroundTexture(val);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetGradientBackground(int val)
{
  this->GetRenderer()->SetGradientBackground(val ? true : false);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetTexturedBackground(int val)
{
  this->GetRenderer()->SetTexturedBackground(val ? true : false);
}


//*****************************************************************
// Forwarded to vtkPVInteractorStyle if present on local processes.
//----------------------------------------------------------------------------
void SVVtkRenderView::SetCamera2DManipulators(const int manipulators[9])
{
  this->SetCameraManipulators(this->TwoDInteractorStyle, manipulators);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetCamera3DManipulators(const int manipulators[9])
{
  this->SetCameraManipulators(this->ThreeDInteractorStyle, manipulators);
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetCameraManipulators(vtkPVInteractorStyle* style, const int manipulators[9])
{
  if (!style)
  {
    return;
  }
  
  style->RemoveAllManipulators();
  enum
  {
    NONE = 0,
    SHIFT = 1,
    CTRL = 2
  };
  
  enum
  {
    PAN = 1,
    ZOOM = 2,
    ROLL = 3,
    ROTATE = 4,
    MULTI_ROTATE = 5,
    ZOOM_TO_MOUSE = 6
  };
  
  for (int manip = NONE; manip <= CTRL; manip++)
  {
    for (int button = 0; button < 3; button++)
    {
      int manipType = manipulators[3 * manip + button];
      vtkSmartPointer<vtkCameraManipulator> cameraManipulator;
      switch (manipType)
      {
        case PAN:
          //cameraManipulator = vtkSmartPointer<vtkTrackballPan>::New();
          break;
        case ZOOM:
          // cameraManipulator = vtkSmartPointer<vtkPVTrackballZoom>::New();
          break;
        case ROLL:
          // cameraManipulator = vtkSmartPointer<vtkPVTrackballRoll>::New();
          break;
        case ROTATE:
          //cameraManipulator = vtkSmartPointer<vtkPVTrackballRotate>::New();
          break;
        case MULTI_ROTATE:
          //cameraManipulator = vtkSmartPointer<vtkPVTrackballMultiRotate>::New();
          break;
        case ZOOM_TO_MOUSE:
          // cameraManipulator = vtkSmartPointer<vtkPVTrackballZoomToMouse>::New();
          break;
      }
      if (cameraManipulator)
      {
        cameraManipulator->SetButton(button + 1); // since button starts with 1.
        cameraManipulator->SetControl(manip == CTRL ? 1 : 0);
        cameraManipulator->SetShift(manip == SHIFT ? 1 : 0);
        style->AddManipulator(cameraManipulator);
      }
    }
  }
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetCamera2DMouseWheelMotionFactor(double factor)
{
  if (this->TwoDInteractorStyle)
  {
    this->TwoDInteractorStyle->SetMouseWheelMotionFactor(factor);
  }
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetCamera3DMouseWheelMotionFactor(double factor)
{
  if (this->ThreeDInteractorStyle)
  {
    this->ThreeDInteractorStyle->SetMouseWheelMotionFactor(factor);
  }
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkRenderView::getCameraFocalPointAndDistance(double* focalPoint, double& distance)
{
  double* newFocalPoint = GetRenderer()->GetActiveCamera()->GetFocalPoint();
  
  for(int i = 0; i < 3; i++)
  {
    focalPoint[i] = newFocalPoint[i];
  }
  
  double* position = GetRenderer()->GetActiveCamera()->GetPosition();
  distance = sqrt(pow(position[0] - focalPoint[0], 2) + pow(position[1] - focalPoint[1], 2) + pow(position[2] - focalPoint[2], 2));
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVVtkRenderView::setCameraPosition(CameraPosition position)
{
  GetRenderer()->ResetCamera();
  vtkCamera* camera = GetRenderer()->GetActiveCamera();
  double* focalPoint = new double[3];
  double distance;
  
  getCameraFocalPointAndDistance(focalPoint, distance);
  
  switch (position) {
    case CameraPosition::XPlus:
      camera->SetPosition(focalPoint[0] + distance, focalPoint[1], focalPoint[2]);
      camera->SetViewUp(0.0, 0.0, 1.0);
      break;
    case CameraPosition::XMinus:
      camera->SetPosition(focalPoint[0] - distance, focalPoint[1], focalPoint[2]);
      camera->SetViewUp(0.0, 0.0, 1.0);
      break;
    case CameraPosition::YPlus:
      camera->SetPosition(focalPoint[0], focalPoint[1] + distance, focalPoint[2]);
      camera->SetViewUp(0.0, 0.0, 1.0);
      break;
    case CameraPosition::YMinus:
      camera->SetPosition(focalPoint[0], focalPoint[1] - distance, focalPoint[2]);
      camera->SetViewUp(0.0, 0.0, 1.0);
      break;
    case CameraPosition::ZPlus:
      camera->SetPosition(focalPoint[0], focalPoint[1], focalPoint[2] - distance);
      camera->SetViewUp(0.0, 1.0, 0.0);
      break;
    case CameraPosition::ZMinus:
      camera->SetPosition(focalPoint[0], focalPoint[1], focalPoint[2] + distance);
      camera->SetViewUp(0.0, 1.0, 0.0);
      break;
  }
  delete[] focalPoint;
  
  GetInteractor()->Render();
}



//----------------------------------------------------------------------------
void SVVtkRenderView::UpdateCenterAxes()
{
  vtkBoundingBox bbox(this->GeometryBounds);

  // include the center of rotation in the axes size determination.
  bbox.AddPoint(this->CenterAxes->GetPosition());

  double widths[3];
  bbox.GetLengths(widths);

  // lets make some thickness in all directions
  //double diameterOverTen = bbox.GetMaxLength() > 0 ? bbox.GetMaxLength() / 10.0 : 1.0;
  //widths[0] = widths[0] < diameterOverTen ? diameterOverTen : widths[0];
  //widths[1] = widths[1] < diameterOverTen ? diameterOverTen : widths[1];
  //widths[2] = widths[2] < diameterOverTen ? diameterOverTen : widths[2];

  widths[0] *= 0.25;
  widths[1] *= 0.25;
  widths[2] *= 0.25;
  this->CenterAxes->SetScale(widths);

  double bounds[6];
  this->GeometryBounds.GetBounds(bounds);
//  if (this->GridAxes3DActor)
//  {
//    this->GridAxes3DActor->SetTransformedBounds(bounds);
//  }
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetGeometryBounds(double bounds[6], vtkMatrix4x4* matrix /*=NULL*/)
{
  if (matrix && vtkMath::AreBoundsInitialized(bounds))
  {
    double min_point[4] = { bounds[0], bounds[2], bounds[4], 1 };
    double max_point[4] = { bounds[1], bounds[3], bounds[5], 1 };
    matrix->MultiplyPoint(min_point, min_point);
    matrix->MultiplyPoint(max_point, max_point);
    double transformed_bounds[6];
    transformed_bounds[0] = min_point[0] / min_point[3];
    transformed_bounds[2] = min_point[1] / min_point[3];
    transformed_bounds[4] = min_point[2] / min_point[3];
    transformed_bounds[1] = max_point[0] / max_point[3];
    transformed_bounds[3] = max_point[1] / max_point[3];
    transformed_bounds[5] = max_point[2] / max_point[3];
    this->GeometryBounds.AddBounds(transformed_bounds);
  }
  else
  {
    this->GeometryBounds.SetBounds(bounds);
  }
}

//----------------------------------------------------------------------------
void SVVtkRenderView::SetGeometryVisible(size_t index)
{
  if(index >= m_DataSetRenderViewConfigs.size())
  {
    return;
  }
    GetRenderer()->ResetCamera();
    
  SVVtkDataSetRenderViewConfShPtr dataSetRenderConf = m_DataSetRenderViewConfigs[index];
  GetRenderer()->AddActor(dataSetRenderConf->getActor());
  double bb[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
  dataSetRenderConf->getDataSet()->GetBounds(bb);
  
  #if 0
  vtkSmartPointer<vtkOutlineSource> outline = vtkSmartPointer<vtkOutlineSource>::New();
  outline->SetBounds(bb);
  outline->GenerateFacesOff();
  
  vtkSmartPointer<vtkDataSetMapper> outlineMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  outlineMapper->SetInputConnection(outline->GetOutputPort());
  outlineMapper->ScalarVisibilityOff();
  
  vtkActor * actor = vtkActor::New();
  actor->SetMapper(outlineMapper);
  actor->GetProperty()->SetLineWidth(30.0);
  
  vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
  outlineActor->SetMapper(outlineMapper);
  outlineActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
 
  GetRenderer()->AddActor(outlineActor);
	#endif
	
  SetGeometryBounds(bb);
  
  double center[3] = { bb[0] + (bb[1] - bb[0])/2.0, 
                       bb[2] + (bb[3] - bb[2])/2.0, 
                       bb[4] + (bb[5] - bb[4])/2.0 };
  
//  std::cout << bb[0] << "\t" << bb[1] << "\t" << bb[2] << "\t" << bb[3] << "\t" << bb[4] << "\t" << bb[5] <<std::endl;               
//  std::cout << center[0] << "\t" << center[1] << "\t" << center[2] << std::endl;
  
  SetCenterOfRotation(center[0], center[1], center[2]);
  UpdateCenterAxes();
}


//----------------------------------------------------------------------------
void SVVtkRenderView::SetDataSetRenderViewConfigs(DataSetRenderViewConfContainer& configs)
{
  m_DataSetRenderViewConfigs = configs;
  size_t activeDataSet = 0;
  for(size_t i = 0; i < m_DataSetRenderViewConfigs.size(); i++)
  {
    if(m_DataSetRenderViewConfigs[i]->isActive())
    {
      activeDataSet = i;
      break;
    }
  }
}

//----------------------------------------------------------------------------
DataSetRenderViewConfContainer SVVtkRenderView::GetDataSetRenderViewConfigs()
{
  return m_DataSetRenderViewConfigs;
}

//----------------------------------------------------------------------------
void SVVtkRenderView::AddDataSetRenderViewConf(std::shared_ptr<SVVtkDataSetRenderViewConf> config)
{
  m_DataSetRenderViewConfigs.push_back(config);
}
//----------------------------------------------------------------------------
void SVVtkRenderView::RemoveDataSetRenderViewConf(size_t index)
{
  m_DataSetRenderViewConfigs.erase(m_DataSetRenderViewConfigs.begin() + index);
}
//----------------------------------------------------------------------------
void SVVtkRenderView::RemoveDataSetRenderViewConf(const std::string dataSetName)
{
    size_t size = m_DataSetRenderViewConfigs.size();
  for(size_t i = 0; i < size; i++)
  {
    SVVtkDataSetRenderViewConf* conf = m_DataSetRenderViewConfigs[i].get();
    if(conf->getName().compare(dataSetName) == 0)
    {
      RemoveDataSetRenderViewConf(i);
    }
  }
}
//----------------------------------------------------------------------------
void SVVtkRenderView::ClearRenderViewConfigs()
{
  m_DataSetRenderViewConfigs.clear();
}

    
