# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

default:
	gcc -g -pthread -lrt -o lab2_list lab2_list.c SortedList.c SortedList.h

tests:
	rm -f lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	-./lab2_list --threads=1  --iterations=1   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=2   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=4   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=8   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=16  --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=2   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=4   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=8   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=16  --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=2   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=4   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=8   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=16  --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16  --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8   --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16  --yield=id --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1  --iterations=10   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=20   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=40   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=80   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=10   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=20   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=40   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=80   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=10   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=20   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=40   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=80   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40   --yield=id --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80   --yield=id --sync=s --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1  --iterations=10   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=20   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=40   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=80   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=10   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=20   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=40   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=80   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=10   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=20   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=40   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=80   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40   --yield=id --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80   --yield=id --sync=m --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1  --iterations=1000  --sync=s --lists=1  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=s --lists=4  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=s --lists=8  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=s --lists=1  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=s --lists=4  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=s --lists=8  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=s --lists=1  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=s --lists=4  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=s --lists=8  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=s --lists=1  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=s --lists=4  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=s --lists=8  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=s --lists=16 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=s --lists=1  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=s --lists=4  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=s --lists=8  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=s --lists=16 >> lab2b_list.csv

	./lab2_list --threads=1  --iterations=1000  --sync=m --lists=1  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=m --lists=4  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=m --lists=8  >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=m --lists=1  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=m --lists=4  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=m --lists=8  >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=m --lists=1  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=m --lists=4  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=m --lists=8  >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=m --lists=1  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=m --lists=4  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=m --lists=8  >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=m --lists=1  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=m --lists=4  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=m --lists=8  >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --sync=m --lists=16 >> lab2b_list.csv

profile:
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=/u/cs/ugrad/jianan/lib/libprofiler.so CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_list ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf
graphs:
	gnuplot lab2b_list.gp

sources = Makefile lab2_list.c SortedList.c SortedList.h lab2b_list.csv lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png README lab2b_list.gp profile.out

dist: default tests profile graphs
	tar -czf lab2b-604565186.tar.gz $(sources)

clean:
	rm -f lab2_list *.csv *.png *.tar.gz *.out *.gperf
