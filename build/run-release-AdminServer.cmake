EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E make_directory /home/tarsproto/bm/AdminServer)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo cp -rf /data/home/linfengchen/github/TarsBenchmark/src/server/AdminServer/Admin.h /home/tarsproto/bm/AdminServer)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E copy /data/home/linfengchen/github/TarsBenchmark/src/server/AdminServer/Admin.h /home/tarsproto/bm/AdminServer)
