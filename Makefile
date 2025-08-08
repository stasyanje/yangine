INCLUDEDIR = include/engine
SRCDIR = src/engine
CMAKE_DIR = cmake
BUILDDIR = build

SRC_DIR = SRCDIR
INCLUDE_DIR = INCLUDEDIR

## 	src/**/*.cpp or .h
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp, *.h | ForEach-Object { $$_.FullName }")
APP_NAME = app

format:	
	@clang-format -i $(SRC_FILES)

analyze:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(CMAKE_DIR)" rmdir /s /q "$(CMAKE_DIR)"

generate:
	@if not exist "$(CMAKE_DIR)" mkdir $(CMAKE_DIR)
	@cd $(CMAKE_DIR) && cmake -G "Visual Studio 17 2022" -A x64 ..

build: generate
	@cd $(CMAKE_DIR) && msbuild $(APP_NAME).sln -p:Configuration=Debug -p:Platform=x64

run: build
	@$(CMAKE_DIR)\Debug\$(APP_NAME).exe

all: format clean run

.PHONY: format analyze clean build generate run all