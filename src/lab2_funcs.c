#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include "lab2.h"

//Entradas: nombre de archivo de salida, lista de discos, cantidad de discos
//Funcionamiento: Crea un nuevo archivo de salida y escribe los discos y sus valores en el 
void write_result(char *filename, disk * disks, int n_disks) {
	FILE *file_result;
    file_result = fopen(filename, "w");
    if (file_result == NULL)
    {
        perror("No es posible abrir el archivo");
        exit(1);
    }

    int i;
    for (i=0; i<n_disks; i++) {
        disk d = disks[i];
        fprintf(file_result, "Disco %d\n", d.id);
        fprintf(file_result, "Media real: %lf\n", d.media_r);
        fprintf(file_result, "Media imaginaria: %lf\n", d.media_i);
        fprintf(file_result, "Potencia: %lf\n", d.potencia);
        fprintf(file_result, "Ruido: %lf\n", d.ruido);
    }
    fclose(file_result);
    return;
}

file_access file_init(char *filename, int chunk) {
    file_access n_file;
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        perror("No es posible abrir el archivo");
        exit(1);
    }
    n_file.current_line = 0;
    n_file.limit_line = chunk;
    n_file.chunk = chunk;
    n_file.file = f;
    n_file.file_status = READ;
    pthread_mutex_init(&n_file.mutex, NULL);
    return n_file;
}

//Entradas: id de disco
//Funcionamiento: inicializa atributos y mutex de un disco
//Salidas: disco inicializado
disk disk_init(int id) {
    disk n_disk;
    n_disk.id = id+1;
    n_disk.media_i = 0.0;
    n_disk.media_r = 0.0;
    n_disk.potencia = 0.0;
    n_disk.ruido = 0.0;
    n_disk.n_visibilities = 0;
    pthread_mutex_init(&n_disk.mutex, NULL);
    return n_disk;
}

//Entradas: Disco
//Funcionamiento: Muestra el disco y su información
void show_disk(disk disk) {
    printf("Disco %d\n", disk.id);
    printf("Media real: %f\n", disk.media_r);
    printf("Media imaginaria: %f\n", disk.media_i);
    printf("Potencia: %f\n", disk.potencia);
    printf("Ruido: %f\n", disk.ruido);
    printf("N visibilidades: %d\n", disk.n_visibilities);
}

//Entradas: eje u, eje v de la visibilidad
//Funcionamiento: calcula la distancia al centro de la visibilidad
//Salidas: distancia al centro de la visibilidad
double get_distance(double u_axis, double v_axis) {
	double distance = ((u_axis)*(u_axis))+((v_axis)*(v_axis));
	return sqrt(distance);
}

//Entradas: distancia al centro de visibilidad, cantidad de radios, ancho de disco
//Funcionamiento: usando la cantidad de radios y el ancho disco, 
//  se utiliza la distancia para calcular a que disco corresponde la visibilidad
//Salidas: id de disco
int get_disk(double distance, int qty_radius, int disk_width) {
	int i;	
	int disk;
	for(i = 0; i <= qty_radius; i++)
	{
		double radius = (double) i*disk_width;
		double next_radius = (double) (i+1)*disk_width;

		if (i == qty_radius){
			if(distance >= radius)
				disk = i;
		}
		else if (distance >= radius && distance < next_radius)
			disk = i;
	}
	return disk;
}

//Entradas: Valor real, valor imaginario de una visibilidad
//Funcionamiento: Calcula la potencia de una visibilidad
//Salidas: potencia de la visibilidad
double get_potencia(double r_value, double i_value) {
    double distancia = (r_value * r_value) + (i_value * i_value);
    return  sqrt(distancia);
}

//Entradas: Comandos consola
//Funcionamiento: usa getopt para obtener valores desde linea de comando, así también los valida
//Salidas: estructura command_setting con los comandos y sus validaciones
command_setting get_commands(int argc, char** argv) {
    /* Opciones de linea de comando */
	int opt;
    int n_errors = 0;
	command_setting c_setting;
	c_setting.b_flag = 1;
    while ((opt = getopt(argc, argv, ":i:o:d:n:h:c:b")) != -1) {
        switch(opt) {
            case 'i':
                if (strcmp(optarg, "-o") == 0 || strcmp(optarg, "") == 0) {
                    printf("Debe indicar el nombre del archivo de entrada\n");
                    n_errors++;
                } else {
                    c_setting.filename = optarg;
                    printf("Archivo de entrada: %s\n", optarg);
                }
				break;
			case 'o':
                if (strcmp(optarg, "-n") == 0 || strcmp(optarg, "") == 0) {
                    printf("Debe indicar el nombre del archivo de salida\n");
                    n_errors++;
                } else {
                    if (strcmp(optarg, "propiedades.txt") == 0) {
                        c_setting.result_filename = optarg;
                        printf("Archivo de salida: %s\n", optarg);
                    } else {
                        printf("El nombre del archivo de salida indicado es incorrecto\n");
                        n_errors++;
                    }
                }
				break;
			case 'n': // cantidad de discos
                int n_disks = atoi(optarg);
                if (n_disks > 0) {
                    c_setting.disks_qty = n_disks;
				    printf("Cantidad discos: %d\n", n_disks);
                } else {
                    printf("La cantidad de discos debe ser mayor a 0\n");
                    n_errors++;
                }
				break;
			case 'd': // ancho disco
                int disk_width = atoi(optarg);
                if (disk_width > 0) {
                    c_setting.disk_width = disk_width;
				    printf("Ancho disco: %d\n", disk_width);
                } else {
                    printf("El ancho del disco debe ser mayor a 0\n");
                    n_errors++;
                }
				break;
            case 'h': // n hebras
                int n_threads = atoi(optarg);
                if (n_threads > 0) {
                    c_setting.threads_qty = n_threads;
                    printf("Cantidad hebras: %d\n", n_threads);
                } else {
                    printf("La cantidad de hebras debe ser mayor a 0\n");
                    n_errors++;
                }
                break;
            case 'c': // n lineas a leer
                int chunk = atoi(optarg);
                if (chunk > 0) {
                    c_setting.chunk = chunk;
                    printf("Chunk: %d\n", chunk);
                } else {
                    printf("Chunk debe ser mayor a 0\n");
                    n_errors++;
                }
                break;
			case 'b':
				printf("Mostrar por consola: Activado\n");
				c_setting.b_flag = 0; // True
				break;
			case '?':
		        if (opt == 'i')
		          fprintf (stderr, "Opcion -%c requiere el nombre de archivo de entrada.\n", opt);
		      	else if(opt == 'o')
		      		fprintf (stderr, "Opcion -%c requiere el nombre de archivo de salida.\n", opt);
		      	else if(opt == 'n')
		      		fprintf (stderr, "Opcion -%c requiere indicar cantidad de discos\n", opt);
		      	else if(opt == 'd')
		      		fprintf (stderr, "Opcion -%c requiere indicar el ancho de disco\n", opt);
		        else if (isprint (opt))
		         	fprintf (stderr, "Opción desconocida `-%c'.\n", opt);
		        else
		         	fprintf (stderr,"Caracter de opción desconocida `\\x%x'.\n",opt);
			default:
				abort();	
        }
    }
    
    c_setting.n_errors = n_errors;
    return c_setting;
}