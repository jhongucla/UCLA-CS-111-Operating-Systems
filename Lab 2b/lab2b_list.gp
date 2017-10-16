# NAME: Justin Hong
# EMAIL: justinjh@ucla.edu
# ID: 604565186

#! /usr/bin/gnuplot

set terminal png
set datafile separator ","

set title "List-1: Throughput vs Threads for Mutex and Spin-lock Synced List Ops"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_1.png'
plot \
	"< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | head -n7" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv | head -n7" using ($2):(1000000000/($7)) \
	title 'Spin-lock' with linespoints lc rgb 'red'

set title "List-2: Mean Time per Mutex Wait and Mean Time per Op vs Threads"
set output 'lab2b_2.png'
set logscale x 2
set xrange [0.75:]
set ylabel "Time"
unset logscale y
set logscale y 100
set key left top
plot \
	"< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | head -n7" using ($2):($7) \
	title 'Average Time per Operation' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | head -n7" using ($2):($8) \
	title 'Average Wait-for-mutex Time' with linespoints lc rgb 'red'

set title "List-3: Successful Iterations vs Threads"
set output 'lab2b_3.png'
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
unset logscale y
unset yrange
set logscale y 10
set yrange [0.75:1000]
unset y2label
set key right top
plot \
	"< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'No sync, yield=id, 4 lists' with points lc rgb 'green', \
	"< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'Mutex, yield=id, 4 lists' with points lc rgb 'red', \
	"< grep list-id-m lab2b_list.csv" using ($2):($3) \
	title 'Spin-lock, yield=id, 4 lists' with points pointtype 4 lc rgb 'blue'

set title "List-4: Throughput vs Threads for Mutex Partitioned Lists"
set output 'lab2b_4.png'
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
unset yrange
set logscale y 10
set yrange [10000:10000000]
plot \
	"< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 1 list' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-m,[0-9]*,1000,4' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 4 lists' with linespoints lc rgb 'red', \
	"< grep -e 'list-none-m,[0-9]*,1000,8' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 8 lists' with linespoints lc rgb 'violet', \
	"< grep -e 'list-none-m,[0-9]*,1000,16' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 16 lists' with linespoints lc rgb 'orange'

set title "List-5: Throughput vs Threads for Spin-lock Partitioned Lists"
set output 'lab2b_5.png'
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput"
set logscale y 10
set yrange [10000:10000000]
plot \
	"< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 1 list' with linespoints lc rgb 'green', \
	"< grep -e 'list-none-s,[0-9]*,1000,4' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 4 lists' with linespoints lc rgb 'red', \
	"< grep -e 'list-none-s,[0-9]*,1000,8' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 8 lists' with linespoints lc rgb 'violet', \
	"< grep -e 'list-none-s,[0-9]*,1000,16' lab2b_list.csv | tail -n5" using ($2):(1000000000/($7)) \
	title 'w/o yield, 1000 iterations, 16 lists' with linespoints lc rgb 'orange'
