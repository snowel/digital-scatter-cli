main.c:clean
	gcc -g -o a.out main.c -lssl -lcrypto
	./a.out
clean:
	rm -f a.out
