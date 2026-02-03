cmake_minimum_required(VERSION 3.12)

project(fmap
  VERSION 1.3.0
  DESCRIPTION "Provide fmap utils tools to read/write to file using mmap"
  LANGUAGES C
)

# PROJECT_PREFIX ##############################################################
# Uppercase prefix used by all CMake options / definitions / ... of this project
string(TOUPPER ${PROJECT_NAME} PROJECT_PREFIX)
string(REPLACE "-" "_" PROJECT_PREFIX ${PROJECT_PREFIX})

###############################################################################
#                               GLOBAL includes                               #
###############################################################################
# We use install paths CMAKE_INSTALL_* from standard GNU install dirs See:
# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# cmake_print_variables()
include(CMakePrintHelpers)

# Enables other projects to depend on us, using find_package()
# configure_package_config_file() and write_basic_package_version_file()
include(CMakePackageConfigHelpers)

# Common usefull cmake macros/functions...
include(cmake/common.cmake)
common_disable_in_src_builds()

###############################################################################
#                            GLOBAL USEFULL OPTIONS                           #
###############################################################################
# Dump PROJECT related infos
cmake_print_variables(PROJECT_NAME)
cmake_print_variables(PROJECT_PREFIX)
cmake_print_variables(PROJECT_DESCRIPTION)
cmake_print_variables(PROJECT_VERSION)

# C STANDARD ################################################################
set(CMAKE_C_STANDARD 11)
cmake_print_variables(CMAKE_C_STANDARD)

set(CMAKE_C_STANDARD_REQUIRED True)
cmake_print_variables(CMAKE_C_STANDARD_REQUIRED)

# CMAKE_INSTALL_PREFIX ########################################################
# Just print it for the user, do not enforce it since default value is GOOD
cmake_print_variables(CMAKE_INSTALL_PREFIX)

# BUILD_TYPE ##################################################################
common_warn_build_type(DEFAULT RELEASE)
cmake_print_variables(CMAKE_BUILD_TYPE)

###############################################################################
#                           PROJET SPECIFIC OPTIONS                           #
###############################################################################
# N/A

###############################################################################
#                                    BUILD                                    #
###############################################################################
add_subdirectory(src)
