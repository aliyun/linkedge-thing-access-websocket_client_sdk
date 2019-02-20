if [ $# != 1 ] && [ $# != 2 ]; then
    echo "usage: ./start_demo.sh [ip] <tls>"
    exit
fi
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../sdk/os/linux/prebuilt/libwebsockets/lib/:../../sdk/os/linux/prebuilt/openssl/lib/:../../sdk/export/lib/

./demo $1 $2 $3
