#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include "lab2.h"

// Global
file_access g_file;
disk * disks;
int n_disks;
int disk_width;
int qty_radius;

//Entradas: Argumento -> Estructura compartida de acceso al archivo
//Funcionamiento: Funcion de hebra hija la cual lee el archivo de forma concurrente 
//y actualiza los valores del disco según corresponda
//Salidas: NULL
void * calculateProperties(void * arg) {
    file_access *file = (file_access *) arg;
    char line[BUFFER_SIZE];
    
    while (file->file_status != END_OF_FILE) {
        double u,v,r,i,n;
        int it;
        double distance;
        int disk_id;
        printf("Hello from thread %u\n", (int)pthread_self()); 
        fseek(file->file, file->current_line, SEEK_CUR);
        for(it = file->current_line; it < file->limit_line; it++) {
            pthread_mutex_lock(&file->mutex);
            if (fgets(line, BUFFER_SIZE, file->file) != NULL) {
                sscanf(line, "%lf,%lf, %lf, %lf, %lf\n", &u, &v, &r, &i, &n);
                distance = get_distance(u, v);
                disk_id = get_disk(distance, qty_radius, disk_width);
                pthread_mutex_lock(&disks[disk_id].mutex);
                // printf("Disk: %d\n", disk_id);
                // show_disk(disks[disk_id]);
                // printf("u: %f\n", u);
                // printf("v: %f\n", v);
                // printf("r: %f\n", r);
                // printf("i: %f\n", i);
                // printf("n: %f\n", n);
                disks[disk_id].media_r += r;
                disks[disk_id].media_i += i;
                disks[disk_id].ruido += n;
                disks[disk_id].potencia += get_potencia(r,i);
                disks[disk_id].n_visibilities += 1;
                pthread_mutex_unlock(&disks[disk_id].mutex);                
            } else {
                file->file_status = END_OF_FILE;
            }
            pthread_mutex_unlock(&file->mutex);
        }
    }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char **argv)
{
    // Obtienes valores desde la linea de comandos, 
    // en caso de encontrar errores, se termina el proceso
    command_setting c_setting = get_commands(argc, argv);
    if (c_setting.n_errors > 0) {
        printf("Tienes errores\n");
        exit(0);
    }
    
    // Obtener datos desde la linea de comando
    char *filename = c_setting.filename;
    char *result_filename = c_setting.result_filename;
    int n_threads = c_setting.threads_qty;
    int b_flag = c_setting.b_flag; // 0 (TRUE), 1 (FALSE)
    n_disks = c_setting.disks_qty;
    disk_width = c_setting.disk_width;
    qty_radius = n_disks - 1;

    // Init file
    // Hebra madre solo abre el archivo en g_file.file (global)
    g_file = file_init(filename, c_setting.chunk);
    
    // Definir  e inicializar discos de acceso global
    disks = (disk*)malloc(sizeof(disk) * n_disks);
    int x;
    for(x=0;x<n_disks;x++) {
       disks[x] = disk_init(x);
    }
    
    // Creación de hebras
    int i;
    int thread;
    pthread_t thread_id[n_threads];  
    pthread_attr_t attr;
    thread = pthread_attr_init(&attr);
    for (i=0;i<n_threads;i++) {
        thread = pthread_create(&thread_id[i], &attr, calculateProperties, (void*) &g_file);
        if(thread)
        {
            printf("\n ERROR: return code from pthread_create is %d \n", thread);
            exit(1);
        }
    }
    
    // Esperar ejecución hebras hijas
    for(i=0;i<n_threads;i++){
        thread = pthread_join(thread_id[i], NULL);
    }

    // Cerrar archivo de entrada
    fclose(g_file.file);

    // Calcular valores finales (valor/n_visibilidades)
    for(x=0;x<n_disks;x++){
        disks[x].media_r = (disks[x].media_r / disks[x].n_visibilities);
        disks[x].media_i = (disks[x].media_i / disks[x].n_visibilities);
    }

    // Mostrar por consola los discos y valores en caso de que este activo "-b"   
    if (b_flag == 0) {
        for(x=0;x<n_disks;x++){
            show_disk(disks[x]);
        }
    }
    
    // Se escriben discos en archivo de salida
    write_result(result_filename, disks, n_disks);
    
    // Liberar memoria discos
    free(disks);

    // Terminar hebra madre
    pthread_exit(NULL);
    return 0;
}
