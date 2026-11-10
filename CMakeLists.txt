cmake_minimum_required(VERSION 3.11)

project(DYHighMass)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ROOT REQUIRED)
list(APPEND ROOT_LIBRARIES ROOT::Unfold)
find_package(yaml-cpp REQUIRED)
find_package(correctionlib REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(ZLIB)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB sources CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/src/*.cc
)

file(GLOB headers CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/include/*.h
)

include(${ROOT_USE_FILE})

add_library(DYHighMassLIB SHARED ${sources})

target_include_directories(DYHighMassLIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS} ${YAML_CPP_INCLUDE_DIR} ${CORRECTINOLIB_INCLUDE_DIR})

target_link_libraries(
  DYHighMassLIB
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
)

target_include_directories(DYHighMassLIB PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

set_target_properties(DYHighMassLIB PROPERTIES PUBLIC_HEADER "${headers}")

install(TARGETS DYHighMassLIB EXPORT DYHighMassTarget
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT dev
)

add_executable(analysis_MUMU main/analysis_MUMU.cc ${sources} ${headers})

target_link_libraries(
  analysis_MUMU
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
  ${CMAKE_DL_LIBS}
)

add_executable(analysis_EMU main/analysis_EMU.cc ${sources} ${headers})

target_link_libraries(
  analysis_EMU
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
  ${CMAKE_DL_LIBS}
)

add_executable(analysis_EE main/analysis_EE.cc ${sources} ${headers})

target_link_libraries(
  analysis_EE
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
  ${CMAKE_DL_LIBS}
)

add_executable(generate_script main/generate_script.cc ${sources} ${headers})

target_link_libraries(
  generate_script
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
  ${CMAKE_DL_LIBS}
)

add_executable(job_summary main/job_summary.cc ${sources} ${headers})

target_link_libraries(
  job_summary
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
  ${CMAKE_DL_LIBS}
)
option(DEBUG_MODE "Enable debug output" OFF)
if(DEBUG_MODE)
  target_compile_definitions(DYHighMass PRIVATE DEBUGMODE=1)
endif()

install(FILES
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(TARGETS analysis_MUMU analysis_EMU analysis_EE generate_script job_summary DESTINATION bin)
