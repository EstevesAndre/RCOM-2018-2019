all: sender receiver

sender: sender.c
	gcc -o sender -Wall sender.c protocol.c llopen.c llwrite.c
receiver: receiver.c
	gcc -o receiver -Wall -lm receiver.c protocol.c llopen.c llread.c
clean :
	rm sender receiver \
