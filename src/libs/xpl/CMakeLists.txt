StrictCompile()

add_library(bongoxpl SHARED
	dir.c
	dns.c
	fsys.c
	hash.c
	ip.c
	mem.c
	resolve.c
	rwlock.c
	util.c
)

target_link_libraries(bongoxpl
	resolv
	pthread
	${GCRYPT_LIBRARIES}
)

install(TARGETS bongoxpl DESTINATION ${LIB_INSTALL_DIR})
