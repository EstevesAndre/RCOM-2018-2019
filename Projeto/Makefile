all: sender receiver

sender: sender.c
	gcc -o sender -Wall sender.c protocol.c llopen.c llwrite.c llclose.c
receiver: receiver.c
	gcc -o receiver -Wall receiver.c protocol.c llopen.c llread.c llclose.c -lm
clean :
	rm sender receiver \
