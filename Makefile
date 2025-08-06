ARFLAGS = /OUT:

BUILDDIR = build

lint:
	clang-format -i main.cpp src/*.cpp src/*.h include/*.h

clean:
	@if exist "$(BUILDDIR)" rmdir /s /q "$(BUILDDIR)"

build:
	@cmd /C "scripts\build.bat"

cmake_generate:
	@if not exist "build_cmake" mkdir build_cmake
	@cd build_cmake && cmake -G "Visual Studio 17 2022" -A x64 ..

cmake_build: cmake_generate
	@cd build_cmake && msbuild yangine.sln -p:Configuration=Debug -p:Platform=x64

run: cmake_build
	@build_cmake\Debug\yangine_app.exe

.PHONY: lint clean build cmake_generate cmake_build run all

all: build