
set(Main_File main.c)


set(ParserArgs GN)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories(${CMAKE_SOURCE_DIR})
set_source_files_properties(${Main_File}
                    PROPERTIES GENERATED TRUE)

add_custom_target (tracker_kernel_main_gen
        COMMAND ${MANGO_ROOT}/bin/mango_gen_kernel_entry.py ${ParserArgs} ${CMAKE_CURRENT_SOURCE_DIR}/tracker_kernel.cpp
        COMMENT "Generating main.c..."
        WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)




	add_executable(tracker_kernel tracker_kernel.cpp ${Main_File} ${CMAKE_SOURCE_DIR}/Object.cpp Result.cpp Tracker.cpp)
	find_library(MANGODEV mango-dev-gn PATHS ${MANGO_ROOT}/lib)
	find_package(Threads)
  find_package( OpenCV REQUIRED )
	target_link_libraries(tracker_kernel ${MANGODEV} ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS})
	target_include_directories(tracker_kernel PUBLIC ${MANGO_ROOT}/include/libmango/)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tracker_kernel
        DESTINATION ${MANGO_ROOT}/usr/local/share/smart_surveillance/tracker_kernel/
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)


        add_dependencies (smart_surveillance tracker_kernel)
        add_dependencies (tracker_kernel tracker_kernel_main_gen)
