PROGS = fork \
	pthread

fork: fork.c
	gcc -o fork fork.c

pthread: pthread.c
	gcc -o pthread pthread.c -lpthread

clean:
	rm -f $(PROGS)

