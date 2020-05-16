gen_server(tars tb)

add_dependencies(tb tb_comm)

target_link_libraries(tb tb_comm)