#Compiler, Linker and Flags
GCC	= gcc
LD	= gcc
GCCFLAGS = -std=c99 -Wall -Wextra -Wshadow -Werror -Wl,-z,relro,-z,now -g
GCCFLAGS += -fstack-protector-strong -pedantic-errors

#The Directories, Source, Includes, Objects, Binary and Resources
BUILDDIR	= build
CLI		= client
SERVER		= server
TARGETDIR 	= bin
SRCEXT		= c
OBJEXT		= o

#Client Includes
CLI_INCLIST = $(shell find ./client/ -name inc -type d)
CLI_INCLIST += $(shell find ./util/ -name inc -type d)
CLI_INC		= $(addprefix -I, $(CLI_INCLIST))
#The Target Binary Program
CLI_TARGET 	 = client

#Server Includes
SERVER_INCLIST = $(shell find ./server/ -name inc -type d)
SERVER_INCLIST += $(shell find ./util/ -name inc -type d)
SERVER_INC		= $(addprefix -I, $(SERVER_INCLIST))
#The Target Binary Program
SERVER_TARGET 	 = server

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#Client List of all sources and objects
CLI_SOURCES = $(shell find ./client/ -name *.c)
CLI_SOURCES += $(shell find ./util/ -name *.c)
CLI_OBJECTS = $(addprefix $(BUILDDIR)/$(CLI)/,$(patsubst %.c,%.o, $(notdir $(CLI_SOURCES))))

#Server List of all sources and objects
SERVER_SOURCES = $(shell find ./server/ -name *.c)
SERVER_SOURCES += $(shell find ./util/ -name *.c)
SERVER_OBJECTS = $(addprefix $(BUILDDIR)/$(SERVER)/,$(patsubst %.c,%.o, $(notdir $(SERVER_SOURCES))))

VPATH 	= $(dir $(CLI_SOURCES)) $(dir $(SERVER_SOURCES))

#Default Make
all: directories $(CLI_TARGET) $(SERVER_TARGET)

#Remake
remake: clean all

#Create directories
directories:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BUILDDIR)/$(CLI)
	@mkdir -p $(BUILDDIR)/$(SERVER)
	@mkdir -p $(TARGETDIR)

#Client Compiling
$(BUILDDIR)/$(CLI)/%.o : %.c
	$(GCC) $(CLI_INC) $(GCCFLAGS) -c $< -o $@

#Client Linking
$(CLI_TARGET): $(CLI_OBJECTS)
	@echo "Linking client..."
	$(LD) $(CLI_OBJECTS) -o $(TARGETDIR)/$(CLI_TARGET)

#Server Compiling
$(BUILDDIR)/$(SERVER)/%.o : %.c
	$(GCC) $(SERVER_INC) $(GCCFLAGS) -c $< -o $@

#Linking
$(SERVER_TARGET): $(SERVER_OBJECTS)
	@echo "Linking server..."
	$(LD) $(SERVER_OBJECTS) -o $(TARGETDIR)/$(SERVER_TARGET)

#Clean all
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(TARGETDIR)

#Non-File Targets
.PHONY: all remake clean directories
