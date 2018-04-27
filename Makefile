all:
	g++ -std=c++11 -o test main.cpp mongoose.c -D MG_ENABLE_SSL -lssl -lcrypto

clean:
	rm -f test
