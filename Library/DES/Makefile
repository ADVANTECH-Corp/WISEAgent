#Makefile for DES
#CC = gcc
INCLUDE = .
CFLAGS = -Wall -fPIC

DES: DES.c DES.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c $<
	ar  rcs libDES.a      DES.o
clean:
	rm *.o *.a
