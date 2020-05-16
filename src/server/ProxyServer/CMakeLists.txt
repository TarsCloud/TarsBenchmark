gen_server(bm ProxyServer)

include_directories(../NodeServer)
add_dependencies(ProxyServer NodeServer tb_comm)

target_link_libraries(ProxyServer tb_comm)
