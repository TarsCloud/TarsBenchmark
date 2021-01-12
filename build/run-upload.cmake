EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo upload all)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -P /data/home/linfengchen/github/TarsBenchmark/build/run-upload-tb.cmake)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -P /data/home/linfengchen/github/TarsBenchmark/build/run-upload-NodeServer.cmake)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -P /data/home/linfengchen/github/TarsBenchmark/build/run-upload-AdminServer.cmake)
