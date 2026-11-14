CMAKE ?= cmake
BUILD_DIR ?= build

.PHONY: all clean run test

all:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) --parallel

clean:
	rm -rf $(BUILD_DIR)

run:
	./$(BUILD_DIR)/server/host

test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure
