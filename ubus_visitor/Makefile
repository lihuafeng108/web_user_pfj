#==================================
#project    : ubus_visitor Makefile
#version    : 1.0.0
#author     : lihf
#date       : 2021.01.04
#==================================

DIR_BIN = ./bin
DIR_3TH_INC = ../3th/inc
DIR_3TH_LIB = ../3th/lib
DIR_OBJ = ./obj
DIR_DEP = ./dep

APP_NAME = ubus_visitor.bin
TARGET = $(DIR_BIN)/$(APP_NAME)

PLATFORM ?= rsu4411

CC = arm-linux-gnueabihf-gcc
STRIP = arm-linux-gnueabihf-strip
MACHINE =
EXTLIB =
EXTDEF =

CC = arm-linux-gnueabihf-gcc
STRIP = arm-linux-gnueabihf-strip
MACHINE = -mfloat-abi=hard -mfpu=neon -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8
EXTLIB = -L$(DIR_3TH_LIB) -lubox -lubus -ljson

DEFINES = -DPLATFORM_$(shell echo $(PLATFORM) | tr '[a-z]' '[A-Z]') $(EXTDEF)
INCFLAGS = -I$(DIR_3TH_INC)
CFLAGS = -Wall $(INCFLAGS) -std=gnu99 $(DEFINES) -rdynamic -Os $(MACHINE)
LDFLAGS = -rdynamic -lpthread -lrt -ldl -lm $(EXTLIB)

vpath %.h $(DIR_3TH_INC)
vpath %.a $(DIR_3TH_LIB)
vpath %.o $(DIR_OBJ)
vpath %.d $(DIR_DEP)

INCLUDES = $(wildcard ./*h $(DIR_3TH_INC)/*.h)
SOURCES = $(wildcard ./*.c)


OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
DEPS = $(patsubst %.c,%.d,$(notdir $(SOURCES)))

all :$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(addprefix $(DIR_OBJ)/,$(OBJS)) $(LDFLAGS) -o $@
	$(STRIP) $(TARGET)

%.o:%.c %.d
	@rm -f $(DIR_OBJ)/$@
	$(CC) $(CFLAGS) -c $< -o $(DIR_OBJ)/$@
	
%.d:%.c
	@set -e
	@rm -f $(DIR_DEP)/$@
	@$(CC) $< $(INCFLAGS) -MM -o $(DIR_DEP)/$@

-include $(addprefix $(DIR_DEP)/,$(DEPS))

.PHONY : clean

clean :
	rm  -rf  $(DIR_OBJ)/*.o  $(DIR_DEP)/*.d  $(TARGET) ./$(APP_NAME) $(DIR_OBJ)/*.s

up:
	svn up

send:
	cp $(TARGET) $(APP_NAME)
	chmod 755 $(APP_NAME)
	zftp_g2n $(APP_NAME) /home/root ${ip}

