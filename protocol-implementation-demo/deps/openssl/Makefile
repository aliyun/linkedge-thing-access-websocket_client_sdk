LIB_TAR = openssl-1.0.2p

mk_path := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))

all: install

pre_make:
	tar zxf $(LIB_TAR).tar.gz -C $(EXTRACT_DIR)
	cp openssl.map $(EXTRACT_DIR)/$(LIB_TAR)
	cd $(EXTRACT_DIR)/$(LIB_TAR) \
		&& ./Configure linux-generic32 \
		shared \
		threads \
		--prefix=$(OUTPUT_DIR) \
		-Wl,--version-script=./openssl.map

make: pre_make
	cd $(EXTRACT_DIR)/$(LIB_TAR) && make -j8 build_libs build_apps

install: make
	cd $(EXTRACT_DIR)/$(LIB_TAR) && make install_sw 

clean:
	cd $(EXTRACT_DIR)/$(LIB_TAR) && make clean
	rm -rf $(EXTRACT_DIR)/$(LIB_TAR)
