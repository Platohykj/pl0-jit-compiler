# PL/0 JIT Compiler Makefile

# Compiler settings
CXX = clang++
CXXFLAGS = `llvm-config --cxxflags` -std=c++17 -O3 -fexceptions -Iinclude -Ivendor/cpp-peglib
LDFLAGS = `llvm-config --ldflags --system-libs --libs`

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
VENDOR_DIR = vendor/cpp-peglib

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cc)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)

# Target executable
TARGET = pl0

# Default target
.PHONY: all
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

# Benchmark target
.PHONY: bench
bench: $(TARGET)
	@echo '*** Python ***'
	@echo `python3 --version`
	@echo `time python3 samples/fib.py > /dev/null`
	@echo '*** Ruby ***'
	@echo `ruby --version`
	@echo `time ruby samples/fib.rb > /dev/null`
	@echo '*** PL/0 ***'
	@echo `time ./pl0 samples/fib.pas > /dev/null`

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Help target
.PHONY: help
help:
	@echo "PL/0 JIT Compiler Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all (default) - Build the pl0 compiler"
	@echo "  bench         - Run performance benchmarks"
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"
