CC := clang

all := dmsn

dmsn : dmsn.c tokenizer.o parser.o string_utils.o type_utils/type_utils.o hash_table/type_record.o hash_table/hash_table.o type_utils/parse_tools.o hash_table/variable_record.o hash_table/function_record.o expression_builder.o expression_utils/expression_utils.o compiler/compiler.o
	$(CC) -O0 -o $@ $^

hashtest : hash_table/hashtest.c hash_table/function_record.o hash_table/type_record.o type_utils/type_utils.o hash_table/hash_table.o hash_table/variable_record.o hash_table/function_record.o
	$(CC) -o $@ $^
%.o : %.c %.h
	$(CC) -c $< -o $@

