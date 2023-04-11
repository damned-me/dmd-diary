NAME=dry
CC=gcc
RM=rm
RMARGS =-rf
obj_files=main.o
src_files=main.c

all: $(NAME)
.PHONY: all

install: all
	cp .build/$(NAME) /bin/$(NAME)
	chmod +x /bin/$(NAME)

run: all
	.build/$(NAME)

$(NAME): main.o
	gcc .build/obj/$< -o .build/$@

%.o : %.c bdir
	$(CC) -c $< -o .build/obj/$@

bdir:
	if [ ! -d ".build" ]; then mkdir .build; fi
	if [ ! -d ".build/obj" ]; then mkdir .build/obj; fi

.PHONY: clean
clean:
	$(RM) $(RMARGS) .build
