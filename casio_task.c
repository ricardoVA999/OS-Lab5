/*Comentarios por Ricardo Valenzuela 18762*/

#include <sys/time.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
// !!!!!! This value is CPU-dependent !!!!!!

//#define LOOP_ITERATIONS_PER_MILLISEC 155000

#define LOOP_ITERATIONS_PER_MILLISEC 178250
//#define LOOP_ITERATIONS_PER_MILLISEC 193750

/*Definiciones de tiempo en milisegundos, microsegundos y nanosegundos*/
#define MILLISEC 	1000
#define MICROSEC	1000000
#define NANOSEC 	1000000000



/*   This is how CASIO tasks are specified 

In the file linux-2.6.24-casio/include/linux/sched.h

also in the file /usr/include/bits/sched.h (the user space scheduler file)

struct sched_param {
	int sched_priority;
#ifdef CONFIG_SCHED_SMS_POLICY
	int casio_pid;
	unsigned long long deadline;
#endif
};
*/


/*Definicion de maximos y minimos para el offset, tiempo de ejecucion y min_interval_time, todo esto en segundos*/
double min_offset,max_offset; //seconds
double min_exec_time,max_exec_time; //seconds
double min_inter_arrival_time,max_inter_arrival_time; //seconds

/*se define e lcasio id como 1*/
unsigned int casio_id,jid=1;
struct itimerval inter_arrival_time;

/*Esta funcion "quema" un milisegundo, es decir que mientras se ejecuta transcurre un milisegundo*/
void burn_1millisecs() {
	unsigned long long i;
	for(i=0; i<LOOP_ITERATIONS_PER_MILLISEC; i++);
}

/*Esta funcion quema un milisegundo segun la cantidad de milisegundos especificada enteriormente*/
void burn_cpu(long milliseconds){
	long i;
	for(i=0; i<milliseconds; i++)
		burn_1millisecs();
}

/*Se limia la variable param, reestableciendola a sus valores iniciales*/
void clear_sched_param(struct sched_param *param)
{
	param->casio_id=-1;
	param->deadline=0;
}

/*Se muestra en pantalla el casio id y el deadline*/
void print_task_param(struct sched_param *param)
{
    	printf("\npid[%d]\n",param->casio_id);
	printf("deadline[%llu]\n",param->deadline);
}

/*Se reestablecen los valores de diferentes variables de tiempo*/
void clear_signal_timer(struct itimerval *t)
{
	t->it_interval.tv_sec = 0;
	t->it_interval.tv_usec = 0;
	t->it_value.tv_sec = 0;
	t->it_value.tv_usec = 0;
}

/*Se establecen valores de diferentes variables de tiempo*/
void set_signal_timer(struct itimerval *t,double secs)
{
	t->it_interval.tv_sec = 0;
	t->it_interval.tv_usec =0 ;
	t->it_value.tv_sec = (int)secs;
	t->it_value.tv_usec = (secs-t->it_value.tv_sec)*MICROSEC;
	
}

/*Se muestran en pantalla diferentes variables de tiempo*/
void print_signal_timer(struct itimerval *t)
{
	printf("Interval: secs [%ld] usecs [%ld] Value: secs [%ld] usecs [%ld]\n",
		t->it_interval.tv_sec,
		t->it_interval.tv_usec,
		t->it_value.tv_sec,
		t->it_value.tv_usec);
}

/*Se obtiene el valor del tiempo*/
double get_time_value(double min, double max)
{
	if(min==max)
		return min;
	return (min + (((double)rand()/RAND_MAX)*(max-min)));
}

/*Comienza una task, se establecen sus tiempos y sus valores*/
void start_task(int s)
{
	printf("\nTask(%d) has just started\n",casio_id);
	set_signal_timer(&inter_arrival_time,get_time_value(min_offset,max_offset));
	setitimer(ITIMER_REAL, &inter_arrival_time, NULL);
}

/*Se "realiza" un trabajo, aqui se toma el tiempo de inicio y final y se muestran en pantalla*/
void do_work(int s)
{
	signal(SIGALRM, do_work);
	set_signal_timer(&inter_arrival_time,get_time_value(min_inter_arrival_time,max_inter_arrival_time));

	setitimer(ITIMER_REAL, &inter_arrival_time, NULL);

	clock_t start, end;
	double elapsed=0;
	start = clock();
	printf("Job(%d,%d) starts\n",casio_id,jid);
	burn_cpu(get_time_value(min_exec_time,max_exec_time)*MILLISEC);
	end = clock();
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Job(%d,%d) ends (%f)\n",casio_id,jid,elapsed);
	jid++;

}

/*Termina una task, se indica el id del task finalizado*/
void end_task(int s)
{
	printf("\nTask(%d) has finished\n",casio_id);
	exit(0);
}

/*Funcion principal del programa*/
int main(int argc, char** argv) {

	struct sched_param param;
	
	unsigned long long seed;
	int i;

	clear_signal_timer(&inter_arrival_time);
	
	clear_sched_param(&param);

	/*Asignaccion de valores para diferentes variables a utilizar*/
	param.sched_priority = 1;
	
	casio_id=param.casio_id=atoi(argv[1]);
	min_exec_time=atof(argv[2]);
	max_exec_time=atof(argv[3]);
	min_inter_arrival_time=atof(argv[4]);
	max_inter_arrival_time=atof(argv[5]);
	param.deadline=atof(argv[6])*NANOSEC;
	min_offset=atof(argv[7]);
	max_offset=atof(argv[8]);
	seed=atol(argv[9]);
	srand(seed);
	signal(SIGUSR1, start_task);
	signal(SIGALRM, do_work);
	signal(SIGUSR2, end_task);

	print_task_param(&param);
	printf("Before sched_setscheduler:priority %d\n",sched_getscheduler(0));

	if ( sched_setscheduler( 0, SCHED_CASIO , &param ) ==-1){
		perror("ERROR");
	}
	
	printf("After sched_setscheduler:priority %d\n",sched_getscheduler(0));

	//Wait for signals while the task is idle

	while(1){
		pause();	
	}



	return 0;
}
	
	
	

	
	
	
	

