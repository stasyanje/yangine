CMAKE_DIR = cmake

## 	modules/**/*.cpp or .h
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path modules -Recurse -Include *.cpp, *.h | ForEach-Object { $$_.FullName }")

format:	
	@clang-format -i $(SRC_FILES)

analyze:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(CMAKE_DIR)" rmdir /s /q "$(CMAKE_DIR)"

generate:
	@cmake -S . -B $(CMAKE_DIR) -G "Ninja"

build: generate
	@cmake --build $(CMAKE_DIR)

.PHONY: format analyze clean build generate
