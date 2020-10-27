# cmake file for a standalone program
#
# J. Dudouet
#
CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

set(CMAKE_INSTALL_MESSAGE LAZY)

#
# NAME OF THE PROGRAM : means main is in PROG_NAME.[c,cpp ...] and it gives exec with name PROG_NAME
SET(PROG_NAME covid-player)
SET(Lib_NAME ccovid-player_core)
#
PROJECT(${PROG_NAME})

#Set C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -Wno-unused-parameter -Wno-sign-compare -Wno-invalid-offsetof -Wno-unused-result" )

#Set ROOT
set (ROOT_FIND_COMPONENTS Core MathCore Hist Gui Gpad Graf RIO Matrix Physics Thread MultiProc Spectrum)
  find_file(ROOT_CONFIG ROOTConfig.cmake HINTS $ENV{ROOTSYS} ${ROOTSYS} ${ROOT_DIR} NO_DEFAULT_PATH PATH_SUFFIXES cmake)
if (ROOT_CONFIG)
  include(${ROOT_CONFIG})
  include(${ROOT_USE_FILE})
endif()


######################
### Link libraries ###
######################

# root
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

if ( NOT ROOT_FOUND )
MESSAGE( FATAL_ERROR "ROOT not found ==> EXIT")
else()
MESSAGE( "ROOT found.
        --> ROOTSYS=$ENV{ROOTSYS}")
endif()

INCLUDE_DIRECTORIES( ${ROOT_INCLUDE_DIR} )
LINK_DIRECTORIES( ${ROOT_LIBRARY_DIR} )
SET(EXTRA_EXTERNAL_LIBRARIES ${EXTRA_EXTERNAL_LIBRARIES} Core MathCore Hist Gui Gpad Graf Tree Rint RIO Proof TreePlayer Physics)

set( COVID_PLAYER_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib )
set( COVID_PLAYER_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin )
set( COVID_PLAYER_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include )
set( COVID_PLAYER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )

SET(covid_viewer_SRC_Dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET( PROG_NAME_SRCS ${covid_viewer_SRC_Dir}/covid_player.cpp )

include_directories(${covid_viewer_SRC_Dir})
file(GLOB covid_viewer_SRC ${covid_viewer_SRC_Dir}/*.cpp)
list(REMOVE_ITEM covid_viewer_SRC ${PROG_NAME_SRCS})

file(GLOB covid_viewer_HEAD ${covid_viewer_SRC_Dir}/*.h)
list(REMOVE_ITEM covid_viewer_HEAD ${covid_viewer_SRC_Dir}/LinkDef.h)

ROOT_GENERATE_DICTIONARY(G__${Lib_NAME} ${covid_viewer_HEAD} LINKDEF ${covid_viewer_SRC_Dir}/LinkDef.h )

add_library(${Lib_NAME} SHARED ${covid_viewer_SRC} ${covid_viewer_HEAD} G__${Lib_NAME}.cxx)
target_link_libraries(${Lib_NAME} ${EXTRA_INTERNAL_LIBRARIES} ${EXTRA_EXTERNAL_LIBRARIES})

#######################
### covid_viewer Program
#######################

add_executable(${PROG_NAME} ${PROG_NAME_SRCS})
target_link_libraries(${PROG_NAME} ${Lib_NAME})

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/covid-player-env.sh.cmake"
  "${CMAKE_BINARY_DIR}/covid-player-env.sh"
  @ONLY
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/covid-player-env.sh DESTINATION bin)

install(TARGETS ${PROG_NAME} ${Lib_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${Lib_NAME}_rdict.pcm DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${Lib_NAME}.rootmap DESTINATION lib)
install(FILES ${covid_viewer_HEAD} DESTINATION include COMPONENT headers)

file(GLOB database ${CMAKE_CURRENT_SOURCE_DIR}/database)
file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

install(DIRECTORY ${database} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${scripts} DESTINATION ${CMAKE_INSTALL_PREFIX})
