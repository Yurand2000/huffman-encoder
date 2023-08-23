COMPILER=g++
OPTIONS=-O3 -std=c++2a -pedantic -Werror -pthread -I$(realpath .)/include/
COMPILE=$(COMPILER) $(OPTIONS)
SRC=src
BUILD=build
BUILD_TESTS=$(BUILD)/tests

include ./src/Makefile

#final path substitutions
SRC_FILES := $(patsubst %,$(SRC)/%,$(SRC_FILES))
OBJECTS = $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SRC_FILES))
DEPENDS := $(patsubst %.o,%.d,$(OBJECTS))

TEST_FILES := $(patsubst %,$(SRC)/%,$(TEST_FILES))
TEST_OBJECTS = $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(TEST_FILES))
TEST_PROGRAMS = $(patsubst $(SRC)/%.cpp, $(BUILD_TESTS)/%.out, $(TEST_FILES))
TEST_DEPENDS := $(patsubst %.o,%.d,$(TEST_OBJECTS))

#build main program
all: $(BUILD)/program

all-chrono: override COMPILE=$(COMPILER) $(OPTIONS) -DCHRONO_ENABLED
all-chrono: $(BUILD)/program

$(BUILD)/program : $(SRC)/main.cpp $(OBJECTS)
	@mkdir $(dir $@) -p
	$(COMPILE) -o $@ $^

-include $(DEPENDS)
-include $(TEST_DEPENDS)

$(BUILD)/%.o : $(SRC)/%.cpp
	@mkdir $(dir $@) -p
	$(COMPILE) -MMD -MP -DTEST_FILE_NAME="\"$(notdir $<)\"" -c $< -o $@

#build tests
$(BUILD_TESTS)/%.out : $(BUILD)/%.o $(OBJECTS)
	@mkdir $(dir $@) -p
	$(COMPILE) -o $@ $^

#execute tests
define execute-test
-@./$(1)

endef

test: $(TEST_OBJECTS) $(TEST_PROGRAMS)
	@echo ''
	$(foreach test_program, $(TEST_PROGRAMS), $(call execute-test,$(test_program)))	

#clean
clean:
	@rm -rf $(BUILD)

clear: clean

.PHONY: all all-chrono test clean clear