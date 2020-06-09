EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo release all)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -P /data/home/linfengchen/github/TarsBenchmark/build/run-release-NodeServer.cmake)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -P /data/home/linfengchen/github/TarsBenchmark/build/run-release-AdminServer.cmake)
