configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bongo-queuetool ${CMAKE_CURRENT_BINARY_DIR}/bongo-queuetool)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bongo-queuetool DESTINATION ${SBIN_INSTALL_DIR})
