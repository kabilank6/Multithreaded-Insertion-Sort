/*
    The starting program to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: Kabilan Kathiravel
    UPI: kkat687

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/times.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <limits.h>


#define SIZE    16
#define BATCH_SIZE PIPE_BUF
struct size_and_data {
    int size;
    int *data;
};

struct bin_info {
    int size;
    int *data;
    // needs to change later
};

void print_data(struct size_and_data array) {
    for (int i = 0; i < array.size; ++i)
        printf("%d ", array.data[i]);
    printf("\n");
}

/* Check to see if the data is sorted. */
bool is_sorted(struct size_and_data array) {
    bool sorted = true;
    for (int i = 0; i < array.size - 1; i++) {
        if (array.data[i] > array.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct size_and_data array) {
    srand(1); // the same random data seed every time
    for (int i = 0; i < array.size; i++) {
        array.data[i] = rand() % 1000;
    }
}

/* Split the data into 4 bins. */
void split_data(struct size_and_data array, struct bin_info bins[]) {
    for (int i = 0; i < array.size; i++) {
        int number = array.data[i];
        if (number < 250) {
            bins[0].data[bins[0].size++] = number;
        } else if (number < 500) {
            bins[1].data[bins[1].size++] = number;
        } else if (number < 750) {
            bins[2].data[bins[2].size++] = number;
        } else {
            bins[3].data[bins[3].size++] = number;
        }
    }
}

/* Allocate space for the data or a bin. */
int *allocate(int size) {
    int *space;
    space = (int *)calloc(size, sizeof(int));
    if (space == NULL) {
        perror("Problem allocating memory.\n");
        exit(EXIT_FAILURE);
    }
    return space;
}

/* Move the data from the bins back to the original array. */
void move_back(struct size_and_data array, struct bin_info bins[]) {
    for (int bin = 0; bin < 4; bin++) {
        for (int i = 0; i < bins[bin].size; i++) {
            *array.data++ = bins[bin].data[i];
        }
    }
}

/* The slow insertion sort. */
void *insertion(void *params) {
    struct bin_info bin = *(struct bin_info *)params;
    for (int i = 1; i < bin.size; i++) {
        for (int j = i; j > 0; j--) {
            if (bin.data[j-1] > bin.data[j]) {
                int temp;
                temp = bin.data[j];
                bin.data[j] = bin.data[j-1];
                bin.data[j-1] = temp;
            } else {
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct size_and_data the_array;
    struct bin_info bins[4];
    int pipe_fds[2];
    int pipe_fds_two[2];
    int pipe_fds_three[2];
    

	if (argc < 2) {
		the_array.size = SIZE;
	} else {
		the_array.size = pow(2, atoi(argv[1]));
	}

    the_array.data = allocate(the_array.size);
    for (int i = 0; i < 4; i++) {
        bins[i].size = 0;
        bins[i].data = allocate(the_array.size);
    }

    produce_random_data(the_array);

    if (the_array.size < 1025)
        print_data(the_array);
    int factor;
    if (the_array.size < 16384) {
        factor = 1;
    }
    else {
        factor = BATCH_SIZE;
    }
    /*struct tms start_times, finish_times;
    time_t start_clock, finish_clock;

    start_clock = time(NULL);
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);*/
    
    if (pipe(pipe_fds) == -1) {
        perror("Pipe one did not work \n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_fds_two) == -1) {
        perror("Pipe two did not work \n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_fds_three) == -1) {
        perror("Pipe three did not work \n");
        exit(EXIT_FAILURE);
    }
    
    

    split_data(the_array, bins);
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += bins[i].size;
    }
    printf("Total size of bins: %d\n", sum);
    struct tms start_times, finish_times;
    time_t start_clock, finish_clock;

    start_clock = time(NULL);
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    int pid;
    /*for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid == 0) {
            insertion(&bins[1]);
            exit(0);
        }
    }*/
    int forkOne = fork();
    int forkTwo = fork();
    if (forkOne > 0 && forkTwo > 0) { 
        // the parent
        int forkThree = fork();
        if (forkThree == 0) {
            insertion(&bins[1]);
            for (int i = 0; i < bins[1].size; i+=factor) {
                write(pipe_fds_three[1], &(bins[1].data[i]), factor*sizeof(int));
            }
            exit(0);

        }
        insertion(&bins[0]);
        for (int i = 0; i < bins[3].size ; i+=factor) {
            read(pipe_fds[0], &(bins[3].data[i]), factor*sizeof(int));
        }
        for (int i = 0; i < bins[2].size; i+=factor) {
            read(pipe_fds_two[0], &(bins[2].data[i]), factor*sizeof(int));
        }
        for (int i = 0; i < bins[1].size; i+=factor) {
            read(pipe_fds_three[0], &(bins[1].data[i]), factor*sizeof(int));
        }
        for (int i = 0; i < 3; i++) {
            wait(NULL);
        }
        times(&finish_times);
        finish_clock = time(NULL);
        printf("finish time in clock ticks: %ld\n", finish_times.tms_utime+finish_times.tms_cutime);
        printf("Total elapsed time in seconds: %ld\n", finish_clock - start_clock);
        
        move_back(the_array, bins);


        if (the_array.size < 1025)
            print_data(the_array);

        printf(is_sorted(the_array) ? "sorted\n" : "not sorted\n");

        free(the_array.data);
        for (int i = 0; i < 4; i++) {
            free(bins[i].data);
        }

        exit(EXIT_SUCCESS);
    }
    else if (forkOne > 0 && forkTwo == 0){ 
        // the child
        insertion(&bins[3]);
        for (int i = 0; i < bins[3].size; i+=factor) {
            write(pipe_fds[1], &(bins[3].data[i]), factor*sizeof(int));
        }
        
        exit(0);
    }
    else if (forkOne == 0 && forkTwo > 0){ 
        // the child
        
        insertion(&bins[2]);
        for (int i = 0; i < bins[2].size; i+=factor) {
            write(pipe_fds_two[1], &(bins[2].data[i]), factor*sizeof(int));
        }
        exit(0);
    }
    else {
        exit(0);
    }
 
}

