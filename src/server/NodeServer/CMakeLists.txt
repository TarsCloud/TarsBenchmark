gen_server(bm NodeServer)

add_dependencies(NodeServer tb_comm)

target_link_libraries(NodeServer tb_comm)
target_link_libraries(NodeServer tarsutil)
