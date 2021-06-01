/* Author: Ryan McAllister-Grum
 * Class: CSC484A Intro to Parallel Processing
 * Assignment: HW-04
 *
 * Usage: ./Factorial <N> <T>
 *      <N>: Integer value to calculate factorial.
 *      <T>: Number of threads.
 *
 * Description: Factorial.c computes the factorial (N!)
 * for a user-provided non-negative integer, spread
 * out across the user-specified number of Pthreads.
 * In particular, the multiplication of all values
 * <= N to 1 will be evenly distributed over the
 * threads, where each thread computes its partial
 * product for its given values, and then they
 * return their value from the function, to which the
 * main thread will combine into the final product.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables entered by the user that will be
 * referenced by all threads.
 */
int n;
long thread_count;
// Global variable factorial holds the resulting factorial.
long long int factorial = 1;
// Global mutex that will be used to control updates to factorial.
pthread_mutex_t mutex;
/* Global struct that will contain the arguments for Pth_Factorial;
 * in particular, it contains each thread's start value and rank.
 */
struct factorial_args {
	long rank;
	int start;
};


// Usage outputs an explanation of the program's command line parameters to stderr.
void Usage(char* program_name) {
	fprintf(stderr, "Usage: ./%s <N> <T>\n", program_name);
	fprintf(stderr, "<N>: Integer value to calculate factorial.\n");
	fprintf(stderr, "<T>: Number of threads.\n");
}

// Get_Input reads in the user's input from the command line.
void Get_Input(int argc, char **argv) {
	/* Prompt the user to enter in the
	 * integer value for calculating the
	 * factorial if they did not
	 * enter one in as a parameter.
	 */
	if (argc < 2) {
		printf("Please enter in the nonnegative integer value for the factorial: ");
		fflush(stdout);
		scanf("%i", &n);
	} else
		n = atoi(argv[1]);

	// Check whether the user's input for N is valid.	
	if (n < 0) {
		fprintf(stderr, "Error: negative integer value entered (%i).\n", n);
		fprintf(stderr, "Please enter in a nonnegative integer value for the factorial.\n");
		fprintf(stderr, "\n");
		Usage(argv[0]);
		exit(-1);
	}
	
	// Next, prompt the user for the number of threads to use.
	if (argc < 3) {
		printf("Please enter the number of threads to use: ");
		fflush(stdout);
		scanf("%li", &thread_count);
	} else
		thread_count = atol(argv[2]);
	
	// Check whether the user's input for thread_count is valid.
	if (thread_count < 1) {
		fprintf(stderr, "Error: invalid number of threads (%li).\n", thread_count);
		fprintf(stderr, "Please enter in a thread count greater than zero.\n");
		fprintf(stderr, "\n");
		Usage(argv[0]);
		exit(-1);
	}
}

/* Pth_Factorial computes the partial product for each
 * thread for their chunk of the factorial for N!.
 *  Parameters:
 *    args: contains a pointer to a struct factorial_args.
 */
void* Pth_Factorial(void* args) {
	// my_args holds this thread's arguments.
	struct factorial_args *my_args = (struct factorial_args *) args;
	// partial holds the partial product for the thread.
	long long int partial = 1;

	// len holds the number of values this thread should multiply.
	long len = 0;
	if (my_args->start != 0)
		len = n/thread_count +
			// Spread the remainder out over the threads smaller than the remainder.
			(my_args->rank <= (n - (n / thread_count) * thread_count) - 1 ? 1 : 0);


	// Compute the partial product.
	if (len > 0) {
		long i = my_args->start;
		for (; i <= my_args->start + (len - 1); i++)
			partial *= i;
		// Multiply the result to factorial.
		pthread_mutex_lock(&mutex);
		factorial *= partial;
		pthread_mutex_unlock(&mutex);
	}

	// Clean up my_args (was not freed in main).
	free(my_args);
	
	return NULL;
}

int main(int argc, char** argv) {
	// Threads holds references to the Pthreads that will execute Pth_Factorial.
	pthread_t *threads;
	
	// Prompt the user for their input values.
	Get_Input(argc, argv);
	
	// Initialize mutex.
	pthread_mutex_init(&mutex, NULL);
	
	
	// Create and initialize the threads.
	threads = malloc(sizeof(pthread_t)*thread_count);
	long i;
	int start = 1;
	for (i = 0; i < thread_count; i++) {
		// Declare a new factorial_args to hold the thread's arguments.
		struct factorial_args *args = (struct factorial_args *)malloc(sizeof(struct factorial_args));
		args->start = (i < n ? start : 0); // Set start to zero for threads that equal or exceed n.
		args->rank = i;
		
		// Create the thread and pass in the arguments.
		pthread_create(&threads[i], NULL, Pth_Factorial, (void*) args);
		
		// Increment the start value for the next thread.
		if (i < n)
			start += n/thread_count +
				// Spread the remainder out over the threads smaller than the remainder.
				(i <= (n - (n / thread_count) * thread_count) - 1 ? 1 : 0);
	}
	
	
	// Destroy the threads.
	for (i = 0; i < thread_count; i++)
		pthread_join(threads[i], NULL);
	
	
	// Output the result.
	printf("%d! = %lli\n", n, factorial);
	
	
	// Free the pthreads_t array, mutex, and exit.
	free(threads);
	pthread_mutex_destroy(&mutex);
	return 0;
}