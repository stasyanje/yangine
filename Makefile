CMAKE_DIR = cmake

## 	src/**/*.cpp or .h
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path src -Recurse -Include *.cpp, *.h | ForEach-Object { $$_.FullName }")
APP_NAME = app

VCVAR := "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat"

format:	
	@clang-format -i $(SRC_FILES)

analyze:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(CMAKE_DIR)" rmdir /s /q "$(CMAKE_DIR)"

generate:
	@cmd /c "call $(VCVARS)"
	@cmake --preset win"

build: generate
	@cmake --build --preset win

all: format clean build

.PHONY: format analyze clean build generate run all
