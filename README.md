# Multithreaded-Insertion-Sort

My first assignment for COMPSCI 340 (Operating Systems) in Semester Two 2021. The goal of the assignment was to see if we could speed up the beloved insertion sort (sorting algorithm) by running it concurrently, firstly using four threads (each of which sorts a subset of the data), then using four processes (each of which also sorts a subset of the data); either strategy (hopefully) increasing CPU utilisation and consequently significantly reducing runtime for a given size.

This assignment was done in C, and if you wish to run any of my four C files here you should aim to do so using a Unix system (for example, when I was doing this assignment, I used Ubuntu 20.04 on Windows Subsystem for Linux). This is because some libraries used here (such as pthread.h or sys/mman.h) are specifically compatible with that system, and not Windows for example. Instructions regarding how to compile and run the program at your own leisure using Bash (given you have set up a compatible environment) are outlined towards the bottom.

Perhaps the key thing to note here is that the insertion sort method used throughout each C file (even on a single thread or process) can be distinguished from a traditional insertion sort by its splitting of data into four 'bins' using the split_data method.

Below, I outline how each of my four C files are distinguished from one another:

- a1.1.c - Tries using pthread to speed up the split_data function using four threads rather than the tradition of one. However, this runs the risk of producing a race condition between multiple threads, so to deal with this I placed a mutex lock. Unfortunately it seemed that the overhead from the lock made this slower than just using one thread for the same purpose.
- a1.2.c - Tries using pthread to speed up the insertion sort using four threads (each thread corresponding to a distinct bin) rather than the tradition of one. This has no race condition concerns as each thread is working on something distinct - hence it is not necessary to add locks here. I found this to be far faster than not using pthread, which makes sense.
- a1.3.c - Tries using fork system call to speed up the insertion sort using four processes (each process corresponding to a distinct bin) rather than the tradition of one. This also has no race condition concerns for the same reason as a1.2.c, so locks aren't necessary. However as the processes don't share the bin memory with each other, we need to pass messages to bring all the changes to life. We do this using pipe - child processes write their sorted bin to a pipe from which the parent reads and updates its corresponding bins. I also found this to be far faster than a1.1.c but slightly slower than a1.2.c - perhaps due to the overhead of using processes instead of threads (message passing and context switching likely made this more computationally expensive than using the same number of threads).
- a1.4.c - Also tries using fork system call to speed up the insertion sort using four processes (each process corresponding to a distinct bin) rather than the tradition of one. This also has no race condition concerns for the same reason as a1.2.c and a1.3.c, so locks aren't necessary. However the processes do share the bin memory with each other as we use mmap() here to share bin memory between processes. As such, no pipes or anything of the sort are needed after each process has finished its insertion sort. However, the parent does need to wait for each child process to finish sorting before updating with all changes - otherwise it may terminate execution too early (that is, before the child processes have finished!). Hence the wait call is used here. This also was far faster than a1.1.c but is slower than both a1.2.c and a1.3.c.

So, how can we run any of these files?

For a c file to run, we need to compile it. However, each of these four files uses math.h and each of a1.1.c and a1.2.c uses pthread.

To compile a1.1.c for example, we can type:

```
gcc -O2 a1.1.c -o a1.1 -lm -lpthread
```

And then after that, we can execute 'a1.1' to run the program. That is

```
./a1.1
```
This runs on a default size of 2^4 (16 elements in array). You can put any positive integer x after the a1.1 to get an array size of 2^x.

For example, to run the program on a list size of 2^13 = 8192, you would put in your terminal

```
./a1.1 13
```

And you should be good to go!

If you have any questions regarding this at all, please feel free to reach out at kabilan-k@hotmail.com.

Cheers!
