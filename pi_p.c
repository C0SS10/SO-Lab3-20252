#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int tid;        // ID del hilo
    int T;          // Número total de hilos
    int n;          // Número total de intervalos
    double fH;      // Tamaño del intervalo
} ThreadArgs;

// Función que calcula 4/(1+x^2)
double f(double a)
{
    return (4.0 / (1.0 + a*a));
}

// Función que ejecuta cada hilo
void *thread_func(void *arg) {
    ThreadArgs *a = (ThreadArgs*)arg;
    int tid = a->tid;
    int T = a->T;
    int n = a->n;
    double fH = a->fH;

    // Calcular el rango de iteraciones para este hilo
    int base = n / T;
    int rem = n % T;
    int start = tid * base + (tid < rem ? tid : rem);
    int end = start + base + (tid < rem ? 1 : 0);

    // Calcular suma parcial para este rango
    double local_sum = 0.0;
    for (int i = start; i < end; ++i) {
        double x = fH * ((double)i + 0.5);
        local_sum += f(x);
    }

    // Retornar resultado mediante malloc
    double *result = malloc(sizeof(double));
    *result = local_sum;
    pthread_exit(result);
}

double CalcPi(int n, int T)
{
    struct timespec t0, t1;
    const double fH = 1.0 / (double)n;
    double fSum = 0.0;

    // Iniciar medición de tiempo
    clock_gettime(CLOCK_MONOTONIC, &t0);

    // Si T=1, ejecutar versión serial
    if (T <= 1) {
        for (int i = 0; i < n; i++) {
            double fX = fH * ((double)i + 0.5);
            fSum += f(fX);
        }
    } else {
        // Versión paralela
        pthread_t *threads = malloc(T * sizeof(pthread_t));
        ThreadArgs *args = malloc(T * sizeof(ThreadArgs));

        // Crear T hilos
        for (int i = 0; i < T; i++) {
            args[i].tid = i;
            args[i].T = T;
            args[i].n = n;
            args[i].fH = fH;
            if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
                fprintf(stderr, "Error creating thread %d\n", i);
                exit(1);
            }
        }

        // Esperar hilos y sumar resultados parciales
        for (int i = 0; i < T; i++) {
            void *ret;
            if (pthread_join(threads[i], &ret) != 0) {
                fprintf(stderr, "Error joining thread %d\n", i);
                exit(1);
            }
            double *partial_sum = (double*)ret;
            fSum += *partial_sum;
            free(partial_sum);
        }

        // Liberar memoria
        free(threads);
        free(args);
    }

    // Finalizar medición de tiempo
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    printf("Time elapsed: %.6f seconds\n", elapsed);

    return fH * fSum;
}

int main(int argc, char **argv)
{
    const double fPi25DT = 3.141592653589793238462643;
    int n = 2000000000;
    int T = 1;  // Número de hilos por defecto

    // Procesar argumentos de línea de comandos
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    if (argc > 2) {
        T = atoi(argv[2]);
    }

    if (n <= 0 || n > 2147483647) {
        printf("\nEl número de intervalos debe estar entre 0 y 2147483647\n");
        return 1;
    }

    if (T <= 0) {
        printf("\nEl número de hilos debe ser mayor que 0\n");
        return 1;
    }

    double fPi = CalcPi(n, T);

    printf("\npi es aproximadamente = %.20f\nError               = %.20f\n",
           fPi, fabs(fPi - fPi25DT));

    return 0;
}