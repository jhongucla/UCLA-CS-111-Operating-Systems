# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

default:
	gcc -lmraa -lm -o lab4b lab4b.c

.SILENT:
check:	default
	./lab4b --period=2 --scale=K; \
	if [[ $$? -ne 1 ]]; then \
	echo "## Failed wrong scale argument test ##"; else \
	echo "## Passed wrong scale argument test ##"; \
	fi
	./lab4b --period=2 --temp; \
	if [[ $$? -ne 1 ]]; then \
	echo "## Failed wrong argument test ##"; else \
	echo "## Passed wrong argument test ##"; \
	fi

clean:
	rm -f lab4b logfile lab4b-604565186.tar.gz

dist:
	tar -czf lab4b-604565186.tar.gz lab4b.c README Makefile