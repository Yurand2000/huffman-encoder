.PHONY: all
all:
	@mkdir build -p
	g++ -O3 -std=c++2a -o build/a.out src/main.cpp src/file_utils.cpp src/cmd_args.cpp src/encoder/encoder_table.cpp

.PHONY: run
run: all
	@echo ''
	@./build/a.out

.PHONY: clean
clean:
	@rm -r -f build

.PHONY: test
test:
	@mkdir build -p
	@mkdir build/test -p

	g++ -O3 -std=c++2a -o build/encoder_table_tests.out src/encoder/encoder_table.cpp src/encoder/encoder_table_tests.cpp
	@echo ''
	@./build/encoder_table_tests.out