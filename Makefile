ARFLAGS = /OUT:

BUILDDIR = build

lint:
	clang-format -i src/*.cpp include/*.h

clean:
	@if exist "$(BUILDDIR)" rmdir /s /q "$(BUILDDIR)"

build:
	@cmd /C "scripts\build.bat"

.PHONY: lint clean build all

all: build