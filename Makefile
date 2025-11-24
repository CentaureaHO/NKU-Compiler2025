INC_DIR := . interfaces utils
SRC_DIR := frontend middleend backend interfaces utils
OBJ_DIR := obj
BIN_DIR := bin

CXX ?= clang++
INCLUDES = $(addprefix -I, $(INC_DIR))
CXX_STANDARD = -std=c++17
DBGFLAGS = -g
WERROR_FLAGS := -Wall -Wextra -Wpedantic -Werror
WARN_IGNORE := -Wno-unused-parameter
CUSTOM_FLAGS := -DLOCAL_TEST
CXXFLAGS = -O2 -MMD -MP $(CXX_STANDARD) $(INCLUDES) $(WERROR_FLAGS) $(DBGFLAGS) $(WARN_IGNORE) $(CUSTOM_FLAGS)

-include toolchains.conf
RISCV_GCC ?= riscv64-unknown-elf-gcc
RISCV_AR  ?= riscv64-unknown-elf-ar

SOURCES = $(shell find $(SRC_DIR) -name "*.cpp" -type f)
MAIN_SRC = main.cpp

OBJECTS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
MAIN_OBJ = $(OBJ_DIR)/main.o
GEN_SRCS = $(BISON_C) $(LEXER_C)
GEN_OBJECTS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(GEN_SRCS))
ALL_OBJECTS = $(OBJECTS) $(GEN_OBJECTS) $(MAIN_OBJ)

TARGET = $(BIN_DIR)/compiler

PARSER_DIR = frontend/parser
LEXER_SRC  = $(PARSER_DIR)/lexer.l
BISON_SRC = $(PARSER_DIR)/yacc.y

LEXER_C    = $(LEXER_SRC:.l=.cpp)
BISON_H   = $(BISON_SRC:.y=.h)
BISON_C   = $(BISON_SRC:.y=.cpp)
LOC_H     = $(PARSER_DIR)/location.hh

LEXER_FILES = $(LEXER_C) $(BISON_H) $(BISON_C) $(LOC_H)

SOURCES := $(filter-out $(GEN_SRCS), $(SOURCES))

all: $(LEXER_FILES) $(TARGET)

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%/:
	@mkdir -p $@

$(TARGET): $(ALL_OBJECTS) | $(BIN_DIR)
	@echo "Linking object files -> $@"
	@$(CXX) $(ALL_OBJECTS) -o $@

$(OBJ_DIR)/main.o: main.cpp | $(OBJ_DIR)
	@echo "Compiling main.cpp -> $(OBJ_DIR)/main.o"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $< -> $@"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(GEN_OBJECTS): $(LEXER_FILES)

-include $(ALL_OBJECTS:.o=.d)

$(BISON_H) $(BISON_C) $(LOC_H): $(BISON_SRC)
	@bison -d --language=c++ --defines=$(BISON_H) -o $(BISON_C) $(BISON_SRC)
	@if [ -f $(BISON_H) ]; then clang-format -i $(BISON_H); fi
	@if [ -f $(BISON_C) ]; then clang-format -i $(BISON_C); fi
	@if [ -f $(LOC_H) ]; then clang-format -i $(LOC_H); fi

$(LEXER_C): $(LEXER_SRC) $(BISON_H)
	@flex --c++ --outfile=$(LEXER_C) $(LEXER_SRC)
	@if [ -f $(LEXER_C) ]; then clang-format -i $(LEXER_C); fi

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

clean-lexer:
	rm -f $(LEXER_FILES)

lexer: $(LEXER_FILES)

format:
	@find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.hh" \) -exec clang-format -i {} +

.PHONY: all clean clean-lexer lexer format libarm librv

libarm:
	@aarch64-linux-gnu-gcc lib/sylib.c -c -o libtmp.o -Ilib
	@aarch64-linux-gnu-ar rcs lib/libsysy_aarch.a libtmp.o
	@rm libtmp.o

librv:
	@$(RISCV_GCC) lib/sylib.c -c -o libtmp.o -Ilib -mcmodel=medany
	@$(RISCV_AR) rcs lib/libsysy_riscv.a libtmp.o
	@rm libtmp.o
