cmake_minimum_required(VERSION 3.28)
project(moonlisp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME})

# Module File
target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES
  TYPE CXX_MODULES
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/module
  FILES
  "module/constant.ixx"
  "module/exception.ixx"
  "module/lexer.ixx"
  "module/ast.ixx"
  "module/parser.ixx"
  "module/value.ixx"
  "module/builtin.ixx"
  "module/compiler.ixx"
  "module/vm.ixx"
)

target_sources(${PROJECT_NAME}
  PRIVATE
  "source/main.cpp")

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    "/experimental:module"
    "/permissive-"
    "/utf-8"
  )

  set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_SCAN_FOR_MODULES true
    CXX_SCAN_FOR_MODULES ON
  )
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

  if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
  endif()

endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE
    "-fmodules"
    "-std=c++20"
  )
endif()