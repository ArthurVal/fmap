# .rst:
# ----- common.cmake -----
# This module provide utils/common generic functionallities for cmake.
#
# ~~~
# Variables:
#   * n/a
# Functions:
#   * common_disable_in_src_builds()
#   * common_warn_build_type([DEFAULT])
#   * common_option(<NAME> [CHOICES] [DESCRIPTION] [DEFAULT])
# Macros:
#   * n/a
# ~~~

# ~~~
# Disable in-source builds.
#
# By default, in-source builds are disabled in order to not add compilation
# artifacts into the source directory which is most likely tracked by the CVS.
#
# In case you are FORCED to do in-src builds (for a distro, docker, ... ? IDK
# ...), set IN_SRC_BUILDS variable to ON in order to bypass the in-src build
# check.
#
# Arguments:
#   * N/A
# ~~~
function(common_disable_in_src_builds)
  option(IN_SRC_BUILDS
    "Enable/Disable In-source builds (Disabled by default)"
    OFF
  )

  if(NOT IN_SRC_BUILDS)
    get_filename_component(_SRC_DIR "${CMAKE_SOURCE_DIR}" REALPATH)
    get_filename_component(_BIN_DIR "${CMAKE_BINARY_DIR}" REALPATH)
    if("${_SRC_DIR}" STREQUAL "${_BIN_DIR}")
      message(FATAL_ERROR
        " In-source builds are not allowed.\n"
        " Please make an independant build directory using:\n"
        " cmake -S <SOURCE_DIR> -B <BUILD_DIR>\n"
        " \n"
        " Feel free to:\n"
        " rm -rf ${_SRC_DIR}/CMakeFiles ${_SRC_DIR}/CMakeCache.txt\n"
        " \n"
        " \n"
        " If in-source builds are mandatory, use '-DIN_SRC_BUILDS=ON' option"
      )
    endif()
  endif()
endfunction()

# ~~~
# Warn when CMAKE_BUILD_TYPE has not been set. Optionally set it to DEFAULT when
# not defined.
#
# Arguments:
#   * DEFAULT (optional - in):
#       Optional default CMAKE_BUILD_TYPE value set when CMAKE_BUILD_TYPE is
#       not defined by the user.
# ~~~
function(common_warn_build_type)
  cmake_parse_arguments(_args
    ""        # <- Flags
    "DEFAULT" # <- One value
    ""        # <- Multi values
    ${ARGN}
  )

  if(
      NOT DEFINED CMAKE_BUILD_TYPE
      OR "${CMAKE_BUILD_TYPE}" STREQUAL ""
    )
    message(WARNING
      " CMAKE_BUILD_TYPE has not been defined. It should be set to one of: \n"
      " - Debug:          ${CMAKE_C_FLAGS_DEBUG}\n"
      " - Release:        ${CMAKE_C_FLAGS_RELEASE}\n"
      " - RelWithDebInfo: ${CMAKE_C_FLAGS_RELWITHDEBINFO})\n"
      " - MinSizeRel:     ${CMAKE_C_FLAGS_MINSIZEREL}\n"
      " - ... (Other based on the generator used)\n"
      " ==> Default to \"${_args_DEFAULT}\"\n"
    )

    if(_args_DEFAULT)
      set(CMAKE_BUILD_TYPE ${_args_DEFAULT}
        CACHE STRING
        "Build optimizations. One of Debug, Release, RelWithDebInfo, MinSizeRel..."
        FORCE
      )
    endif()
  endif()
endfunction()

# ~~~
# Create a new option NAME with the possibility to constrain its value to a
# subset of CHOICES values (not necessary binary ON/OFF).
#
# Arguments:
#   * NAME (in):
#       Option variable name.
#   * CHOICES [VALUE_1, VALUE_2, ...] (optional - in):
#       The list of expected value for this option.
#       If undefined, default to binary ON/OFF list (same as option()).
#   * DEFAULT <VALUE> (optional - in):
#       Optional default value for the option (will be automatically appended to
#       the CHOICES value list if not in it).
#       If undefined, it will be set to the first element of CHOICES.
#   * DESCRIPTION <DESCR>  (optional - in):
#       Optional description string used for this option. The description
#       will be automatically appended with the list of expected values.
#   * ...:
#       Any unparsed arguments will be appended to CHOICES
# ~~~
function(common_option NAME)
  cmake_parse_arguments(arg
    ""                    # <- Flags
    "DESCRIPTION;DEFAULT" # <- One value
    "CHOICES"             # <- Multi values
    ${ARGN}
  )

  if(_args_UNPARSED_ARGUMENTS)
    list(APPEND _args_CHOICES ${_args_UNPARSED_ARGUMENTS})
  endif()

  if(NOT _args_CHOICES)
    # Default to BINARY CHOICES, same as cmake option()
    set(_args_CHOICES OFF ON)
  endif()

  if(NOT _args_DEFAULT)
    # Default to the first element of CHOICES
    list(GET _args_CHOICES 0 _args_DEFAULT)
  elseif(NOT _args_DEFAULT IN_LIST _args_CHOICES)
    # Prepend default to CHOICES, if default is not already inside CHOICES
    list(PREPEND _args_CHOICES ${_args_DEFAULT})
  endif()

  # Giving only one choice is not very usefull...
  list(LENGTH _args_CHOICES _size)
  if(_size LESS_EQUAL 1)
    message(FATAL_ERROR
      " Did not provided enough CHOICES values for option '${NAME}'.\n"
      " Got ${_size} values ([${_args_CHOICES_STR}]), expecting at least 2.\n"
    )
  endif()

  # _args_CHOICES_STR is a 'prettier' string used for messages
  list(JOIN _args_CHOICES ", " _args_CHOICES_STR)

  # Update the description with the choices...
  if(_args_DESCRIPTION)
    string(APPEND _args_DESCRIPTION " (Must be one of: [${_args_CHOICES_STR}])")
  else()
    set(_args_DESCRIPTION "One of: [${_args_CHOICES_STR}]")
  endif()

  # ... and the default value if provided
  string(APPEND _args_DESCRIPTION " (Default: ${_args_DEFAULT})")

  set(${NAME} ${_args_DEFAULT}
    CACHE STRING
    ${_args_DESCRIPTION}
  )

  # This is for cmake-gui users
  set_property(CACHE ${NAME}
    PROPERTY STRINGS
    ${_args_CHOICES}
  )

  if(NOT ${NAME} IN_LIST _args_CHOICES)
    message(FATAL_ERROR
      " Unknown value for ${NAME} (= ${${NAME}}).\n"
      " Expecting to be one of [${_args_CHOICES_STR}].\n"
      " Set it by either:\n"
      " - Configure cmake using '-D${NAME}=<VALUE>'\n"
      " - Update ${NAME} inside CMakeCache.txt manually\n"
    )
  endif()
endfunction()
