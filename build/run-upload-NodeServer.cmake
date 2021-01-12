EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo http://web.tars.com/api/upload_and_publish -Fsuse=@NodeServer.tgz -Fapplication=bm -Fmodule_name=NodeServer -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND curl http://web.tars.com/api/upload_and_publish?ticket= -Fsuse=@NodeServer.tgz -Fapplication=bm -Fmodule_name=NodeServer -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo 
---------------------------------------------------------------------------)
