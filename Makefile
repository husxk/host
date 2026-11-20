CMAKE ?= cmake
BUILD_DIR ?= build
COVERAGE_DIR ?= build-coverage

.PHONY: all clean run test coverage

all:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) --parallel

clean:
	rm -rf $(BUILD_DIR) $(COVERAGE_DIR)

run:
	./$(BUILD_DIR)/server/host

test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure

coverage:
	$(CMAKE) -S . -B $(COVERAGE_DIR) -DHOST_COVERAGE=ON
	$(CMAKE) --build $(COVERAGE_DIR) --parallel
	find $(COVERAGE_DIR) -name '*.gcda' -delete
	ctest --test-dir $(COVERAGE_DIR) --output-on-failure
	mkdir -p $(COVERAGE_DIR)/coverage
	gcovr -r . --filter server/src/ --filter server/include/ --filter common/ \
		--txt --html-details $(COVERAGE_DIR)/coverage/index.html $(COVERAGE_DIR)
