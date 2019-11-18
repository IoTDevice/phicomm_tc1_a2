include ../Makefile.mk

OBJECTDIR=objs

USERAPP_SRC = \
app_main.c

APPCFLAGS = -DSUPPORT_UART_THROUGH

USERAPP_OBJS=${addprefix $(OBJECTDIR)/,$(USERAPP_SRC:.c=.o) $(USERAPP_SRC:.c=.o)}

all:
	@mkdir -p objs
	make userapps.a 

userapps.a: $(USERAPP_OBJS)
	echo "$(AR) $(AROPTS) $@ $^"
	$(AR) $(AROPTS) $@ $^

print:
	echo "USERAPP_SRC=$(USERAPP_SRC)"
	echo "USERAPP_OBJS=$(USERAPP_OBJS)"

$(OBJECTDIR)/%.o: %.c | $(OBJECTDIR)
	@echo "-----------------make z1 target:$@ $(makefn3)-----------------"
	$(CC) $(CFLAGS) $(APPCFLAGS) -c $< -o $@

clean:
	/bin/rm -f *.o *.elf *.a *.bin *.asm 
	/bin/rm -rf $(OBJECTDIR)

