cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LLVM_DIR /opt/homebrew/opt/llvm)

find_package(Clang REQUIRED CONFIG HINTS "${LLVM_DIR}/lib/cmake/clang/")

add_executable(checker main.cpp)

target_include_directories(checker PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(checker PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangTooling
)
