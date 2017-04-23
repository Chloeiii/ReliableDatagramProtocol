.phony all:receiver sender 

receiver: receiver.c
	gcc receiver.c -o receiver
sender: sender.c
	gcc sender.c  -o sender

.PHONY clean:
clean:
	-rm -rf *.o *.exe

