CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

IF(NOT Torch_FOUND)
  FIND_PACKAGE(Torch REQUIRED)
ENDIF()

IF(NOT TH_LIBRARIES)
  SET(TH_LIBRARIES "TH")
ENDIF(NOT TH_LIBRARIES)
MESSAGE(STATUS "TH_LIBRARIES: ${TH_LIBRARIES}")

IF(NOT THS_INSTALL_BIN_SUBDIR
    OR NOT THS_INSTALL_LIB_SUBDIR
    OR NOT THS_INSTALL_INCLUDE_SUBDIR
    OR NOT THS_INSTALL_CMAKE_SUBDIR)

  INCLUDE_DIRECTORIES(${TH_INCLUDE_PATH} ${TH_INCLUDE_PATH}/TH)
  LINK_DIRECTORIES(${TH_LIB_PATH})

  SET(THS_INSTALL_BIN_SUBDIR "bin" CACHE PATH "THS install binary subdirectory")
  SET(THS_INSTALL_LIB_SUBDIR "lib" CACHE PATH "THS install library subdirectory")
  SET(THS_INSTALL_INCLUDE_SUBDIR "include" CACHE PATH "THS install include subdirectory")
  SET(THS_INSTALL_CMAKE_SUBDIR "share/cmake/THS" CACHE PATH "THS install cmake subdirectory")
ELSE()
  SET(THS_INSTALL_BIN_SUBDIR ${Torch_INSTALL_BIN_SUBDIR})
  SET(THS_INSTALL_LIB_SUBDIR ${Torch_INSTALL_LIB_SUBDIR})
  SET(THS_INSTALL_INCLUDE_SUBDIR ${Torch_INSTALL_INCLUDE_SUBDIR})
  SET(THS_INSTALL_CMAKE_SUBDIR ${Torch_INSTALL_CMAKE_SUBDIR})
ENDIF()

# Flags
# When using MSVC
IF(MSVC)
  # we want to respect the standard, and we are bored of those **** .
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE=1)
ENDIF(MSVC)

IF (CMAKE_VERSION VERSION_LESS "3.1")
  SET(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
ELSE ()
  SET(CMAKE_C_STANDARD 99)
ENDIF ()

SET(hdr
  THS.h
  THSTensor.h
  )

SET(src
  THSTensor.c
  )

SET(src ${src} ${hdr})
ADD_LIBRARY(THS SHARED ${src})

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")
TARGET_LINK_LIBRARIES(THS ${TH_LIBRARIES})

IF(NOT THS_SO_VERSION)
  SET(THS_SO_VERSION 1)
ENDIF(NOT THS_SO_VERSION)
MESSAGE(STATUS "THS_SO_VERSION: ${THS_SO_VERSION}")
SET_TARGET_PROPERTIES(THS PROPERTIES
  VERSION   ${THS_SO_VERSION}
  SOVERSION ${THS_SO_VERSION})

INSTALL(TARGETS THS
  RUNTIME DESTINATION "${THS_INSTALL_BIN_SUBDIR}"
  LIBRARY DESTINATION "${THS_INSTALL_LIB_SUBDIR}"
  ARCHIVE DESTINATION "${THS_INSTALL_LIB_SUBDIR}")


INSTALL(FILES
  THS.h
  THSGenerateAllTypes.h
  THSGenerateFloatTypes.h
  THSGenerateComplexTypes.h
  THSGenerateIntTypes.h
  THSTensor.h
  DESTINATION "${THS_INSTALL_INCLUDE_SUBDIR}/THS")

INSTALL(FILES
  generic/THSTensor.c
  generic/THSTensor.h
  generic/THSTensorMath.c
  generic/THSTensorMath.h
  DESTINATION "${THS_INSTALL_INCLUDE_SUBDIR}/THS/generic")
