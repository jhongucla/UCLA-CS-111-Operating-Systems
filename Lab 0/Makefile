# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

.SILENT:
default:
	gcc -g -o lab0 lab0.c
check: default testOpen testCreat testCatch testFinish testReadWrite
clean: 
	rm -f lab0 *.o *.txt *.tar.gz
sources = Makefile lab0.c README breakpoint.png backtrace.png
dist: default
	tar -czf lab0-604565186.tar.gz $(sources) 
testOpen:
	echo "hello world" > open.txt; \
	chmod u-r open.txt; \
	echo "hello world" | ./lab0 --input open.txt &>/dev/null; \
	if [[ $$? -ne 2 ]]; then \
	echo "Failed open test"; else \
	echo "Passed open test"; \
	fi; \
	rm -f open.txt

testCreat:
	echo "hello world" > open.txt; \
	touch creat.txt; \
	chmod u-w creat.txt; \
	echo "hello world" | ./lab0 --input open.txt --output creat.txt &>/dev/null; \
	if [[ $$? -ne 3 ]]; then \
	echo "Failed creat test"; else \
	echo "Passed creat test"; \
	fi; \
	rm -f open.txt creat.txt

testCatch:
	echo "hello world" | ./lab0 --segfault --catch &>/dev/null; \
	if [[ $$? -ne 4 ]]; then \
	echo "Failed catch test"; else \
	echo "Passed catch test"; \
	fi

testFinish:
	echo "hello world" > open.txt; \
	./lab0 --input open.txt --output creat.txt &>/dev/null; \
	if [[ $$? -ne 0 ]]; then \
	echo "Failed finish test"; else \
	echo "Passed finish test"; \
	fi; \
	rm -f open.txt creat.txt

testReadWrite:
	echo "hello world" > open.txt; \
	./lab0 --input open.txt --output creat.txt &>/dev/null; \
	cmp open.txt creat.txt; \
	if [[ $$? -ne 0 ]]; then \
	echo "Failed read write test"; else \
	echo "Passed read write test"; \
	fi; \
	rm -f open.txt creat.txt