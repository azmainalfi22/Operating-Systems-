// Sleeping Student-Tutor Consultation System
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define TOTAL_STUDENTS 10
#define TOTAL_CHAIRS 3

int chairs_available = TOTAL_CHAIRS;
int students_served = 0;

pthread_mutex_t chair_mutex;
pthread_mutex_t print_mutex;
sem_t student_sem;

void* student_routine(void* arg) {
    int id = *(int*)arg;
    free(arg);

    sleep(rand() % 2); // random arrival between 0-1 seconds

    sem_wait(&student_sem);

    pthread_mutex_lock(&chair_mutex);

    if (chairs_available > 0) {
        chairs_available--;

        pthread_mutex_lock(&print_mutex);
        printf("Student %d started waiting for consultation\n", id);
        pthread_mutex_unlock(&print_mutex);

        pthread_mutex_unlock(&chair_mutex);
        sem_post(&student_sem);

        pthread_mutex_lock(&chair_mutex);

        pthread_mutex_lock(&print_mutex);
        printf("A waiting student started getting consultation\n");
        printf("Number of students now waiting: %d\n", TOTAL_CHAIRS - chairs_available - 1);
        printf("ST giving consultation\n");
        printf("Student %d is getting consultation\n", id);
        pthread_mutex_unlock(&print_mutex);

        sleep(1); // consultation duration

        students_served++;

        pthread_mutex_lock(&print_mutex);
        printf("Student %d finished getting consultation and left\n", id);
        printf("Number of served students: %d\n", students_served);
        pthread_mutex_unlock(&print_mutex);

        chairs_available++;

        pthread_mutex_unlock(&chair_mutex);
    } else {
        pthread_mutex_lock(&print_mutex);
        printf("No chairs remaining in lobby. Student %d Leaving.....\n", id);
        pthread_mutex_unlock(&print_mutex);

        sem_post(&student_sem);

        pthread_mutex_lock(&print_mutex);
        printf("Student %d finished getting consultation and left\n", id);
        printf("Number of served students: %d\n", students_served);
        pthread_mutex_unlock(&print_mutex);

        pthread_mutex_unlock(&chair_mutex);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t students[TOTAL_STUDENTS];
    srand(time(NULL));

    sem_init(&student_sem, 0, TOTAL_CHAIRS);
    pthread_mutex_init(&chair_mutex, NULL);
    pthread_mutex_init(&print_mutex, NULL);

    for (int i = 0; i < TOTAL_STUDENTS; i++) {
        int* student_id = malloc(sizeof(int));
        *student_id = i;
        pthread_create(&students[i], NULL, student_routine, student_id);
    }

    for (int i = 0; i < TOTAL_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    pthread_mutex_lock(&print_mutex);
    printf("All students have been processed. Total served: %d\n", students_served);
    pthread_mutex_unlock(&print_mutex);

    sem_destroy(&student_sem);
    pthread_mutex_destroy(&chair_mutex);
    pthread_mutex_destroy(&print_mutex);

    return 0;
}
