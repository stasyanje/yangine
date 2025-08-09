CMAKE_DIR = cmake

## 	modules/**/*.cpp or .h
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path modules -Recurse -Include *.cpp, *.h | ForEach-Object { $$_.FullName }")
APP_NAME = app

format:	
	@clang-format -i $(SRC_FILES)

analyze:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(CMAKE_DIR)" rmdir /s /q "$(CMAKE_DIR)"

generate:
	@cmake --preset win

build: generate
	@cmake --build --preset win

.PHONY: format analyze clean build generate
