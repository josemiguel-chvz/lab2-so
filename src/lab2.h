#define READ    0
#define WRITE   1
#define END_OF_FILE -1
#define MAX_VISIBILITIES 100000
#define BUFFER_SIZE  455

struct command_setting {
	char *filename;
	char *result_filename;
	int disks_qty;
	int disk_width;
    int threads_qty;
    int chunk;
	int b_flag; // 0 TRUE ; 1 FALSE
    int n_errors;
};
typedef struct command_setting command_setting;

struct disk {
    int id;
    double media_r;
    double media_i;
    double potencia;
    double ruido;
    int n_visibilities;
    pthread_mutex_t mutex;
};
typedef struct disk disk;

struct file_access{
    int current_line;
    int limit_line;
    int file_status;
    int chunk;
    FILE* file;
    pthread_mutex_t mutex;
};
typedef struct file_access file_access;

command_setting get_commands(int argc, char** argv);
void * calculateProperties(void * i);
disk disk_init();
void show_disk(disk disk);
file_access file_init(char *filename, int chunk);
double get_distance(double u_axis, double v_axis);
int get_disk(double distance, int qty_radius, int disk_width);
double get_potencia(double r_value, double i_value);
void write_result(char *filename, disk * disks, int n_disks);