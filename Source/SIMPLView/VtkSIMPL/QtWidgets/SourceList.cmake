
set(VtkSIMPL_Widgets_HDRS 
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/SelectVolumeWidget.h
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkEditFiltersWidget.h
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkMappingData.h
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkToggleWidget.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkVisualizationWidget.h
  )

set(VtkSIMPL_Widgets_SRCS
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/SelectVolumeWidget.cpp
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkEditFiltersWidget.cpp
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkMappingData.cpp
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkToggleWidget.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/VtkVisualizationWidget.cpp
  )

set(VtkSIMPL_Widgets_UIS 
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/UI_Files/SelectVolumeWidget.ui
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/UI_Files/VtkEditFiltersWidget.ui
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/UI_Files/VtkMappingData.ui
  # ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/UI_Files/VtkToggleWidget.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/QtWidgets/UI_Files/VtkVisualizationWidget.ui
  )

cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/QtWidgets" "${VtkSIMPL_Widgets_HDRS}" "${VtkSIMPL_Widgets_SRCS}" "0")
