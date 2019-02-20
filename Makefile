LINUX_PREBUILT="sdk/os/linux/prebuilt/"

SDK_SRC=sdk/*.c sdk/ali_ws/*.c sdk/utility/json/*.c sdk/utility/log/*.c sdk/os/linux/os.c
SDK_INCLUDE=-I sdk/utility -I sdk/utility/log -I sdk/utility/json -I sdk/ali_ws -I sdk/include -I sdk/os/linux/prebuilt/libwebsockets/include -I sdk/os/linux/prebuilt/openssl/include -I sdk/os -I sdk/export/include
SDK_DEPEND_LIB=-l websockets -l ssl -l pthread -l crypto  
SDK_DEPEND_LIB_PATH=-L sdk/os/linux/prebuilt/libwebsockets/lib -L sdk/os/linux/prebuilt/openssl/lib

.PHONY : leda clean
 
all: demo leda
  
leda :
	gcc $(SDK_SRC) $(SDK_INCLUDE)  $(SDK_DENPEND_LIB) $(SDK_DEPEND_LIB_PATH) -fPIC -shared -o sdk/export/lib/libleda.so

demo : leda
	gcc demo/linux/demo.c -I sdk/export/include -l leda $(SDK_DEPEND_LIB) -L sdk/export/lib $(SDK_DEPEND_LIB_PATH) -o ./demo/linux/demo 

clean :
	find . -name *.o -exec rm -vf {} \;
	rm -rf ./demo/linux/demo
	rm -rf ./sdk/export/lib/libleda.so
