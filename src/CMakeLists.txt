aux_source_directory(. SRC_LIST)

add_library(mprpc SHARED ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)

# 显式设置输出路径
set_target_properties(mprpc PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
)