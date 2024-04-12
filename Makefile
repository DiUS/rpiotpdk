.DEFAULT: rpiotpdk

LDFLAGS=-lcrypto

.PHONY: rpiotpdk
rpiotpdk: build/rpiotpdk

build/rpiotpdk: src/*.c | src/*.h
	@mkdir -p build
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

.PHONY: clean
clean:
	-rm -rf build
