#Makefile for Base64
#CC = gcc
INCLUDE = .
CFLAGS = -Wall -fPIC

Base64: Base64.c Base64.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c $<
	ar  rcs libBase64.a      Base64.o
clean:
	rm *.o *.a
