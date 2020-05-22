gen_server(tars tb)

add_dependencies(tb tb_comm)

target_link_libraries(tb -Wl,--whole-archive tb_comm -Wl,--no-whole-archive)
target_link_libraries(tb tarsutil)


