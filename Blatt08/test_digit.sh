for i in 1 2 4 8 16 ; do
	echo $i
	g++ -std=c++14 -O2 -DDIGITS=$i radixsort.cpp -o radixsort.exe
	time ./radixsort.exe
done