CFLAGS := -fsanitize=address,leak,undefined -ggdb3
CC := cc
LFLAGS := -ledit
OBJ := obj
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(OBJ)/%.o,$(SOURCES))

ish: $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o ish

$(OBJ)/%.o: %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@
