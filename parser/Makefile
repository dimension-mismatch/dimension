CC := clang

all := dmsn

.PHONY: clean

dmsn : dmsn.c tokenizer.o parser.o token_cursor.o hash_table/hash_table.o construct_utils.o expression_builder.o hash_table/pattern_trie.o error_handling/error_manager.o dimension-IR/ir_construct_utils.o dimension-IR/ir_parser.o variable_to_register.o dimension-IR/registers.o compiler/compiler.o dimension-IR/program_builder.o compiler/compute_size.o
	$(CC) -g -fsanitize=address -O0 -Wall -o $@ $^

%.o : %.c %.h
	$(CC) -g -c $< -o $@

another : 
	./dmsn ../another_test.dmsn

ir : hash_table/hash_table.o dimension-IR/ir_parser.o dimension-IR/ir_construct_utils.o
	$(CC) -g -fsanitize=address -O0 -o $@ $^
test :
	as  ../test.s -o ../test.o
	gcc -o ../test ../test.o -nostdlib -static 
	../test
test.s :
	./dmsn ../test.dmsn ../test.s

assemble :
	as  ../test.s -o ../test.o
	gcc -o ../test ../test.o -nostdlib -static 
	../test

clean :
	rm -f *.o
	rm -f validator/*.o
	rm -f hash_table/*.o
	rm -f error_handling/*.o
	rm -f dimension-IR/*.o
	rm -rf dmsn.dSYM