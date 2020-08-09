cmake_minimum_required(VERSION 3.10.2)
project(PCL_COMPILER VERSION 1.0)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)

llvm_map_components_to_libnames(LLVM_LIBS core)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)


add_custom_target(LEXER ALL
	COMMAND flex -s -o ${PCL_COMPILER_SOURCE_DIR}/pcl_lexer.cpp ${PCL_COMPILER_SOURCE_DIR}/pcl_lexer.l)

add_custom_target(PARSER ALL
COMMAND bison -d -o ${PCL_COMPILER_SOURCE_DIR}/parser.cpp ${PCL_COMPILER_SOURCE_DIR}/parser.y)

file(GLOB_RECURSE SOURCE_FILES
	${PCL_COMPILER_SOURCE_DIR}/*.cpp)

add_library(pcl_lib STATIC ${PCL_COMPILER_SOURCE_DIR}/lib.cpp)

target_compile_options(pcl_lib
	PRIVATE
		-Wno-all
		-Wno-extra
		-Wno-everything
)
add_executable(PCL ${SOURCE_FILES})
add_dependencies(PCL LEXER PARSER)

target_include_directories(PCL PRIVATE ${PCL_COMPILER_SOURCE_DIR} ${LLVM_INCLUDE_DIRS})

# Link against LLVM libraries
target_link_libraries(PCL ${llvm_libs})

add_custom_command(
	TARGET PCL
	DEPENDS compile_commands.json
	WORKING_DIRECTORY ${PCL_COMPILER_SOURCE_DIR}
)
