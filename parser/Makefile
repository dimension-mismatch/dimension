CC := clang

all := dmsn

dmsn : dmsn.c tokenizer.o parser.o token_cursor.o hash_table/hash_table.o construct_utils.o
	$(CC) -g -fsanitize=address -O0 -o $@ $^

%.o : %.c %.h
	$(CC) -g -c $< -o $@

another : 
	./dmsn ../another_test.dmsn

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