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
#ifndef _smvtkrenderview_H_
#define _smvtkrenderview_H_

#include <vector>

#include <vtkNew.h>
#include <vtkRenderViewBase.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>
#include <vtkBoundingBox.h>

// VTK Classes
class vtkTexture;
class vtkCamera;
class vtkScalarBarWidget;
class vtkBorderRepresentation;
class vtkInformation;
class vtkMatrix4x4;
class vtkOrientationMarkerWidget;
class vtkGenericOpenGLRenderWindow;
class vtkAxesActor;

// PVExtensions
class vtkPVCenterAxesActor;
class vtkPVInteractorStyle;
class vtkInteractorStyleRubberBand3D;
class vtkInteractorStyleRubberBandZoom;
class vtkInteractorStyleDrawPolygon;

// SIMPLView Class Forward Declare
class VtkAbstractFilter;
class SVVtkDataSetRenderViewConf;

using SVVtkDataSetRenderViewConfShPtr = std::shared_ptr<SVVtkDataSetRenderViewConf>;
using DataSetRenderViewConfContainer = std::vector<SVVtkDataSetRenderViewConfShPtr>;

/**
* @brief The SVVtkRenderView class
*/
class SVVtkRenderView : public vtkRenderViewBase
{
  public:
    static SVVtkRenderView* New();
    vtkTypeMacro(SVVtkRenderView, vtkRenderViewBase) void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
    
    enum InteractionModes
    {
      INTERACTION_MODE_UNINTIALIZED = -1,
      INTERACTION_MODE_3D = 0,
      INTERACTION_MODE_2D, // not implemented yet.
      INTERACTION_MODE_SELECTION,
      INTERACTION_MODE_ZOOM,
      INTERACTION_MODE_POLYGON
    };
    
    using EnumType = int;
    
    enum class CameraPosition : EnumType
    {
      XPlus = 0,  //!<
      XMinus = 1, //!<
      YPlus = 2,  //!<
      YMinus = 3, //!<
      ZPlus = 4,  //!<
      ZMinus = 5  //!<
    };
    
    void InitializeRenderView(vtkGenericOpenGLRenderWindow* renWin);


    virtual void SetInteractionMode(int mode);
    vtkGetMacro(InteractionMode, int)
    
    /**
    * Get/Set the active camera. The active camera is set on both the composited
    * and non-composited renderer.
    */
    vtkCamera* GetActiveCamera();
    virtual void SetActiveCamera(vtkCamera*);
    
    // *****************************************************************
    // Forwarded to orientation axes widget.
    virtual void SetOrientationAxesInteractivity(bool);
    virtual void SetOrientationAxesVisibility(bool);
    void SetOrientationAxesLabelColor(double r, double g, double b);
    void SetOrientationAxesOutlineColor(double r, double g, double b);
    
    // *****************************************************************
    // Forwarded to center axes.
    virtual void SetCenterAxesVisibility(bool);
    
    // *****************************************************************
    // Forward to vtkPVInteractorStyle instances.
    virtual void SetCenterOfRotation(double x, double y, double z);
    virtual void SetRotationFactor(double factor);
    
    // *****************************************************************
    // Forward to 3D renderer.
    vtkSetMacro(UseHiddenLineRemoval, bool) virtual void SetUseDepthPeeling(int val);
    virtual void SetMaximumNumberOfPeels(int val);
    virtual void SetBackground(double r, double g, double b);
    virtual void SetBackground2(double r, double g, double b);
    virtual void SetBackgroundTexture(vtkTexture* val);
    virtual void SetGradientBackground(int val);
    virtual void SetTexturedBackground(int val);
    
    // *****************************************************************
    // Forwarded to vtkPVInteractorStyle if present on local processes.
    virtual void SetCamera2DManipulators(const int manipulators[9]);
    virtual void SetCamera3DManipulators(const int manipulators[9]);
    void SetCameraManipulators(vtkPVInteractorStyle* style, const int manipulators[9]);
    virtual void SetCamera2DMouseWheelMotionFactor(double factor);
    virtual void SetCamera3DMouseWheelMotionFactor(double factor);
    
    // *****************************************************************
    // Set the various preset camera positions
    void getCameraFocalPointAndDistance(double* focalPoint, double& distance);
    void setCameraPosition(CameraPosition position);
    
    /**
   * UpdateCenterAxes().
   * Updates CenterAxes's scale and position.
   */
    void UpdateCenterAxes();
    
    void SetGeometryBounds(double bounds[6], vtkMatrix4x4* transform = NULL);
 
    void SetGeometryVisible(size_t index);
   
    void SetDataSetRenderViewConfigs(DataSetRenderViewConfContainer& configs);
    DataSetRenderViewConfContainer GetDataSetRenderViewConfigs();
    
    void AddDataSetRenderViewConf(std::shared_ptr<SVVtkDataSetRenderViewConf> config);
    void RemoveDataSetRenderViewConf(size_t index);
    void RemoveDataSetRenderViewConf(const std::string dataSetName);
    void ClearRenderViewConfigs();
 
    
  protected:
    SVVtkRenderView();
    ~SVVtkRenderView() override;
    
  private:
    int InteractionMode;
    bool ParallelProjection = false;
    
    //    vtkInteractorStyleRubberBand3D* RubberBandStyle = nullptr;
    //    vtkInteractorStyleRubberBandZoom* RubberBandZoom = nullptr;
    //    vtkInteractorStyleDrawPolygon* PolygonStyle = nullptr;
    
    vtkPVCenterAxesActor* CenterAxes = nullptr;
    vtkAxesActor*         OrientationAxes = nullptr;
    vtkOrientationMarkerWidget* OrientationWidget = nullptr;
    
    // 2D and 3D interactor style
    vtkPVInteractorStyle* TwoDInteractorStyle = nullptr;
    vtkPVInteractorStyle* ThreeDInteractorStyle = nullptr;
    
    // Active interactor style either [TwoDInteractorStyle, ThreeDInteractorStyle]
    vtkPVInteractorStyle* InteractorStyle = nullptr;
    
    vtkBoundingBox GeometryBounds;
    
    // Cached state. Is currently ignored for distributed rendering.
    bool UseHiddenLineRemoval = true;
    
    DataSetRenderViewConfContainer m_DataSetRenderViewConfigs;
        
    SVVtkRenderView(const SVVtkRenderView&) = delete;
    void operator=(const SVVtkRenderView&) = delete;
};

#endif /* _smvtkrenderview_H_ */
