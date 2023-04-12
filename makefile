NAME=dry
CC=gcc
RM=rm
RMARGS =-rf
obj_files=main.o
src_files=main.c

all: $(NAME)
.PHONY: all

install: all
	echo "default_diary = diary\ntext_editor = emacsclient -t\nvideo_player = mpv\ndefault_dir = ~/.dry/storage	" > /etc/dry.conf
	mkdir ~/.dry/storage -p
	touch ~/.dry/dry.conf
	touch ~/.dry/diaries.ref
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
	gcc .build/obj/$< -o .build/$@

%.o : %.c bdir
	$(CC) -c $< -o .build/obj/$@

bdir:
	if [ ! -d ".build" ]; then mkdir .build; fi
	if [ ! -d ".build/obj" ]; then mkdir .build/obj; fi

.PHONY: clean
clean:
	$(RM) $(RMARGS) .build
