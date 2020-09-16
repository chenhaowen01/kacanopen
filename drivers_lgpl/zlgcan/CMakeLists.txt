cmake_minimum_required(VERSION 2.8)

message(STATUS "Configuring virtual driver")

set(LIB_NAME can_zlgcan)

file(GLOB LIB_SRC
	"*.cpp"
)

set(ZLGCAN_DIR "ZLGCAN_DIR-NOTFOUND" CACHE PATH "ZLGCAN library directory")

if (EXISTS ${ZLGCAN_DIR})
    set(ZLGCAN_INCLUDE_DIR ${ZLGCAN_DIR})
    set(ZLGCAN_LIBRARY_DIR ${ZLGCAN_DIR})
    message(STATUS "ZLGCAN_DIR ${ZLGCAN_DIR}")
    message(STATUS "ZLGCAN_INCLUDE_DIR ${ZLGCAN_INCLUDE_DIR}")
    message(STATUS "ZLGCAN_LIBRARY_DIR ${ZLGCAN_LIBRARY_DIR}")
else()
    message(STATUS "ZLGCAN_INCLUDE_DIR ${ZLGCAN_INCLUDE_DIR}")
    message(STATUS "ZLGCAN_LIBRARY_DIR ${ZLGCAN_LIBRARY_DIR}")
endif()

find_library(ZLGCAN_LIBS zlgcan ${ZLGCAN_LIBRARY_DIR})
if (EXISTS ${ZLGCAN_LIBS})
    message(STATUS "set ZLGCAN_LIBS to ${ZLGCAN_LIBS}")
else()
    message(FATAL_ERROR "ZLGCAN_LIBS: ${ZLGCAN_LIBS} is not exist.")
endif()

include_directories(${ZLGCAN_INCLUDE_DIR})
link_directories(${ZLGCAN_LIBRARY_DIR})

add_library(${LIB_NAME} SHARED ${LIB_SRC})
target_link_libraries(${LIB_NAME} ${ZLGCAN_LIBS})

# Install
kacanopen_install(${LIB_NAME} FALSE)
