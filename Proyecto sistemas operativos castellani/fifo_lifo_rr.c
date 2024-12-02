#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ACTIVIDADES 100

// Estructura de actividad
typedef struct {
    char id[20];      // Identificador de la actividad
    int ti;           // Tiempo de inicio
    int t;            // Duración de la actividad
    int tf;           // Tiempo de finalización
    int T;            // Tiempo de retorno (tf - ti)
    int E;            // Tiempo de espera (T - t_original)
    double I;         // Índice de utilización (t_original / T)
    int t_original;   // Duración original de la actividad
} Actividad;

// Declaraciones de funciones
void leerArchivo(const char* nombreArchivo, Actividad actividades[], int* num_actividades);
void mostrarResultados(const char* metodo, Actividad actividades[], int num_actividades);
void calcularFIFO(Actividad actividades[], int num_actividades);
void calcularLIFO(Actividad actividades[], int num_actividades);
void calcularRR(Actividad actividades[], int num_actividades, int quantum);

// Función principal
int main() {
    Actividad actividades[MAX_ACTIVIDADES];
    int num_actividades = 0;

    // Leer actividades desde el archivo CSV
    leerArchivo("datos.csv", actividades, &num_actividades);
    if (num_actividades == 0) {
        printf("No se han leído actividades\n");
        return 1;
    }

    int quantum;
    printf("Ingrese el quantum para Round Robin: ");
    scanf("%d", &quantum);

    // FIFO
    Actividad fifo_actividades[MAX_ACTIVIDADES];
    memcpy(fifo_actividades, actividades, sizeof(Actividad) * num_actividades);
    clock_t start = clock();
    calcularFIFO(fifo_actividades, num_actividades);
    clock_t end = clock();
    double durationFIFO = (double)(end - start) / CLOCKS_PER_SEC;
    mostrarResultados("FIFO", fifo_actividades, num_actividades);

    // LIFO
    Actividad lifo_actividades[MAX_ACTIVIDADES];
    memcpy(lifo_actividades, actividades, sizeof(Actividad) * num_actividades);
    start = clock();
    calcularLIFO(lifo_actividades, num_actividades);
    end = clock();
    double durationLIFO = (double)(end - start) / CLOCKS_PER_SEC;
    mostrarResultados("LIFO", lifo_actividades, num_actividades);

    // Round Robin
    Actividad rr_actividades[MAX_ACTIVIDADES];
    memcpy(rr_actividades, actividades, sizeof(Actividad) * num_actividades);
    start = clock();
    calcularRR(rr_actividades, num_actividades, quantum);
    end = clock();
    double durationRR = (double)(end - start) / CLOCKS_PER_SEC;
    mostrarResultados("Round Robin", rr_actividades, num_actividades);
   

    return 0;
}

// Leer archivo CSV
void leerArchivo(const char* nombreArchivo, Actividad actividades[], int* num_actividades) {
    FILE* archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo %s\n", nombreArchivo);
        return;
    }

    char linea[256];
    *num_actividades = 0;

    // Leer la primera línea (encabezados)
    fgets(linea, sizeof(linea), archivo);

    // Leer las actividades
    while (fgets(linea, sizeof(linea), archivo)) {
        char id[20];
        int ti, t;

        sscanf(linea, "%[^,],%d,%d", id, &ti, &t);

        Actividad act;
        strcpy(act.id, id);
        act.ti = ti;
        act.t = t;
        act.t_original = t;
        act.tf = 0;
        act.T = 0;
        act.E = 0;
        act.I = 0.0;

        actividades[*num_actividades] = act;
        (*num_actividades)++;
    }

    fclose(archivo);
}

// Mostrar resultados
void mostrarResultados(const char* metodo, Actividad actividades[], int num_actividades) {
    printf("Resultados para %s:\n", metodo);
    printf("%-12s%-10s%-10s%-10s%-10s\n", "Actividad", "tf", "T", "E", "I");
    printf("----------------------------------------------------\n");

    double totalT = 0, totalE = 0, totalI = 0;
    for (int i = 0; i < num_actividades; i++) {
        printf("%-12s%-10d%-10d%-10d%-10.4f\n", actividades[i].id, actividades[i].tf, actividades[i].T, actividades[i].E, actividades[i].I);
        totalT += actividades[i].T;
        totalE += actividades[i].E;
        totalI += actividades[i].I;
    }

    printf("----------------------------------------------------\n");
    printf("%-12s%-10s%-10.2f%-10.2f%-10.4f\n", "Promedios", "-", totalT / num_actividades, totalE / num_actividades, totalI / num_actividades);
}

// FIFO (First-In, First-Out)
void calcularFIFO(Actividad actividades[], int num_actividades) {
    int tiempo = 0;

    for (int i = 0; i < num_actividades; i++) {
        Actividad* act = &actividades[i];

        if (act->ti > tiempo) {
            tiempo = act->ti;
        }

        tiempo += act->t;
        act->tf = tiempo;
        act->T = act->tf - act->ti;
        act->E = act->T - act->t_original;
        act->I = (double)act->t_original / act->T;
    }
}

// LIFO (Last-In, First-Out)
void calcularLIFO(Actividad actividades[], int num_actividades) {
    int tiempo = 0;

    // Procesar desde el final hacia el principio (usando LIFO)
    for (int i = num_actividades - 1; i >= 0; i--) {
        Actividad* act = &actividades[i];

        if (act->ti > tiempo) {
            tiempo = act->ti;
        }

        tiempo += act->t;
        act->tf = tiempo;
        act->T = act->tf - act->ti;
        act->E = act->T - act->t_original;
        act->I = (double)act->t_original / act->T;
    }
}

// Round Robin
void calcularRR(Actividad actividades[], int num_actividades, int quantum) {
    int tiempo = 0;
    int index = 0;
    int actividad_restante = num_actividades;
    
    // Hacer una cola circular de las actividades
    while (actividad_restante > 0) {
        // Obtener la actividad actual
        Actividad* act = &actividades[index];

        if (act->t > 0) {
            // Si la actividad comienza después del tiempo actual, ajustamos el tiempo
            if (act->ti > tiempo) {
                tiempo = act->ti;
            }

            // Procesar la actividad con el quantum
            if (act->t > quantum) {
                tiempo += quantum;
                act->t -= quantum;  // Restamos el tiempo que ya se procesó
            } else {
                tiempo += act->t;  // Si la actividad termina en este ciclo
                act->tf = tiempo;
                act->T = act->tf - act->ti;
                act->E = act->T - act->t_original;
                act->I = (double)act->t_original / act->T;
                act->t = 0;  // Marcamos la actividad como completada
                actividad_restante--;  // Reducimos el número de actividades restantes
            }
        }

        // Avanzamos al siguiente proceso
        index = (index + 1) % num_actividades;
    }
}

