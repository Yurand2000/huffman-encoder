COMPILER=g++
OPTIONS=-O3 -std=c++2a -pedantic -Werror
COMPILE=$(COMPILER) $(OPTIONS)
SRC=src
BUILD=build

include ./src/Makefile

#final path substitutions
SRC_FILES := $(patsubst %,$(SRC)/%,$(SRC_FILES))
OBJECTS = $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SRC_FILES))

all: $(BUILD)/program

$(BUILD)/program : $(OBJECTS)
	@mkdir build -p
	$(COMPILE) -o $@ $^

$(BUILD)/%.o : $(SRC)/%.cpp
	@mkdir $(dir $@) -p
	$(COMPILE) -c -o $@ $<

run: all
	@echo ''
	@$(BUILD)/program

test:
	@mkdir build -p
	@mkdir build/test -p

	g++ -O3 -std=c++2a -o build/encoder_table_tests.out src/encoder/encoder_table.cpp src/encoder/encoder_table_tests.cpp
	@echo ''
	@./build/encoder_table_tests.out

clean:
	@rm -rf $(BUILD)

.PHONY: all run test clean