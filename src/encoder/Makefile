#./src/encoder
SRC_ENCODER = encoder.cpp encoder_parallel_native.cpp encoder_parallel_ff.cpp encoded_character.cpp encoder_table.cpp serializable_character.cpp character_serializer.cpp
TEST_ENCODER = encoder_table_tests.cpp serializable_character_tests.cpp encoded_character_tests.cpp

SRC_FILES += $(patsubst %,encoder/%,$(SRC_ENCODER))
TEST_FILES += $(patsubst %,encoder/%,$(TEST_ENCODER))