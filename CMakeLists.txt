﻿# CMakeList.txt: CMake-Projekt für "NBodyCuda". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

project(NBody)


# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (main "NBody.cpp" "NBody.h" "Vec3.h" "OctTree.h" "OctTree.cpp")

FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


option(AUTO_SEARCH_AND_INCLUDE_OpenGL "You can activate this option or include OpenGL by yourself" ON)
option(AUTO_SEARCH_AND_INCLUDE_Glut "You can activate this option or include GLUT by yourself" ON)
option(AUTO_SEARCH_AND_INCLUDE_Glew "You can activate this option or include GLEW by yourself" ON)

# FIND AND INCLUDE OPENGL				
if(AUTO_SEARCH_AND_INCLUDE_OpenGL)
find_package(OpenGL REQUIRED)
include_directories( ${OpenGL_INCLUDE_DIR})
target_link_libraries(main ${OPENGL_LIBRARIES})
endif(AUTO_SEARCH_AND_INCLUDE_OpenGL)

# FIND AND INCLUDE GLUT 				 
if(AUTO_SEARCH_AND_INCLUDE_Glut)
find_package(GLUT REQUIRED)
include_directories( ${GLUT_INCLUDE_DIR})
target_link_libraries(main ${GLUT_LIBRARIES})        
endif(AUTO_SEARCH_AND_INCLUDE_Glut)

# FIND AND INCLUDE GLEW                                                   
if(AUTO_SEARCH_AND_INCLUDE_Glew)
find_package(GLEW REQUIRED)
include_directories( ${GLEW_INCLUDE_DIRS})
target_link_libraries(main ${GLEW_LIBRARIES})
else(AUTO_SEARCH_AND_INCLUDE_Glew)
# At least define the glew library if the package is not searched       
    IF (WIN32)
    set(GLEW_LIBRARY GLEW32)
    ELSE (WIN32)
    set(GLEW_LIBRARY GLEW)
    ENDIF(WIN32)
    target_link_libraries(main ${GLEW_LIBRARY})
endif(AUTO_SEARCH_AND_INCLUDE_Glew)






# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
