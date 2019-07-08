
SDK_SRC=sdk/*.c sdk/utility/ali_ws/*.c sdk/utility/json/*.c sdk/utility/log/*.c sdk/utility/threadpool/*.c sdk/utility/os/linux/os.c 
SDK_INCLUDE=-I sdk/utility -I sdk/utility/log -I sdk/utility/json -I sdk/utility/base-utils -I sdk/utility/threadpool -I sdk/utility/ali_ws -I sdk/export/include -I build/include -I sdk/utility/os
SDK_DEPEND_LIB=-l websockets -l ssl -l pthread -l crypto

SDK_DEPEND_LIB_PATH=-L build/lib/

OUTPUT_DIR=${PWD}/build
EXTRACT_DIR=${PWD}/.OO

export OUTPUT_DIR=${PWD}/build
export EXTRACT_DIR=${PWD}/.OO

.PHONY : leda clean
 
all: demo leda 

prepare:
	mkdir -p .OO
	make -C ./deps/zlib/
	make -C ./deps/openssl/
	make -C ./deps/libwebsockets/
  
leda :
	mkdir -p sdk/export/lib
	gcc $(SDK_SRC) $(SDK_INCLUDE) $(SDK_DENPEND_LIB) $(SDK_DEPEND_LIB_PATH) -fPIC -shared -o sdk/export/lib/libleda.so

demo : leda
	gcc demo/linux/demo.c -I sdk/export/include -l leda $(SDK_DEPEND_LIB) -L sdk/export/lib $(SDK_DEPEND_LIB_PATH) -o ./demo/linux/demo 

clean :
	rm -f ./sdk/*.o ./sdk/unit_test/linux/*.o ./demo/linux/*.o
	rm -rf ./demo/linux/demo
	rm -rf ./sdk/unit_test/linux/simulated_device
	rm -rf ./sdk/export/lib/libleda.so

distclean:
	rm -rf .OO build
