# set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(friend_provider ${SRC_LIST})
target_link_libraries(friend_provider mprpc protobuf pthread)