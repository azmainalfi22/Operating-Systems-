// Multithreaded Fibonacci Computation and Search
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int fibo_size = 0;

void* generate_fibonacci(void* param) {
    int n = *(int*)param;
    int* sequence = (int*)malloc((n + 1) * sizeof(int));

    if (sequence == NULL) {
        perror("Memory allocation failed");
        pthread_exit(NULL);
    }

    if (n >= 0) sequence[0] = 0;
    if (n >= 1) sequence[1] = 1;

    for (int i = 2; i <= n; ++i) {
        sequence[i] = sequence[i - 1] + sequence[i - 2];
    }

    pthread_exit((void*)sequence);
}

void* search_fibonacci(void* param) {
    int* array = (int*)param;
    int queries = 0, index = 0;

    printf("How many numbers you are willing to search?:\n");
    scanf("%d", &queries);

    for (int i = 1; i <= queries; ++i) {
        printf("Enter search %d:\n", i);
        scanf("%d", &index);

        if (index >= 0 && index < fibo_size) {
            printf("result of search #%d = %d\n", i, array[index]);
        } else {
            printf("result of search #%d = -1\n", i);
        }
    }

    pthread_exit(NULL);
}

int main() {
    int n = 0;
    pthread_t thread_gen, thread_search;
    void* result = NULL;

    printf("Enter the term of fibonacci sequence:\n");
    scanf("%d", &n);
    fibo_size = n + 1; 

    pthread_create(&thread_gen, NULL, generate_fibonacci, (void*)&n);
    pthread_join(thread_gen, &result);

    int* fib_sequence = (int*)result;

    for (int i = 0; i <= n; ++i) {
        printf("a[%d] = %d\n", i, fib_sequence[i]);
    }

    pthread_create(&thread_search, NULL, search_fibonacci, (void*)fib_sequence);
    pthread_join(thread_search, NULL);

    free(fib_sequence);

    return 0;
}

