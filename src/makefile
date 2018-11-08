# Variables
COMPILER = gcc
C_FLAGS = -m32
BINARY_NAME = ext2_file_system
RM = rm -f
RM_R = rm -rf
STUPID_FILES = *.gch

# 'make build'
build:
	$(COMPILER) $(C_FLAGS) -o $(BINARY_NAME) main.c util/util.c util/type.h init.c

# 'make run'
run: build
	./$(BINARY_NAME)

# 'make clean'
clean: build
	$(RM) $(BINARY_NAME)
	$(RM_R) $(STUPID_FILES)
