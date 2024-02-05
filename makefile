all: server cli td kd

#make rule per il server
server: server.o includes/utility.o includes/server_utility.o includes/tables.o includes/dishes.o
	gcc -Wall server.o includes/utility.o includes/server_utility.o includes/tables.o includes/dishes.o -o server

#make rule per il client
cli: cli.o includes/utility.o includes/client_utility.o
	gcc -Wall cli.o includes/utility.o includes/client_utility.o -o cli

td: td.o includes/utility.o includes/td_utility.o
	gcc -Wall td.o includes/utility.o includes/td_utility.o -o td

kd: kd.o includes/utility.o includes/kd_utility.o
	gcc -Wall kd.o includes/utility.o includes/kd_utility.o -o kd

server.o: server.c
	gcc -Wall -c server.c -o server.o

cli.o: cli.c
	gcc -Wall -c cli.c -o cli.o

td.o: td.c
	gcc -Wall -c td.c -o td.o

kd.o: kd.c
	gcc -Wall -c kd.c -o kd.o

includes/utility.o: includes/utility.c
	gcc -Wall -c includes/utility.c -o includes/utility.o

includes/server_utility.o: includes/server_utility.c
	gcc -Wall -c includes/server_utility.c -o includes/server_utility.o

includes/tables.o: includes/tables.c
	gcc -Wall -c includes/tables.c -o includes/tables.o

includes/dishes.o: includes/dishes.c
	gcc -Wall -c includes/dishes.c -o includes/dishes.o

includes/client_utility.o: includes/client_utility.c
	gcc -Wall -c includes/client_utility.c -o includes/client_utility.o

includes/td_utility.o: includes/td_utility.c
	gcc -Wall -c includes/td_utility.c -o includes/td_utility.o

includes/kd_utility.o: includes/kd_utility.c
	gcc -Wall -c includes/kd_utility.c -o includes/kd_utility.o

clean:
	rm *.o includes/*.o server cli td kd