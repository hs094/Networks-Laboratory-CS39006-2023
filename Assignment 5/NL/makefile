cli : client.c libmysocket.a ser
	gcc -o client client.c -L. -lmysocket -lpthread -lm

ser : server.c libmysocket.a 
	gcc -o server server.c -L. -lmysocket -lpthread -lm

libmysocket.a : mysocket.o 
	ar rcs libmysocket.a mysocket.o 

mysocket.o : mysocket.c
	gcc -c mysocket.c  -lpthread -lm 

clean :
	rm -f mysocket.o client server libmysocket.a 
 