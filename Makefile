# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -g -Werror -O1

# Source files
SRCS = test_fs_fat.c fs_fat.c fs_file.c

# Header files
HEADERS = fs_fat.h fs_file.h

# Output executable
TARGET = test_fs_fat

# Build target
all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
		$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

# Clean target
clean:
		rm -f $(TARGET) FILE_SYSTEM