all: sender receiver

sender: sender.c
	gcc -o sender -Wall sender.c protocol.c
receiver: receiver.c
	gcc -o receiver -Wall receiver.c protocol.c
