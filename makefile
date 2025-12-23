NAME=dry
CC=gcc
RM=rm
RMARGS=-rf

# Source files
SRCDIR=src
SOURCES=$(SRCDIR)/main.c $(SRCDIR)/utils.c $(SRCDIR)/config.c $(SRCDIR)/crypto.c $(SRCDIR)/entry.c $(SRCDIR)/diary.c
OBJECTS=$(patsubst $(SRCDIR)/%.c,.build/obj/%.o,$(SOURCES))

# Compiler flags
CFLAGS=-Wall -I$(SRCDIR) `pkg-config --cflags libconfig`
LIBS=`pkg-config --libs libconfig`

all: $(NAME)
.PHONY: all

install: all
	mkdir -p /etc/dry
	cp dry.conf /etc/dry/dry.conf
	mkdir -p /etc/bash_completion.d
	cp ./completion /etc/bash_completion.d/dry
	mkdir -p /usr/share/zsh/site-functions
	cp ./completion /usr/share/zsh/site-functions/_dry
	cp .build/$(NAME) /usr/local/bin/$(NAME)
	chmod +x /usr/local/bin/$(NAME)

uninstall:
	rm -f /usr/local/bin/$(NAME)
	rm -f /etc/dry/dry.conf
	rm -f /etc/bash_completion.d/dry
	rm -f /usr/share/zsh/site-functions/_dry

run: all
	.build/$(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o .build/$@ $(LIBS)

.build/obj/%.o: $(SRCDIR)/%.c | bdir
	$(CC) $(CFLAGS) -c $< -o $@

bdir:
	@mkdir -p .build/obj

.PHONY: clean
clean:
	$(RM) $(RMARGS) .build
