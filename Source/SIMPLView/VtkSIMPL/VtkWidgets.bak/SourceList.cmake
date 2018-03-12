
set(VtkSIMPL_VtkWidgets_SRCS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkAbstractWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkPlaneWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkBoxWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkMaskWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkThresholdWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkCropWidget.cpp
)

set(VtkSIMPL_VtkWidgets_HDRS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkAbstractWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkPlaneWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkBoxWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkMaskWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkThresholdWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/VtkCropWidget.h
)

set(VtkSIMPL_VtkWidgets_UIS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/UI_Files/VtkPlaneWidget.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/UI_Files/VtkBoxWidget.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/UI_Files/VtkMaskWidget.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/UI_Files/VtkThresholdWidget.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkWidgets/UI_Files/VtkCropWidget.ui
)


cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/VtkWidgets" "${VtkSIMPL_VtkWidgets_HDRS}" "${VtkSIMPL_VtkWidgets_SRCS}" "0")
