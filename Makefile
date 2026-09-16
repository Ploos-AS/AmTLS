CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c99
CPPFLAGS ?= -Iinclude -Isrc

BUILD := build
LIBOBJ := $(BUILD)/amtls.o $(BUILD)/allocator.o
PUMPOBJ := $(BUILD)/handshake_pump.o
BSDSOCKETOBJ := $(BUILD)/bsdsocket_transport.o
LIBLIFECYCLEOBJ := $(BUILD)/library_lifecycle.o

.PHONY: all check smoke-68000 verify-backend-lock clean

all: $(BUILD)/TLSInfo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/amtls.o: src/amtls.c include/amtls/amtls.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/amtls.c -o $@

$(BUILD)/allocator.o: src/platform/allocator.c src/platform/allocator.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/platform/allocator.c -o $@

$(BUILD)/handshake_pump.o: src/backends/handshake_pump.c src/backends/handshake_pump.h src/backends/backend.h src/platform/transport.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/backends/handshake_pump.c -o $@

$(BUILD)/bsdsocket_transport.o: src/platform/bsdsocket_transport.c src/platform/bsdsocket_transport.h src/platform/transport.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/platform/bsdsocket_transport.c -o $@

$(BUILD)/library_lifecycle.o: src/amiga/library_lifecycle.c include/amtls/library.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/amiga/library_lifecycle.c -o $@

$(BUILD)/TLSInfo: cli/tlsinfo.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) cli/tlsinfo.c $(LIBOBJ) -o $@

$(BUILD)/test_api: tests/test_api.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_api.c $(LIBOBJ) -o $@

$(BUILD)/test_m1: tests/test_m1.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_m1.c $(LIBOBJ) -o $@

$(BUILD)/test_m2_1_pump: tests/test_m2_1_pump.c $(PUMPOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_m2_1_pump.c $(PUMPOBJ) -o $@

$(BUILD)/test_m3_1_bsdsocket: tests/test_m3_1_bsdsocket.c $(BSDSOCKETOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_m3_1_bsdsocket.c $(BSDSOCKETOBJ) -o $@

$(BUILD)/test_m3_2_library: tests/test_m3_2_library.c $(LIBLIFECYCLEOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_m3_2_library.c $(LIBLIFECYCLEOBJ) -o $@

check: verify-backend-lock $(BUILD)/test_api $(BUILD)/test_m1 $(BUILD)/test_m2_1_pump $(BUILD)/test_m3_1_bsdsocket $(BUILD)/test_m3_2_library $(BUILD)/TLSInfo
	./$(BUILD)/test_api
	./$(BUILD)/test_m1
	./$(BUILD)/test_m2_1_pump
	./$(BUILD)/test_m3_1_bsdsocket
	./$(BUILD)/test_m3_2_library
	./$(BUILD)/TLSInfo

verify-backend-lock:
	python3 tools/verify_backend_lock.py

smoke-68000: | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/amtls.c -o $(BUILD)/amtls-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/platform/allocator.c -o $(BUILD)/allocator-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/backends/handshake_pump.c -o $(BUILD)/handshake_pump-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/platform/bsdsocket_transport.c -o $(BUILD)/bsdsocket_transport-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/amiga/library_lifecycle.c -o $(BUILD)/library_lifecycle-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c cli/tlsinfo.c -o $(BUILD)/tlsinfo-68000.o

clean:
	rm -rf $(BUILD)
