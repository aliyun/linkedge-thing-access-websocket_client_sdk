VERSION=3.1.0

all: install

pre_make:
	tar zxvf libwebsockets-$(VERSION).tar.gz -C $(EXTRACT_DIR)

make: pre_make
	cd $(EXTRACT_DIR)/libwebsockets-$(VERSION) \
	&& mkdir -p build \
    && cd build \
    && cmake .. \
        -DLWS_WITH_HTTP2=1 \
        -DLWS_OPENSSL_INCLUDE_DIRS=$(OUTPUT_DIR)/include/ \
        -DLWS_OPENSSL_LIBRARIES="$(OUTPUT_DIR)/lib/libssl.so;$(OUTPUT_DIR)/lib/libcrypto.so" \
        -DLWS_ZLIB_LIBRARIES=$(OUTPUT_DIR)/lib/libz.so \
        -DLWS_ZLIB_INCLUDE_DIRS=$(OUTPUT_DIR)/include/ \
        -Wno-error=unused-label \
    && make

install: make
	cp $(EXTRACT_DIR)/libwebsockets-$(VERSION)/build/lib/libwebsockets.a $(OUTPUT_DIR)/lib
	cp $(EXTRACT_DIR)/libwebsockets-$(VERSION)/build/lib/libwebsockets.so* $(OUTPUT_DIR)/lib
	cp -r $(EXTRACT_DIR)/libwebsockets-$(VERSION)/build/include/* $(OUTPUT_DIR)/include/

