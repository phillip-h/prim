CFLAGS = -Wall -Wextra -pedantic -std=c99 -Isrc -Ofast
LIB = prim.a
LDLIBS = -lm
TEST = prim-tests
CLI = prim

all: $(LIB) $(TEST) $(CLI)

$(LIB): src/prim.o
	ar rc $(LIB) src/prim.o
	ranlib $(LIB)

test: $(TEST)

$(TEST): $(LIB) test/primtests.o
	$(CC) $(CFLAGS) test/primtests.o -o $(TEST) $(LIB) $(LDLIBS)

cli: $(CLI)

$(CLI): $(LIB) cli/primcli.o
	$(CC) $(CFLAGS) cli/primcli.o -o $(CLI) $(LIB) $(LDLIBS)

clean:
	@$(RM) src/*.o
	@$(RM) test/*.o
	@$(RM) cli/*.o
	@$(RM) $(LIB)
	@$(RM) $(TEST)
	@$(RM) $(CLI)

