cmake_minimum_required(VERSION 3.5)
PROJECT(Showmesh)

# Find FLTK
FIND_PACKAGE(FLTK REQUIRED)
IF(FLTK_FOUND)
   INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
ENDIF(FLTK_FOUND)

# Look for OpenGL.
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_INCLUDE_PATH)
  INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_PATH})
ENDIF(OPENGL_INCLUDE_PATH)

# Look for PNG
FIND_PACKAGE(PNG REQUIRED)
IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ENDIF(PNG_FOUND)

ADD_CUSTOM_COMMAND(
	OUTPUT showmeshui.cxx showmeshui.h
	COMMAND ${FLTK_FLUID_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/showmeshui.fl
	DEPENDS showmeshui.fl
	MAIN_DEPENDENCY showmeshui.fl
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(Showmesh ${CMAKE_CURRENT_BINARY_DIR}/showmeshui.cxx showmesh.cxx gluttext.cxx mesh.cxx gl2ps.c
	point3.cxx meshrender.cxx glcapture.cxx meshbase.cxx strlcpy.c
	main.cxx command.cxx meshproc.cxx scache.cxx)
TARGET_LINK_LIBRARIES(Showmesh ${PNG_LIBRARY} ${FLTK_LIBRARIES} ${OPENGL_LIBRARIES} z)
