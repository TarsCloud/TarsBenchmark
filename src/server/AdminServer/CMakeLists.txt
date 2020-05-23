gen_server(bm AdminServer)

include_directories(../NodeServer)
add_dependencies(AdminServer NodeServer tb_comm)

if(APPLE)
    target_link_libraries(AdminServer -Wl,-force_load tb_comm)
else()
    target_link_libraries(AdminServer -Wl,--whole-archive tb_comm -Wl,--no-whole-archive)
endif()

target_link_libraries(AdminServer tarsutil)
