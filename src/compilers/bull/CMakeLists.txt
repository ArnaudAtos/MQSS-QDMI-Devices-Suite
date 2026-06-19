# Copyright 2026
#
# Licensed under the Apache License, Version 2.0 with LLVM Exceptions (the
# "License"); you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://github.com/Munich-Quantum-Software-Stack/QDMI-Devices/blob/develop/LICENSE
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# ------------------------------------------------------------------------------

# Find the pybind11 cmake package
# This package should be installed using "pip install pybind11"
# find_package(Python3 3.10...3.12 REQUIRED COMPONENTS Interpreter Development)
find_package(Python3 3.14 REQUIRED COMPONENTS Interpreter Development)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
find_package(pybind11 REQUIRED)

add_library(qaptiva_compiler_device SHARED qaptiva_compiler.cpp)

target_link_libraries(qaptiva_compiler_device PUBLIC qdmi::qdmi qdmi::project_warnings pybind11::embed)

generate_prefixed_qdmi_headers(${QAPTIVA_COMPILER_QDMI_PREFIX})
target_include_directories(
  qaptiva_compiler_device PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include ${Python3_INCLUDE_DIRS}
)

add_library(qdmi::qaptiva_compiler_device ALIAS qaptiva_compiler_device)

if(ENABLE_COVERAGE)
  target_compile_options(qaptiva_compiler_device INTERFACE --coverage -fprogile-arcs -ftest-coverage -O0)
  target_link_libraries(tqaptiva_compiler_device INTERFACE gcov --coverage)
endif()
