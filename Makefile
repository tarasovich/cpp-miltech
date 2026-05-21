BUILD_DIR := build/debug

.PHONY: build format lint test quality clean

build:
	cmake --build --preset debug

format:
	find . \( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./build/*" \
		-exec clang-format -i {} +

lint:
	find . \( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./build/*" \
		-exec clang-tidy {} -p $(BUILD_DIR) \;

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

quality: format lint test

clean:
	rm -rf $(BUILD_DIR)
