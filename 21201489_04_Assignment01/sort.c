#include <stdio.h>
#include <stdlib.h>

void sortDescending(int arr[], int length) {
    for (int i = 0; i < length - 1; i++) {
        for (int j = i + 1; j < length; j++) {
            if (arr[i] < arr[j]) {
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) return -1;

    int n = argc - 1;
    int numbers[n];

    for (int i = 1; i < argc; i++) {
        numbers[i - 1] = atoi(argv[i]);
    }

    sortDescending(numbers, n);

    for (int i = 0; i < n; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    return 0;
}
