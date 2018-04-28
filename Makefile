macOSFlags=-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib

all:
	g++ -std=c++11 -o test main.cpp mongoose.c -D MG_ENABLE_SSL -lssl -lcrypto $(macOSFlags)
clean:
	rm -f test
