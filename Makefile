lisp: lisp.c
	$(CC) -std=c99 -Wall lisp.c mpc.c -ledit -lm -o lisp 
