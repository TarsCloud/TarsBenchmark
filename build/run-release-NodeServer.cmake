EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E make_directory /home/tarsproto/bm/NodeServer)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo cp -rf /data/home/linfengchen/github/TarsBenchmark/src/server/NodeServer/Node.h /home/tarsproto/bm/NodeServer)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E copy /data/home/linfengchen/github/TarsBenchmark/src/server/NodeServer/Node.h /home/tarsproto/bm/NodeServer)
