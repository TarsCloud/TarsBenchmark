#!/bin/bash

TARS_WEB_HOST=$1
TARS_WEB_TOKEN=$2
NODE_SERVER_IP=$3
ADMIN_SERVER_IP=$4
WORKDIR=$(cd $(dirname $0); pwd)

if [ $# -eq 4 ]; then
    TARS_CPP_PATH="/usr/local/tars/cpp"
elif [ $# -eq 5 ]; then
    TARS_CPP_PATH=$5
else
    echo "Usage:";
    echo "  $0 webhost tarstoken adminsip nodeip";
    echo "  $0 webhost tarstoken adminsip nodeip tarscpp";
    echo "Description:";
    echo "  webhost: tars web admin host";
    echo "  tarstoken: can fetch from http://webhost:3001/auth.html#/token";
    echo "  adminsip: ip address of adminserver will be installed";
    echo "  nodeip: ip address of nodeserver will be installed";
    echo "  tarscpp: the path which tarscpp has installed";
    exit 1
fi

### check tarscpp environment
if [ ! -d ${TARS_CPP_PATH} ]; then
    echo "tarscpp not exits, please install fisrt"
    echo "Goto https://tarscloud.github.io/TarsDocs_en/env/tarscpp.html"
    exit 1
fi


### check os environment
OSNAME=`uname`
if [[ "$OSNAME" == "Windows_NT" ]]; then
    echo "benchmark don't support windows";
    exit 1
fi

function LOG_INFO()
{
	local msg=$(date +%Y-%m-%d" "%H:%M:%S);

	for p in $@
	do
		msg=${msg}" "${p};
	done

	echo -e "\033[32m $msg \033[0m"
}

#输出配置信息
LOG_INFO "===>install benchmark server >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
LOG_INFO "PARAMS:        "$*
LOG_INFO "OS:            "$OSNAME
LOG_INFO "TarsCpp:       "$TARS_CPP_PATH
LOG_INFO "WEBHOST:       "$TARS_WEB_HOST
LOG_INFO "TOKEN:         "$TARS_WEB_TOKEN
LOG_INFO "===<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< print envirenment finish.\n";

################################################################################
function build_server()
{
    rm -rf $WORKDIR/build
    mkdir $WORKDIR/build && cd $WORKDIR/build && cmake .. && make all && make tar
}

function build_webconf()
{
    cd $WORKDIR

    LOG_INFO "===>install benchmark template:\n";
    curl -s -X POST -H "Content-Type: application/json" http://${TARS_WEB_HOST}/api/add_profile_template?ticket=${TARS_WEB_TOKEN} -d@assets/template.json|echo

    LOG_INFO "===>install nodeserver:\n";
    sed -i "s/host_ip/$NODE_SERVER_IP/g" assets/nodeserver.json
    curl -s -X POST -H "Content-Type: application/json" http://${TARS_WEB_HOST}/api/deploy_server?ticket=${TARS_WEB_TOKEN} -d@assets/nodeserver.json|echo

    LOG_INFO "===>install adminserver:\n";
    sed -i "s/host_ip/$ADMIN_SERVER_IP/g" assets/adminserver.json
    curl -s -X POST -H "Content-Type: application/json" http://${TARS_WEB_HOST}/api/deploy_server?ticket=${TARS_WEB_TOKEN} -d@assets/adminserver.json|echo
}

function upload_server()
{
    cd $WORKDIR/build

    LOG_INFO "===>upload adminserver:\n"
    curl -s http://${TARS_WEB_HOST}/api/upload_and_publish?ticket=${TARS_WEB_TOKEN} -Fsuse=@AdminServer.tgz -Fapplication=benchmark -Fmodule_name=AdminServer -Fcomment=auto-upload|echo

    LOG_INFO "===>upload nodeserver:\n"
    curl -s http://${TARS_WEB_HOST}/api/upload_and_publish?ticket=${TARS_WEB_TOKEN} -Fsuse=@NodeServer.tgz -Fapplication=benchmark -Fmodule_name=NodeServer -Fcomment=auto-upload
}

build_server
build_webconf
upload_server
