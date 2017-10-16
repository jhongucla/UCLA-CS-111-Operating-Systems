# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

default:
	gcc -lmraa -pthread -lm -o lab4c_tcp lab4c_tcp.c
	gcc -lmraa -pthread -lm -lssl -lcrypto -o lab4c_tls lab4c_tls.c

clean:
	rm -f lab4c_tcp lab4c_tls lab4c-604565186.tar.gz

dist:
	tar -czf lab4c-604565186.tar.gz lab4c_tcp.c lab4c_tls.c README Makefile