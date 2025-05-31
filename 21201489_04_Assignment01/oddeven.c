#include <stdio.h>
#include <stdlib.h>

void checkOddEven(int arr[], int length) {
    for (int i = 0; i < length; i++) {
        printf("The element %d at index %d is %s\n", arr[i], i, arr[i] % 2 == 0 ? "even" : "odd");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) return -1;

    int n = argc - 1;
    int numbers[n];

    for (int i = 1; i < argc; i++) {
        numbers[i - 1] = atoi(argv[i]);
    }

    checkOddEven(numbers, n);

    return 0;
}
