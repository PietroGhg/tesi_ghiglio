cmake_minimum_required(VERSION 3.5)


include_directories( ${OpenCV_INCLUDE_DIRS} )
  find_package( OpenCV REQUIRED )
# Compilation flags

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -Wextra -pedantic -DDEBUG -Wall --pedantic -std=c++11 -DGNEMU")



# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

set(MANGO_ROOT "/opt/mango" CACHE STRING "MANGO installation root directory")

set(smart_surveillance_SOURCES main.cpp Object.cpp kernel_runner.cpp)

add_executable (smart_surveillance ${smart_surveillance_SOURCES})


target_include_directories(smart_surveillance PUBLIC ${MANGO_ROOT}/include/libmango/)
target_include_directories(smart_surveillance PUBLIC ${MANGO_ROOT}/include/)

set(BOSP_PATH "/opt/mango/bosp")


target_include_directories(smart_surveillance PUBLIC ${BOSP_PATH}/include/bbque/)
target_include_directories(smart_surveillance PUBLIC ${BOSP_PATH}/include/)

target_link_libraries(smart_surveillance PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_pms.so ${OpenCV_LIBS})
target_link_libraries(smart_surveillance PUBLIC ${BOSP_PATH}/lib/bbque/libbbque_tg.so)


find_library(MANGO_LIBRARY
	NAMES mango
	HINTS "${MANGO_ROOT}/lib")



target_link_libraries(smart_surveillance PUBLIC ${MANGO_LIBRARY})


install(TARGETS smart_surveillance DESTINATION ${MANGO_ROOT}/usr/bin)
install(FILES surveillance.recipe DESTINATION ${BOSP_PATH}/etc/bbque/recipes)

add_subdirectory(motion_kernel)
add_subdirectory(classifier_kernel)
add_subdirectory(tracker_kernel)
add_subdirectory(gui_kernel)
