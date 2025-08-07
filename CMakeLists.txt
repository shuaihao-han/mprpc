# 设置cmake最低版本 和 项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc-hsh)

set(CMAKE_BUILD_TYPE "Debug") # 设置编译类型为Debug

# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
# 放 使用 rpc服务的 使用者 caller和 消费者callee
add_subdirectory(example)

