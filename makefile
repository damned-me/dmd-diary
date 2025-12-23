NAME=dry
CC=gcc
RM=rm
RMARGS =-rf
obj_files=main.o
src_files=main.c
cflgs=`pkg-config --cflags libconfig`
libs=`pkg-config --libs libconfig`

all: $(NAME)
.PHONY: all

install: all
	mkdir -p /etc/dry
	cp dry.conf /etc/dry/dry.conf
	cp ./completion.sh /etc/bash_completion.d/dry
	cp .build/$(NAME) /bin/$(NAME)
	chmod +x /bin/$(NAME)

uninstall: all
	rm /bin/$(NAME)
	rm /etc/dry.conf
	rm /etc/bash_completion.d/dry

run: all
	.build/$(NAME)

$(NAME): main.o
	$(CC) $(cflgs) .build/obj/$< -o .build/$@ $(libs)

%.o : %.c bdir
	$(CC)  -c $< -o .build/obj/$@

bdir:
	if [ ! -d ".build" ]; then mkdir .build; fi
	if [ ! -d ".build/obj" ]; then mkdir .build/obj; fi

.PHONY: clean
clean:
	$(RM) $(RMARGS) .build
