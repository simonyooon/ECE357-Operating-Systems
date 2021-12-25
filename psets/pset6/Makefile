shellgame.exe: main.o sem.o spinlock.o tas64.o
	gcc -o shellgame.exe main.o sem.o spinlock.o tas64.o

main.o: main.c
	gcc -c main.c

sem.o: sem.c sem.h
	gcc -c sem.c

spinlock.o: spinlock.c spinlock.h
	gcc -c spinlock.c

tas64.o: tas64.S
	gcc -c tas64.S

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp *.h backups
	cp Makefile backups