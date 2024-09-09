#sudo apt-get install -y libgpiod-dev
#ipcrm -m 
#ipcrm -m 10
.DEFAULT_GOAL := release 
# the compiler: gcc for C program or g++ for C++ program
CC = gcc
#File Extension by .c for C program or .cpp for C++ program
FE = c
# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings
#  -o 	  - output flag 

COMMON_FLAGS = -pthread  -lsqlite3 -lcurl -lcjson
DEBUG_FLAGS = -DDEBUG -g
RELEASE_FLAGS = -DRELEASE
# Directories
MAIN_DIR = ./build
OUT_DIR = $(MAIN_DIR)/out
BUILD_DIR = $(MAIN_DIR)/bin
PROGRAM_MAIN = main.$(FE)
# Search for source files and create a list of object files
NOT_INCLUDE_FILES := ! -name 'main.$(FE)' #! -name 'main.cpp 
NOT_INCLUDE_DIRS := -not -path "./build/*"

ALL_SOURCES := $(shell find . -name '*.$(FE)' $(NOT_INCLUDE_FILES) $(NOT_INCLUDE_DIRS))  # Exclude main.cpp and out/ directory.
ALL_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.$(FE),%.o,$(ALL_SOURCES)))) 

# Default target
all: clean dirCreation $(PROGRAM_MAIN)
# Creating directories
dirCreation:
	mkdir -p $(OUT_DIR)
	mkdir -p $(BUILD_DIR)

# Compiling the main program
$(PROGRAM_MAIN): $(ALL_OBJECTS) | print_end 
	$(CC) $(PROGRAM_MAIN) $(BUILD_FLAGS) $^ $(COMMON_FLAGS) -o $(OUT_DIR)/fingerprint

# Including source file directories
vpath %.$(FE) $(sort $(dir $(ALL_SOURCES)))
# Compiling source files into object files
$(BUILD_DIR)/%.o: %.$(FE)
	$(CC) -c $(BUILD_FLAGS) $< -o $@

# Cleaning up build directories
.PHONY: clean

print_end:
	@echo "Compiled Build objects successfully."

clean:
	@clear
	rm -rf $(MAIN_DIR)
	@echo "cleaned successfully."

#make git com="s"
git:
	git add .
	git commit -m "$(commit)"
	git push
	@echo "Compiled push successfully."

# Conditional build depending on mode
# Default build is in release mode
# To build in debug mode, call `make debug`
debug:
	$(MAKE) all BUILD_FLAGS="$(DEBUG_FLAGS)"
	@echo "Build complete. Mode: Debug"
release:
	$(MAKE) all BUILD_FLAGS="$(RELEASE_FLAGS)"
	@echo "Build complete. Mode: Release"