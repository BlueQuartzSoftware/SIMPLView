
set(VtkSIMPL_VTKExtensions_Default_SRCS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Default/vtkPVGridAxes3DActor.cxx

)

set(VtkSIMPL_VTKExtensions_Default_HDRS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VTKExtensions/Default/vtkPVGridAxes3DActor.h
)

set(VtkSIMPL_VTKExtensions_Default_UIS

)


cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/VTKExtensions/Default" "${VtkSIMPL_VTKExtensions_Default_HDRS}" "${VtkSIMPL_VTKExtensions_Default_SRCS}" "0")
