EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo http://web.tars.com/api/upload_and_publish -Fsuse=@tb.tgz -Fapplication=tars -Fmodule_name=tb -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND curl http://web.tars.com/api/upload_and_publish?ticket= -Fsuse=@tb.tgz -Fapplication=tars -Fmodule_name=tb -Fcomment=developer-auto-upload)
EXECUTE_PROCESS(COMMAND /usr/local/bin/cmake -E echo 
---------------------------------------------------------------------------)
