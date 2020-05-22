gen_server(tars tb)

add_dependencies(tb tb_comm)

if(APPLE)
    target_link_libraries(tb -Wl,-force_load tb_comm)
else()
    target_link_libraries(tb -Wl,--whole-archive tb_comm -Wl,--no-whole-archive)
endif()

target_link_libraries(tb tarsutil)
