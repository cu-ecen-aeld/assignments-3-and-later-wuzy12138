ifndef CROSS_COMPILE
override CROSS_COMPILE=
endif

CFLAGS = -g -Wall

writer:
	$(CROSS_COMPILE)gcc $(CFLAGS) -o writer writer.c

clean:
	-rm -f writer
