server_rasp:main.o event_linux.o http.o socket.o utils.o
	gcc -o server_rasp  main.o event_linux.o http.o socket.o utils.o
main.o:main.c event.h socket.h types.h
	gcc -c main.c
event_linux.o:event_linux.c event.h types.h
	gcc -c event_linux.c
http.o:http.c http.h types.h
	gcc -c http.c
socket.o:socket.c socket.h types.h
	gcc -c socket.c
utils.o:utils.c utils.h types.h
	gcc -c utils.c
clean:
	rm -rf server_rasp main.o event_linux.o http.o socket.o utils.o
