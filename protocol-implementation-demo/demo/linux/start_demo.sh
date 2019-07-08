#! /bin/bash

if [ $# != 2 ] && [ $# != 3 ]; then
	echo -e "usage: ./start_demo.sh [\$server_ip] [\$server_port] <\$use_tls>\n"\
			"    \$server_ip is the ip address of websocket driver\n"\
			"    \$server_port is the listen port of websocket driver\n"\
			"    \$use_tls is the tls switch, 0 close, 1 open, none is close for default"
    exit
fi

export LD_LIBRARY_PATH=../../build/lib/:../../sdk/export/lib/:$LD_LIBRARY_PATH

./demo $1 $2 $3
