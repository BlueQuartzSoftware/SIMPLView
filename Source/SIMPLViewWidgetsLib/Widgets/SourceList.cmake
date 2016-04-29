
set(SIMPLViewWidgetsLib_Widgets_HDRS "")
set(SIMPLViewWidgetsLib_Widgets_SRCS "")
set(SIMPLViewWidgetsLib_Widgets_UIS "")


# --------------------------------------------------------------------
# List the Classes here that are QWidget Derived Classes
set(SIMPLView_WIDGETS
    BookmarksToolboxWidget
    DataContainerArrayWidget
    SIMPLViewUpdateCheckDialog
    DropBoxWidget
    FilterInputWidget
    FilterLibraryToolboxWidget
    FilterListToolboxWidget
    IssuesDockWidget
    StandardOutputDockWidget
    PipelineFilterWidget
    AboutPlugins
    PluginDetails
)

foreach(FPW ${SIMPLView_WIDGETS})
  set(SIMPLViewWidgetsLib_Widgets_MOC_HDRS ${SIMPLViewWidgetsLib_Widgets_MOC_HDRS}
    ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/${FPW}.h
    )
  set(SIMPLViewWidgetsLib_Widgets_SRCS ${SIMPLViewWidgetsLib_Widgets_SRCS}
    ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/${FPW}.cpp
    )
  set(SIMPLViewWidgetsLib_Widgets_UIS ${SIMPLViewWidgetsLib_Widgets_UIS}
    ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/UI_Files/${FPW}.ui
    )
endforeach()

# Add in the remaining sources that are actually widgets but are completely Custom and do NOT use private
# inheritance through a .ui file
set(SIMPLViewWidgetsLib_Widgets_MOC_HDRS
  ${SIMPLViewWidgetsLib_Widgets_MOC_HDRS}
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/DSplashScreen.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/PipelineViewWidget.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/FilterListWidget.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/FilterLibraryTreeWidget.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/DREAM3DListWidget.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksModel.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksTreeView.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksItemDelegate.h
)

set(SIMPLViewWidgetsLib_Widgets_HDRS
    ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksItem.h
)

set(SIMPLViewWidgetsLib_Widgets_SRCS
  ${SIMPLViewWidgetsLib_Widgets_SRCS}
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/DSplashScreen.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/PipelineViewWidget.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/DREAM3DListWidget.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/FilterListWidget.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/FilterLibraryTreeWidget.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksItem.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksModel.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksTreeView.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/BookmarksItemDelegate.cpp
  )

SET(SIMPLViewWidgetsLib_Widgets_Util_HDRS
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/MoveFilterCommand.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/AddFiltersCommand.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/RemoveFilterCommand.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/ClearFiltersCommand.h
)

SET(SIMPLViewWidgetsLib_Widgets_Util_SRCS
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/MoveFilterCommand.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/AddFiltersCommand.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/RemoveFilterCommand.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/util/ClearFiltersCommand.cpp
  )

cmp_IDE_SOURCE_PROPERTIES( "Widgets/util" "${SIMPLViewWidgetsLib_Widgets_Util_HDRS}" "${SIMPLViewWidgetsLib_Widgets_SRCS}" "${PROJECT_INSTALL_HEADERS}")
source_group("${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets util" FILES ${HEADERS} ${SOURCES})

cmp_IDE_SOURCE_PROPERTIES( "SIMPLViewWidgetsLib/Widgets" "${SIMPLViewWidgetsLib_Widgets_MOC_HDRS};${SIMPLViewWidgetsLib_Widgets_HDRS}" "${SIMPLViewWidgetsLib_Widgets_SRCS}" "0")

cmp_IDE_GENERATED_PROPERTIES("SIMPLViewWidgetsLib/Widgets/UI_Files" "${SIMPLViewWidgetsLib_Widgets_UIS}" "")

# --------------------------------------------------------------------
# and finally this will run moc:
QT5_WRAP_CPP( SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS ${SIMPLViewWidgetsLib_Widgets_MOC_HDRS} )
set_source_files_properties( ${SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS} PROPERTIES GENERATED TRUE)
set_source_files_properties( ${SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)


set(SIMPLViewWidgetsLib_Widgets_SRCS
  ${SIMPLViewWidgetsLib_Widgets_Util_SRCS}
  ${SIMPLViewWidgetsLib_Widgets_SRCS}
  ${SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS}
)

# -- Run MOC and UIC on the necessary files
# QT5_ADD_RESOURCES( SIMPLViewWidgetsLib_Generated_RC_SRCS "${SIMPLViewProj_SOURCE_DIR}/Documentation/Filters/Generated_FilterDocs.qrc"  )

# --------------------------------------------------------------------
# Continue on with our Qt4 section
QT5_WRAP_UI( SIMPLViewWidgetsLib_Widgets_Generated_UI_HDRS ${SIMPLViewWidgetsLib_Widgets_UIS} )

# --------------------------------------------------------------------
#-- Put the Qt generated files into their own group for IDEs
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Moc" "" "${SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Uic" "${SIMPLViewWidgetsLib_Widgets_Generated_UI_HDRS}" "" "0")
#cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${SIMPLViewWidgetsLib_Generated_RC_SRCS}" "" "0")




