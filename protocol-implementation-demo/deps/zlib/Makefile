LIB_TAR = zlib-1.2.11

all: install

pre_make:
	tar zxvf $(LIB_TAR).tar.gz -C $(EXTRACT_DIR)

make: pre_make
	cd $(EXTRACT_DIR)/$(LIB_TAR) && ./configure --prefix=$(OUTPUT_DIR) && make CFLAGS=-std=c99

install: make
	cd $(EXTRACT_DIR)/$(LIB_TAR) && make install


