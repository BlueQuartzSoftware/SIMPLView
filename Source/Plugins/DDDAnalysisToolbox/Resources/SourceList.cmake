QT5_ADD_RESOURCES( ${PLUGIN_NAME}_Generated_RC_SRCS "${${PLUGIN_NAME}_SOURCE_DIR}/Resources/${PLUGIN_NAME}.qrc"  )
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${${PLUGIN_NAME}_Generated_RC_SRCS}" "" "0")
