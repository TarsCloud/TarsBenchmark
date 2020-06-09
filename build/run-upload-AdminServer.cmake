EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo http://web.tars.com/api/upload_and_publish -Fsuse=@AdminServer.tgz -Fapplication=bm -Fmodule_name=AdminServer -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND curl http://web.tars.com/api/upload_and_publish?ticket= -Fsuse=@AdminServer.tgz -Fapplication=bm -Fmodule_name=AdminServer -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo 
---------------------------------------------------------------------------)
