#StrictCompile()

add_library(bongomsgapi SHARED
	msgaddr.c
	msgapi.c
	msgauth.c
	msgcollector.c
	msgdate.c
	msghttp.c
	sqldb.c
)

target_link_libraries(bongomsgapi
	bongoxpl
	bongocal
	bongojson
	${SQLITE_LIBRARIES}
	${LIBICAL_LIBRARIES}
	${CURL_LIBRARIES}
)

install(TARGETS bongomsgapi DESTINATION ${LIB_INSTALL_DIR})

add_subdirectory(auth-backends)
