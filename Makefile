# Roll No: MT25090
# GRS_PA02 Makefile

ROLL = MT25090

CC = gcc
CFLAGS = -O2 -pthread -Wall
ZERO_FLAGS = -D_GNU_SOURCE

CLIENT = $(ROLL)_Part_A_Client

A1_SERVER = $(ROLL)_Part_A1_Server
A2_SERVER = $(ROLL)_Part_A2_Server
A3_SERVER = $(ROLL)_Part_A3_Server

all: $(CLIENT) $(A1_SERVER) $(A2_SERVER) $(A3_SERVER)

$(CLIENT):
	$(CC) $(CFLAGS) $(CLIENT).c -o $(CLIENT)

$(A1_SERVER):
	$(CC) $(CFLAGS) $(A1_SERVER).c -o $(A1_SERVER)

$(A2_SERVER):
	$(CC) $(CFLAGS) $(A2_SERVER).c -o $(A2_SERVER)

$(A3_SERVER):
	$(CC) $(CFLAGS) $(ZERO_FLAGS) $(A3_SERVER).c -o $(A3_SERVER)

clean:
	rm -f $(CLIENT) \
	      $(A1_SERVER) \
	      $(A2_SERVER) \
	      $(A3_SERVER) \
		  *.png \
		  *.txt

.PHONY: all clean
