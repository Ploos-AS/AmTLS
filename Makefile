CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c99
CPPFLAGS ?= -Iinclude -Isrc

BUILD := build
LIBOBJ := $(BUILD)/amtls.o $(BUILD)/allocator.o

.PHONY: all check smoke-68000 clean

all: $(BUILD)/TLSInfo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/amtls.o: src/amtls.c include/amtls/amtls.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/amtls.c -o $@

$(BUILD)/allocator.o: src/platform/allocator.c src/platform/allocator.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/platform/allocator.c -o $@

$(BUILD)/TLSInfo: cli/tlsinfo.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) cli/tlsinfo.c $(LIBOBJ) -o $@

$(BUILD)/test_api: tests/test_api.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_api.c $(LIBOBJ) -o $@

$(BUILD)/test_m1: tests/test_m1.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_m1.c $(LIBOBJ) -o $@

check: $(BUILD)/test_api $(BUILD)/test_m1 $(BUILD)/TLSInfo
	./$(BUILD)/test_api
	./$(BUILD)/test_m1
	./$(BUILD)/TLSInfo

smoke-68000:
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/amtls.c -o $(BUILD)/amtls-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c src/platform/allocator.c -o $(BUILD)/allocator-68000.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -m68000 -c cli/tlsinfo.c -o $(BUILD)/tlsinfo-68000.o

clean:
	rm -rf $(BUILD)
