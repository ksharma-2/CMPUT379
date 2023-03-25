make:
	make a2p1
	make a2p2

a2p1: a2p1.c++
	g++ -o a2p1 a2p1.c++

a2p2: a2p2.c++
	g++ -o a2p2 a2p2.c++
	make fifo

fifo:
	mkfifo fifo-0-1
	mkfifo fifo-0-2
	mkfifo fifo-0-3
	mkfifo fifo-1-0
	mkfifo fifo-2-0
	mkfifo fifo-3-0

clean:
	make clean_a2p1
	make clean_a2p2

clean_a2p1:
	rm a2p1

clean_a2p2:
	rm a2p2
	make clean_fifo

clean_fifo:
	rm fifo-0-1
	rm fifo-0-2
	rm fifo-0-3
	rm fifo-1-0
	rm fifo-2-0
	rm fifo-3-0	

tar:
	tar -czvf sharma-a1.tar.gz ./a1p2.c ./a1p3.cpp ./makefile ./p3.data