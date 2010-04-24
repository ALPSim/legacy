#  Copyright Synge Todo 2010.
#  Distributed under the Boost Software License, Version 1.0.
#      (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

macro(add_alps_test)
  if(${ARGC} EQUAL 1)
    set(name ${ARGV0})
    set(input ${ARGV0})
    set(output ${ARGV0})
  else(${ARGC} EQUAL 1)
    if(${ARGC} EQUAL 2)
      set(name ${ARGV0})
      set(input ${ARGV1})
      set(output ${ARGV1})
    else(${ARGC} EQUAL 2)
      set(name ${ARGV0})
      set(input ${ARGV1})
      set(output ${ARGV2})
    endif(${ARGC} EQUAL 2)
  endif(${ARGC} EQUAL 1)
  enable_testing()
  if(MSVC)
    get_target_property(EXE_NAME ${name} LOCATION)
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${EXE_NAME} ${PROJECT_BINARY_DIR}/bin)
  endif(MSVC)
  
  if(EXISTS ${PROJECT_SOURCE_DIR}/config/run_test.cmake)
    add_test(${name}
      ${CMAKE_COMMAND}
        -Dcmd=${name}
        -Dsourcedir=${CMAKE_CURRENT_SOURCE_DIR}
        -Dbinarydir=${CMAKE_CURRENT_BINARY_DIR}
        -Ddllexedir=${PROJECT_BINARY_DIR}/bin
        -Dinput=${input}
        -Doutput=${output}
        -P ${PROJECT_SOURCE_DIR}/config/run_test.cmake
      )
  else(EXISTS ${PROJECT_SOURCE_DIR}/config/run_test.cmake)
    add_test(${name}
      ${CMAKE_COMMAND}
        -Dcmd=${name}
        -Dsourcedir=${CMAKE_CURRENT_SOURCE_DIR}
        -Dbinarydir=${CMAKE_CURRENT_BINARY_DIR}
        -Ddllexedir=${PROJECT_BINARY_DIR}/bin
        -Dinput=${input}
        -Doutput=${output}
        -P ${CMAKE_INSTALL_PREFIX}/share/alps/run_test.cmake
      )
  endif(EXISTS ${PROJECT_SOURCE_DIR}/config/run_test.cmake)
endmacro(add_alps_test)
