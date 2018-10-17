all: sender receiver

sender: sender.c
	gcc -o sender -Wall sender.c protocol.c llopen.c llwrite.c
receiver: receiver.c
	gcc -o receiver -Wall receiver.c protocol.c llopen.c llread.c -lm
clean :
	rm sender receiver \
