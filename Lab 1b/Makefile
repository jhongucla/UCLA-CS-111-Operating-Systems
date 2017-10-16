# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

default: client server
client:
	gcc -lmcrypt -g -o lab1b-client lab1b-client.c
server:
	gcc -lmcrypt -g -o lab1b-server lab1b-server.c
clean:
	rm -f lab1b-client lab1b-server lab1b-604565186.tar.gz
sources = Makefile README lab1b-client.c lab1b-server.c my.key
dist:
	tar -czvf lab1b-604565186.tar.gz $(sources)