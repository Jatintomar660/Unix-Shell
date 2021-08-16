Shell.out: Shell.c parse.h color.h 
	gcc Shell.c -o Shell.out

clean:
	rm -f Shell.out
