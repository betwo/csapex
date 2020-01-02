if(WIN32)
#add_definitions(-Wall)
else()
add_definitions(-W -Wall -Wextra -Wno-unused-parameter -fno-strict-aliasing -Wno-unused-function -pedantic)
endif()


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CSAPEX_BOOT_PLUGIN_DIR $ENV{HOME}/.csapex/boot)

## Enforce that we use C++17
if(CMAKE_VERSION VERSION_LESS "3.8.2")
  message(FATAL_ERROR "CMake Version < 3.8.2 is no longer supported.")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if (UNIX AND NOT APPLE)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
    if ("${ld_version}" MATCHES "GNU gold")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    endif()
endif()


#
# COVERAGE
#
if(ENABLE_COVERAGE)
    message("generating coverage information")

    SET(GCC_COVERAGE_COMPILE_FLAGS "--coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
    SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")

    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
endif()


macro(csapex_package)

        find_package(catkin QUIET)
        if(${catkin_FOUND})
                catkin_package(${ARGN})

        else()
                set(options)
                set(oneValueArgs)
                set(multiValueArgs INCLUDE_DIRS LIBRARIES DEPENDS CATKIN_DEPENDS)
                cmake_parse_arguments(csapex_package "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

                find_package(YAML-CPP REQUIRED)
                find_package(console_bridge REQUIRED)
                find_package(class_loader REQUIRED)
                if(WIN32)
                        add_definitions(/D POCO_NO_AUTOMATIC_LIBS)
                        add_definitions(/D NOMINMAX)
                endif()

                find_package(Boost COMPONENTS program_options filesystem system regex serialization thread REQUIRED)
                find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

                set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
                set(CATKIN_PACKAGE_INCLUDE_DESTINATION ${INSTALL_DIR}/include)
                set(CATKIN_PACKAGE_LIB_DESTINATION ${INSTALL_DIR}/lib)
                set(CATKIN_GLOBAL_BIN_DESTINATION ${INSTALL_DIR}/bin)
                set(CATKIN_PACKAGE_SHARE_DESTINATION ${INSTALL_DIR}/share/${PROJECT_NAME})
                set(CSAPEX_MODULE_DESTINATION ${INSTALL_DIR}/CMake)

                set(catkin_INCLUDE_DIRS
                        ${Boost_INCLUDE_DIRS}
                        ${YAML_CPP_INCLUDE_DIR}
                        ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS}
                        )
                set(catkin_LIBRARIES
                        ${Boost_LIBRARIES}
                        ${YAML_CPP_LIBRARIES}
                        ${class_loader_LIBRARIES}
                        ${console_bridge_LIBRARIES}
                        ${Poco_LIBRARIES}
                        )
                set(csapex_plugin_${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/${csapex_package_INCLUDE_DIRS}" CACHE INTERNAL "build_include")
                set(csapex_plugin_${PROJECT_NAME}_LIBRARIES ${csapex_package_LIBRARIES} CACHE INTERNAL "build_libs")

                if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
                        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
                        set(CSAPEX_CMAKE_MODULE_PATHS ${CSAPEX_CMAKE_MODULE_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL  "csapex_cmake_dirs")
                endif()
        endif()
endmacro()
