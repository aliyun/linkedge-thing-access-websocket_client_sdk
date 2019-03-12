if [ $# != 2 ] && [ $# != 3 ]; then
	echo -e "usage: ./start_demo.sh [\$csv_path] [\$ip] <\$tls_switch>\n"\
			"    \$csv_path is the path of csv file downloaded from IoT platform\n"\
			"    \$ip is the ip address of Link IoT Edge\n"\
			"    \$tls_switch is the tls switch, 0 close, 1 open, none is close for default"
    exit
fi
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../sdk/os/linux/prebuilt/libwebsockets/lib/:../../sdk/os/linux/prebuilt/openssl/lib/:../../sdk/export/lib/

./demo $1 $2 $3
