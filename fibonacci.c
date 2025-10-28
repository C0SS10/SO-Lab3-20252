#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Estructura para pasar datos al hilo trabajador
typedef struct {
    long long *array;  // Puntero al arreglo compartido
    int n;             // Número de elementos a generar
} thread_data_t;

/*
 * Función ejecutada por el hilo trabajador
 * Calcula los N números de Fibonacci y los almacena en el arreglo compartido
 */
void *calcular_fibonacci(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    long long *fib = data->array;
    int n = data->n;
    
    // Casos base
    if (n >= 1) {
        fib[0] = 0;
    }
    if (n >= 2) {
        fib[1] = 1;
    }
    
    // Cálculo iterativo de la secuencia
    for (int i = 2; i < n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
    }
    
    pthread_exit(NULL);
}

/*
 * Función para obtener el tiempo actual en microsegundos
 */
long long obtener_tiempo_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char *argv[]) {
    // Verificar argumentos de línea de comandos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        fprintf(stderr, "  N: Número de elementos de Fibonacci a generar\n");
        return 1;
    }
    
    // Convertir argumento a entero
    int n = atoi(argv[1]);
    
    // Validar entrada
    if (n <= 0) {
        fprintf(stderr, "Error: N debe ser un número positivo\n");
        return 1;
    }
    
    if (n > 93) {
        fprintf(stderr, "Advertencia: N > 93 puede causar desbordamiento en long long\n");
    }
    
    printf("Generando los primeros %d números de Fibonacci...\n\n", n);
    
    // Asignar memoria dinámica para el arreglo compartido
    long long *fibonacci_array = (long long *)malloc(n * sizeof(long long));
    if (fibonacci_array == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        return 1;
    }
    
    // Preparar datos para el hilo trabajador
    thread_data_t thread_data;
    thread_data.array = fibonacci_array;
    thread_data.n = n;
    
    // Variable para el identificador del hilo
    pthread_t worker_thread;
    
    // Medir tiempo de inicio
    long long inicio = obtener_tiempo_us();
    
    // Crear el hilo trabajador
    int result = pthread_create(&worker_thread, NULL, calcular_fibonacci, &thread_data);
    if (result != 0) {
        fprintf(stderr, "Error: No se pudo crear el hilo trabajador (código %d)\n", result);
        free(fibonacci_array);
        return 1;
    }
    
    printf("Hilo trabajador creado. Esperando finalización...\n");
    
    // Bloquear hasta que el hilo trabajador termine (sincronización)
    result = pthread_join(worker_thread, NULL);
    if (result != 0) {
        fprintf(stderr, "Error: pthread_join falló (código %d)\n", result);
        free(fibonacci_array);
        return 1;
    }
    
    // Medir tiempo de finalización
    long long fin = obtener_tiempo_us();
    double tiempo_total = (fin - inicio) / 1000000.0;
    
    printf("Hilo trabajador finalizado.\n\n");
    
    // Imprimir la secuencia de Fibonacci generada
    printf("Secuencia de Fibonacci:\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("F(%d) = %lld\n", i, fibonacci_array[i]);
    }
    printf("--------------------------------------------\n");
    
    // Imprimir métricas de desempeño
    printf("\nMétricas de Desempeño:\n");
    printf("  Tiempo de ejecución: %.6f segundos\n", tiempo_total);
    printf("  Elementos generados: %d\n", n);
    printf("  Hilos utilizados: 1 (trabajador) + 1 (principal)\n");
    
    // Liberar memoria
    free(fibonacci_array);
    
    return 0;
}
