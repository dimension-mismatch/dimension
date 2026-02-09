CC := clang

all := dmsn

dmsn : dmsn.c tokenizer.o parser.o token_cursor.o hash_table/hash_table.o construct_utils.o expression_builder.o hash_table/pattern_trie.o error_handling/error_manager.o
	$(CC) -g -fsanitize=address -O0 -o $@ $^

%.o : %.c %.h
	$(CC) -g -c $< -o $@

another : 
	./dmsn ../another_test.dmsn

ir : dimension-IR/ir-interpreter.c hash_table/hash_table.o dimension-IR/ir-parser.o dimension-IR/dmsn-vm.o
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