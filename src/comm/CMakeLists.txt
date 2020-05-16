
aux_source_directory(. DIR_SRC)
aux_source_directory(./proto/ PROTO_SRC)
aux_source_directory(./licote/ LICOTE_SRC)

add_library(tb_comm ${DIR_SRC} ${PROTO_SRC} ${LICOTE_SRC})


