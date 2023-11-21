#
# Find the ARGoS package
#
if(ARGOS_BUILD_FOR_SIMULATOR)
  find_package(PkgConfig)
  pkg_check_modules(ARGOS REQUIRED argos3_simulator)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
  include_directories(${ARGOS_INCLUDE_DIRS})
  link_directories(${ARGOS_LIBRARY_DIRS})
elseif(ARGOS_BUILD_FOR_LOCALRVR)
  find_package(PkgConfig)
  pkg_check_modules(ARGOS REQUIRED argos3_localrvr)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ARGOS_PREFIX}/share/argos3/cmake)
  include_directories(${ARGOS_INCLUDE_DIRS})
  link_directories(${ARGOS_LIBRARY_DIRS})
endif()

# find eigen3
# find_package(Eigen3 REQUIRED)
# message(STATUS "Eigen include directory: ${EIGEN3_INCLUDE_DIR}")
# message(STATUS "Eigen version detected: ${EIGEN3_VERSION}")
# include_directories(${EIGEN3_INCLUDE_DIR})

# find pagmo2
find_package(Pagmo REQUIRED)
message(STATUS "Pagmo version detected")

##find boost
#set(_PAGMO_BOOST_MINIMUM_VERSION 1.60.0)
#find_package(Boost ${_PAGMO_BOOST_MINIMUM_VERSION} QUIET REQUIRED)
#set(_PAGMO_REQUIRED_BOOST_LIBS serialization)
#message(STATUS "Required Boost libraries: ${_PAGMO_REQUIRED_BOOST_LIBS}")
#find_package(Boost ${_PAGMO_BOOST_MINIMUM_VERSION} REQUIRED COMPONENTS "${_PAGMO_REQUIRED_BOOST_LIBS}")
#if(NOT Boost_FOUND)
#    message(FATAL_ERROR "Not all requested Boost components were found, exiting.")
#endif()
#message(STATUS "Detected Boost version: ${Boost_VERSION}")
#message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
#include_directories(${Boost_INCLUDE_DIR})
# Might need to recreate targets if they are missing (e.g., older CMake versions).
#if(NOT TARGET Boost::boost)
#    message(STATUS "The 'Boost::boost' target is missing, creating it.")
#    add_library(Boost::boost INTERFACE IMPORTED)
#    set_target_properties(Boost::boost PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
#endif()
#if(NOT TARGET Boost::disable_autolinking)
#    message(STATUS "The 'Boost::disable_autolinking' target is missing, creating it.")
#    add_library(Boost::disable_autolinking INTERFACE IMPORTED)
#    if(WIN32)
#        set_target_properties(Boost::disable_autolinking PROPERTIES INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB")
#    endif()
#endif()
#foreach(_PAGMO_BOOST_COMPONENT ${_PAGMO_REQUIRED_BOOST_LIBS})
#    if(NOT TARGET Boost::${_PAGMO_BOOST_COMPONENT})
#        message(STATUS "The 'Boost::${_PAGMO_BOOST_COMPONENT}' imported target is missing, creating it.")
#        string(TOUPPER ${_PAGMO_BOOST_COMPONENT} _PAGMO_BOOST_UPPER_COMPONENT)
#        if(Boost_USE_STATIC_LIBS)
#            add_library(Boost::${_PAGMO_BOOST_COMPONENT} STATIC IMPORTED)
#        else()
#            add_library(Boost::${_PAGMO_BOOST_COMPONENT} UNKNOWN IMPORTED)
#        endif()
#        set_target_properties(Boost::${_PAGMO_BOOST_COMPONENT} PROPERTIES
#            INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
#        set_target_properties(Boost::${_PAGMO_BOOST_COMPONENT} PROPERTIES
#            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
#            IMPORTED_LOCATION "${Boost_${_PAGMO_BOOST_UPPER_COMPONENT}_LIBRARY}")
#    endif()
#endforeach()




#
# Check for Lua 5.2
#
IF(NOT ARGOS_BUILD_FOR_RVR)
  find_package(Lua52)
  if(LUA52_FOUND)
    include_directories(${LUA_INCLUDE_DIR})
  endif(LUA52_FOUND)
endif (NOT ARGOS_BUILD_FOR_RVR)

#
# Check for Qt and OpenGL when compiling for the simulator
#
if(ARGOS_BUILD_FOR_SIMULATOR)
  include(ARGoSCheckQTOpenGL)
endif(ARGOS_BUILD_FOR_SIMULATOR)

#
# Check for Qt when compiling for the local e-puck tools
#
if(ARGOS_BUILD_FOR_LOCALRVR)
  find_package(Qt4 REQUIRED QtCore QtGui)
endif(ARGOS_BUILD_FOR_LOCALRVR)

#
# Check for PThreads
# It is required only when compiling the real robot
#
if(NOT ARGOS_BUILD_FOR_SIMULATOR)
  find_package(Pthreads)
  if(NOT PTHREADS_FOUND)
    message(FATAL_ERROR "Required library pthreads not found.")
  endif(NOT PTHREADS_FOUND)
  add_definitions(${PTHREADS_DEFINITIONS})
  include_directories(${PTHREADS_INCLUDE_DIR})
endif(NOT ARGOS_BUILD_FOR_SIMULATOR)
