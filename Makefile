INCLUDEDIR = include/engine
SRCDIR = src/engine
CMAKE_DIR = cmake
BUILDDIR = build

SRC_DIR = SRCDIR
INCLUDE_DIR = INCLUDEDIR

## 	src/**/*.cpp or .h
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp, *.h | ForEach-Object { $$_.FullName }")

# Run clang-format on source code
.PHONY: format
format:	
	@clang-format -i $(SRC_FILES)

.PHONY: lint
lint:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(BUILDDIR)" rmdir /s /q "$(BUILDDIR)"

generate:
	@if not exist "$(CMAKE_DIR)" mkdir $(CMAKE_DIR)
	@cd $(CMAKE_DIR) && cmake -G "Visual Studio 17 2022" -A x64 ..

build: generate
	@cd $(CMAKE_DIR) && msbuild yangine.sln -p:Configuration=Debug -p:Platform=x64

run: build
	@$(CMAKE_DIR)\Debug\app.exe

.PHONY: lint clean build generate build run all