# Variables
COMPILER = gcc
C_FLAGS = -m32
BINARY_NAME = ext2_file_system
RM = rm -f
RM_R = rm -rf
STUPID_FILES = *.gch

# 'make build'
build:
	$(COMPILER) -o $(BINARY_NAME) $(C_FLAGS) util/type.h util/*.c init.c cmd/level[1-9]/*.c cmd/*.c main.c

# 'make run'
run: build
	./$(BINARY_NAME)

# 'make clean'
clean: build
	$(RM) $(BINARY_NAME)
	$(RM_R) $(STUPID_FILES)
