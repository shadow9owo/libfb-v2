GCCPARAMS = -Iinclude -g

OBJECTS=obj/main.o \
	obj/fb.o \

obj/%.o: src/%.c
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

all: main

main: $(OBJECTS)
	gcc $(OBJECTS) -o main $(GCCPARAMS)

clean:
	rm -rf main obj
