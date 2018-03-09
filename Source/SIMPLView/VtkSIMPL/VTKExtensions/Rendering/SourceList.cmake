
set(VtkSIMPL_VTKExtensions_Rendering_SRCS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVCenterAxesActor.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVAxesWidget.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVAxesActor.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVInteractorStyle.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkCameraManipulator.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkCameraManipulatorGUIHelper.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballMultiRotate.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballPan.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballRoll.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballRotate.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballZoom.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballZoomToMouse.cxx
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkTrackballPan.cxx
)

set(VtkSIMPL_VTKExtensions_Rendering_HDRS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVCenterAxesActor.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVAxesWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVAxesActor.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVInteractorStyle.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkCameraManipulator.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkCameraManipulatorGUIHelper.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballMultiRotate.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballPan.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballRoll.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballRotate.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballZoom.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkPVTrackballZoomToMouse.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Rendering/vtkTrackballPan.h
)
set(VtkSIMPL_VTKExtensions_Rendering_UIS

)


cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/VTKExtensions/Rendering" "${VtkSIMPL_VTKExtensions_Rendering_HDRS}" "${VtkSIMPL_VTKExtensions_Rendering_SRCS}" "0")
