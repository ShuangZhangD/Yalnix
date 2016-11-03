#
#	Sample Makefile for Yalnix kernel and user programs.
#	
#	Prepared by Sean Smith and Adam Salem and various Yalnix developers
#	of years past...
#
#	You must modify the KERNEL_SRCS and KERNEL_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix kernel.
#
#	You must modify the USER_SRCS and USER_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix user programs
#
#	The Yalnix kernel built will be named "yalnix".  *ALL* kernel
#	Makefiles for this lab must have a "yalnix" rule in them, and
#	must produce a kernel executable named "yalnix" -- we will run
#	your Makefile and will grade the resulting executable
#	named "yalnix".
#

#make all will make all the kernel objects and user objects
ALL = $(KERNEL_ALL) $(USER_APPS)
KERNEL_ALL = yalnix

###
BUILD_PATH = ~/Desktop/coredump
###

#List all kernel source files here.  
KERNEL_SRCS = kernel.c traphandler.c listcontrol.c processmanage.c io.c lock.c pipe.c cvar.c memorymanage.c loadprogram.c testutil.c
#List the objects to be formed form the kernel source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
KERNEL_OBJS = kernel.o traphandler.o listcontrol.o processmanage.o io.o lock.o pipe.o cvar.o memorymanage.o loadprogram.o testutil.o
#List all of the header files necessary for your kernel
KERNEL_INCS = kernel.h traphandler.h listcontrol.h processmanage.h io.h lock.h pipe.h cvar.h memorymanage.h loadprogram.h testutil.h selfdefinedstructure.h  


#List all user programs here.
USER_APPS = init testbrk bigstack
#List all user program source files here.  SHould be the same as the previous list, with ".c" added to each file
USER_SRCS = init.c testbrk.c bigstack.c
#List the objects to be formed form the user  source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
USER_OBJS = init.o testbrk.o bigstack.o
#List all of the header files necessary for your user programs
USER_INCS = 

#write to output program yalnix
YALNIX_OUTPUT = yalnix

#
#	These definitions affect how your kernel is compiled and linked.
#       The kernel requires -DLINUX, to 
#	to add something like -g here, that's OK.
#

#Set additional parameters.  Students generally should not have to change this next section

#Use the gcc compiler for compiling and linking
CC = gcc

DDIR58 = /yalnix
LIBDIR = $(DDIR58)/lib
INCDIR = $(DDIR58)/include
ETCDIR = $(DDIR58)/etc

# any extra loading flags...
LD_EXTRA = 

KERNEL_LIBS = $(LIBDIR)/libkernel.a $(LIBDIR)/libhardware.so

# the "kernel.x" argument tells the loader to use the memory layout
# in the kernel.x file..
KERNEL_LDFLAGS = $(LD_EXTRA) -L$(LIBDIR) -lkernel -lelf -Wl,-T,$(ETCDIR)/kernel.x -Wl,-R$(LIBDIR) -lhardware
LINK_KERNEL = $(LINK.c)

#
#	These definitions affect how your Yalnix user programs are
#	compiled and linked.  Use these flags *only* when linking a
#	Yalnix user program.
#

USER_LIBS = $(LIBDIR)/libuser.a
ASFLAGS = -D__ASM__
CPPFLAGS= -m32 -fno-builtin -I. -I$(INCDIR) -g -DLINUX 


##########################
#Targets for different makes
# all: make all changed components (default)
# clean: remove all output (.o files, temp files, LOG files, TRACE, and yalnix)
# count: count and give info on source files
# list: list all c files and header files in current directory
# kill: close tty windows.  Useful if program crashes without closing tty windows.
# $(KERNEL_ALL): compile and link kernel files
# $(USER_ALL): compile and link user files
# %.o: %.c: rules for setting up dependencies.  Don't use this directly
# %: %.o: rules for setting up dependencies.  Don't use this directly

all: $(ALL)	

clean:
	rm -f *.o *~ TTYLOG* TRACE $(YALNIX_OUTPUT) $(USER_APPS)  core.*

count:
	wc $(KERNEL_SRCS) $(USER_SRCS)

list:
	ls -l *.c *.h

kill:
	killall yalnixtty yalnixnet yalnix

no-core:
	rm -f core.*

$(KERNEL_ALL): $(KERNEL_OBJS) $(KERNEL_LIBS) $(KERNEL_INCS)
	$(LINK_KERNEL) -o $@ $(KERNEL_OBJS) $(KERNEL_LDFLAGS)

$(USER_APPS): $(USER_OBJS) $(USER_INCS)
	$(ETCDIR)/yuserbuild.sh $@ $(DDIR58) $@.o







