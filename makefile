#sudo apt-get install -y libgpiod-dev
#ipcrm -m 
#ipcrm -m 10

# the compiler: gcc for C program or g++ for C++ program
CC = gcc
#File Extension by .c for C program or .cpp for C++ program
FE = c
# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings
#  -o 	  - output flag 

FLAGS = -pthread  -lsqlite3 -lcurl
DEBUG = -g
MAIN_DIR = ./build
OUT_DIR = $(MAIN_DIR)/out
BUILD_DIR = $(MAIN_DIR)/bin
PROGRAM_MAIN = main.$(FE)

NOT_INCLUDE_FILES := ! -name 'main.$(FE)' #! -name 'main.cpp ! -name 'test.cpp'
NOT_INCLUDE_DIRS := -not -path "./build/*"

ALL_SOURCES := $(shell find . -name '*.$(FE)' $(NOT_INCLUDE_FILES) $(NOT_INCLUDE_DIRS))  # Exclude main.cpp and out/ directory.
ALL_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.$(FE),%.o,$(ALL_SOURCES)))) 

all: clean dirCreation $(PROGRAM_MAIN)

dirCreation:
	mkdir -p $(OUT_DIR)
	mkdir -p $(BUILD_DIR)

$(PROGRAM_MAIN): $(ALL_OBJECTS) | print_end 
	$(CC) $(PROGRAM_MAIN) $(DEBUG) $^  $(FLAGS) -o $(OUT_DIR)/$(PROGRAM_MAIN:.$(FE)=.out) 
	@echo "Build complete."

vpath %.$(FE) $(sort $(dir $(ALL_SOURCES)))

$(BUILD_DIR)/%.o: %.$(FE)
	$(CC) -c $(FLAGS) $< -o $@

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
