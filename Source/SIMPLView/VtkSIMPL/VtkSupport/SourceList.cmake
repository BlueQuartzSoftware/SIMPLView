set(VtkSIMPL_VtkSupport_SRCS
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SIMPLVtkBridge.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkRenderView.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkEdgeGeom.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkQuadGeom.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkTetrahedralGeom.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkTriangleGeom.cpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkVertexGeom.cpp
	# ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkRenderController.cpp
  ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkLookupTableController.cpp
)

set(VtkSIMPL_VtkSupport_HDRS
	#${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SIMPLVtkArray.hpp
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SIMPLVtkBridge.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkRenderView.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkDataSetRenderViewConf.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/SVVtkDataArrayRenderViewConf.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkEdgeGeom.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkQuadGeom.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkTetrahedralGeom.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkTriangleGeom.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkVertexGeom.h
	# ${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkRenderController.h
	${SIMPLView_SOURCE_DIR}/VtkSIMPL/VtkSupport/VtkLookupTableController.h
)

cmp_IDE_SOURCE_PROPERTIES( "${PROJECT_NAME}/VtkSIMPL/VtkSupport" "${VtkSIMPL_VtkSupport_HDRS}" "${VtkSIMPL_VtkSupport_SRCS}" "0")
