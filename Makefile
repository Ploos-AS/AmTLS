CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c99
CPPFLAGS ?= -Iinclude

BUILD := build
LIBOBJ := $(BUILD)/amtls.o

.PHONY: all check clean

all: $(BUILD)/TLSInfo

$(BUILD):
	mkdir -p $(BUILD)

$(LIBOBJ): src/amtls.c include/amtls/amtls.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/amtls.c -o $@

$(BUILD)/TLSInfo: cli/tlsinfo.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) cli/tlsinfo.c $(LIBOBJ) -o $@

$(BUILD)/test_api: tests/test_api.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_api.c $(LIBOBJ) -o $@

check: $(BUILD)/test_api $(BUILD)/TLSInfo
	./$(BUILD)/test_api
	./$(BUILD)/TLSInfo

clean:
	rm -rf $(BUILD)
