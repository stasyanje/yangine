ARFLAGS = /OUT:

INCLUDEDIR = include/engine
SRCDIR = src/engine
BUILDDIR = build

lint:
	clang-format -i main.cpp $(SRCDIR)/*.cpp $(SRCDIR)/*.h $(INCLUDEDIR)/*.h

clean:
	@if exist "$(BUILDDIR)" rmdir /s /q "$(BUILDDIR)"

generate:
	@if not exist "$(BUILDDIR)" mkdir $(BUILDDIR)
	@cd $(BUILDDIR) && cmake -G "Visual Studio 17 2022" -A x64 ..

build: generate
	@cd $(BUILDDIR) && msbuild yangine.sln -p:Configuration=Debug -p:Platform=x64

run: cmake_build
	@$(BUILDDIR)\Debug\yangine_app.exe

.PHONY: lint clean build cmake_generate cmake_build run all

all: build