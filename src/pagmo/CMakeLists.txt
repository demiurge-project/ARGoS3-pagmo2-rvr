
set(pagmo_header
   mylogger.hpp 
   algorithms/cmaes.hpp
   algorithms/xnes.hpp
   problems/mpi_argos.hpp
    )

set(pagmo_sources
   mylogger.cpp 
   algorithms/cmaes.cpp
   algorithms/xnes.cpp
    )

# Add the shared library mypagmo
# shared libraries are linked dynamically and loaded at runtime

find_package(Eigen3 REQUIRED)
message(STATUS "Eigen include directory: ${EIGEN3_INCLUDE_DIR}")
message(STATUS "Eigen found")

add_library(mypagmo SHARED ${pagmo_header} ${pagmo_sources})
target_link_libraries(mypagmo PUBLIC Eigen3::Eigen)

set_property(TARGET mypagmo PROPERTY CXX_STANDARD 17)
set_property(TARGET mypagmo PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET mypagmo PROPERTY CXX_EXTENSIONS NO)
