CMAKE_DIR = out

## 	modules/**/*.cpp or .h excluding vendor directories
SRC_FILES = $(shell powershell -Command "Get-ChildItem -Path modules -Recurse -Include *.cpp, *.h | Where-Object { $$_.FullName -notmatch '\\vendor\\' } | ForEach-Object { $$_.FullName }")

format:	
	@clang-format -i $(SRC_FILES)

analyze:
	@clang-tidy $(SRC_FILES)

clean:
	@if exist "$(CMAKE_DIR)" rmdir /s /q "$(CMAKE_DIR)"

generate:
	@cmake -S . -B $(CMAKE_DIR) -G "Ninja" -DCMAKE_CXX_SCAN_FOR_MODULES=OFF

build: generate
	@cmake --build $(CMAKE_DIR)

.PHONY: format analyze clean build generate
