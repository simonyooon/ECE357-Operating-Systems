Operating Systems Project

This project implements 3 modules: 

A spinlock module, using a provided x86 Assembly function that implements the actual spinlocking. A test file and test makefile are provided to demonstrate the functionality of the spinlocking

A semaphore module that implements a unix semaphore, using a Linked List to keep track of processes that are "waiting" for it.

And a "shell game" program that essentially tests the previous implementations. The game creates 3 semaphores, and dedicates 6 tasks to moving "pebbles" from one semaphore to another. Ie Task 1 is dedicated to moving from A -> B while Task 2 is dedicated to A -> C. The program takes in the starting number of shells in each semaphore to begin and the number of moves each task should perform. This provides a robust way to verify the semaphores are indeed working, as the same number of pebbles under each "shell" should persist at the end of each run. Furthermore, the structure of the game avoids any possibility for a deadlock situation.

To build and test the program, a Makefile was provided. Run "make" to build the executable named "shellgame.exe" and execute with the format

./shellgame.exe <# of pebbles> <# of moves>
The program was built to run on a Mac 64 bit machine - to adjust to different machine slight modifications to the tas.S should be made
