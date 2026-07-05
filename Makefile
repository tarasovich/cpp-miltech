BUILD_DIR := build
DEFAULT_PRESET := debug
AARCH64_PRESET := aarch64-debug

.PHONY: configure build build-all format lint test quality clean

configure:
	cmake --preset $(DEFAULT_PRESET)

build: configure
	cmake --build --preset $(DEFAULT_PRESET)

build-all: build
	cmake --preset $(AARCH64_PRESET)
	cmake --build --preset $(AARCH64_PRESET)

format:
	find . \( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./$(BUILD_DIR)/*" \
		-not -path "*/include/lib/*" \
		-exec clang-format -i {} +

format-%:
	find $* \
		\( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./$(BUILD_DIR)/*" \
		-not -path "*/include/lib/*" \
		-exec clang-format -i {} +

lint:
	find . \( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./$(BUILD_DIR)/*" \
		-not -path "*/include/lib/*" \
		-exec clang-tidy {} -p $(BUILD_DIR)/$(DEFAULT_PRESET) \;

lint-%:
	find $* \
		\( -name "*.cpp" -o -name "*.hpp" \) \
		-not -path "./$(BUILD_DIR)/*" \
		-not -path "*/include/lib/*" \
		-exec clang-tidy {} -p $(BUILD_DIR)/$(DEFAULT_PRESET) \;

test: build
	ctest --test-dir $(BUILD_DIR)/$(DEFAULT_PRESET) --output-on-failure

quality: format lint test

install-hooks:
	mkdir -p .git/hooks
	cp scripts/git-hooks/pre-commit .git/hooks/pre-commit
	chmod +x .git/hooks/pre-commit

clean:
	rm -rf $(BUILD_DIR)
