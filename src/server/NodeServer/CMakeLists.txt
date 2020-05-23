gen_server(bm NodeServer)

add_dependencies(NodeServer tb_comm)

if(APPLE)
    target_link_libraries(NodeServer -Wl,-force_load tb_comm)
else()
    target_link_libraries(NodeServer -Wl,--whole-archive tb_comm -Wl,--no-whole-archive)
endif()

target_link_libraries(NodeServer tarsutil)
