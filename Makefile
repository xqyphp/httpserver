http_server:http.o k_buffer.o k_event_linux.o k_cstd.o k_hash.o k_mpool.o k_list.o k_rbtree.o k_socket.o main.o module_html.o
	gcc -o http_server http.o k_buffer.o k_event_linux.o k_cstd.o k_hash.o k_mpool.o k_list.o k_rbtree.o k_socket.o main.o module_html.o
http.o:http.c http.h k_types.h
	gcc -c http.c
k_buffer.o:k_buffer.c k_buffer.h k_types.h
	gcc -c k_buffer.c
k_event_linux.o:k_event_linux.c k_event.h k_types.h
	gcc -c k_event_linux.c
k_cstd.o:k_cstd.c k_cstd.h k_types.h
	gcc -c k_cstd.c
k_hash.o:k_hash.c k_hash.h k_types.h
	gcc -c k_hash.c
k_mpool.o:k_mpool.c k_mpool.h k_types.h
	gcc -c k_mpool.c
k_list.o:k_list.c k_list.h k_types.h
	gcc -c k_list.c
k_rbtree.o:k_rbtree.c k_rbtree.h k_types.h
	gcc -c k_rbtree.c
k_socket.o:k_socket.c k_socket.h k_types.h
	gcc -c k_socket.c
main.o:main.c k_types.h
	gcc -c main.c
module_html.o:module_html.c module_html.h k_types.h
	gcc -c module_html.c
clean:
	rm -rf http_server http.o k_buffer.o k_event_linux.o k_cstd.o k_hash.o k_mpool.o k_list.o k_rbtree.o k_socket.o main.o module_html.o
