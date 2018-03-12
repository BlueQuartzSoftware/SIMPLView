set(VtkSIMPL_VtkFilters_SRCS
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkAbstractFilter.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkClipFilter.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkDataSetFilter.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkMaskFilter.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkSliceFilter.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkCropFilter.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkThresholdFilter.cpp
)

set(VtkSIMPL_VtkFilters_HDRS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkAbstractFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkClipFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkDataSetFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkMaskFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkSliceFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkCropFilter.h
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/VtkThresholdFilter.h
)

set(VtkSIMPL_VtkFilters_UIS
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkDataSetFilter.ui 
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkClipFilter.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkMaskFilter.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkSliceFilter.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkCropFilter.ui
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkFilters/UI_Files/VtkThresholdFilter.ui
)

cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/VtkFilters" "${VtkSIMPL_VtkFilters_HDRS}" "${VtkSIMPL_VtkFilters_SRCS}" "0")
