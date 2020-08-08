#include <stdio.h>
#include <sys/utime.h>
#include <math.h>
#include <iostream>
#include <chrono>
#include <mpi.h>
#include <stdint.h>
#include <Windows.h>


#define DIM 300				//matrix dimension
#define MASTER 0			//id first task
#define FROM_MASTER 1		//set message type
#define FROM_WORKER 2		//set message type


//GLOBAL VARIABLE

MPI_Status status;

double 
a[DIM][DIM],
b[DIM][DIM],
c[DIM][DIM];

double  time1,				//used to measure start time
		time2,				//used to measure stop time
		duration,			//used to measure duration time
		global;				//used to measure total time (max in all nodes)


//MAIN PROGRAM

int main(int argc, char** argv)
{
	int numtasks,					//number of tasks partitions
		taskid,						//task identifier
		source,						//id message source
		dest,						//id message destination
		nbytes,						//number of bytes in message
		mtype,						//message type
		intsize,					//size of integer in bytes
		dbsize,						//size of double float in bytes
		rows,						//rows matrix A sent to each worker
		averow, extra, offset,		//used to determine rows sent to each worker
		i, j, k, count,				//misc
		numworkers;					//number of slaves

	char arq[50],					//output file name
		* str, auxstr[5];			//auxiliar strings

	FILE* fp;
	int tid, wid;

	intsize = sizeof(int);
	dbsize = sizeof(double);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Barrier(MPI_COMM_WORLD);

	numworkers = numtasks - 1;

	/******************************** MASTER TASK ********************************/

	if (taskid == MASTER) {

		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
				a[i][j] = 0.5;

		for (i = 0; i < DIM; i++)
			for (j = 0; j < DIM; j++)
				b[i][j] = 2.0;

		//------------------------------------------------------------------------
		//send matrix data to the worker tasks
		//------------------------------------------------------------------------
		averow = DIM / numworkers;
		extra = DIM % numworkers;
		offset = 0;
		mtype = FROM_MASTER;

		//start master time
		time1 = MPI_Wtime();

		for (dest = 1; dest <= numworkers; dest++) {
			rows = (dest <= extra) ? averow + 1 : averow;
			MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
			count = rows * DIM;
			MPI_Send(&a[offset][0], count, MPI_DOUBLE, dest, 3, MPI_COMM_WORLD);
			offset = offset + rows;
		}
		//------------------------------------------------------------------------


		//------------------------------------------------------------------------
		//send matrix b to all slaves
		//------------------------------------------------------------------------
		MPI_Bcast(&b, DIM * DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		mtype = FROM_WORKER;

		for (i = 1; i <= numworkers; i++) {
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			count = rows * DIM;
			MPI_Recv(&c[offset][0], count, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
		}

		//stop master time
		time2 = MPI_Wtime();
		duration = time2 - time1;

		//master file
		char str[] = "Resultados/Master";
		for (i = 0; i < strlen(str); i++) arq[i] = *(str + i);
		arq[i] = '\0';
		_itoa_s(DIM, auxstr, 16);
		strcat_s(arq, auxstr);
	}

	/******************************** WORKER TASK ********************************/

	if (taskid > MASTER) {

		mtype = FROM_MASTER;
		source = MASTER;

		//start slave time
		time1 = MPI_Wtime();

		MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		count = rows * DIM;
		MPI_Recv(&a, count, MPI_DOUBLE, source, 3, MPI_COMM_WORLD, &status);
		count = DIM * DIM;
		MPI_Bcast(&b, DIM * DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		for (k = 0; k < DIM; k++)
			for (i = 0; i < rows; i++) {
				c[i][k] = 0.0;
				for (j = 0; j < DIM; j++)
					c[i][k] = c[i][k] + a[i][j] * b[j][k];
			}
		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);

		//stop slave time
		time2 = MPI_Wtime();
		duration = time2 - time1;

		MPI_Send(&c, rows * DIM, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);

		//slave file
		char str[] = "Resultados/Slave";
		for (i = 0; i < strlen(str); i++) arq[i] = *(str + i);
		arq[i] = '\0';
		_itoa_s(DIM, auxstr, 16);
		strcat_s(arq, auxstr);
		i = strlen(arq);
		arq[i] = '-';
		arq[i + 1] = '\0';
		_itoa_s(taskid, auxstr, 16);
		strcat_s(arq, auxstr);
	}

	MPI_Reduce(&duration, &global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
//------------------------------------------------------------------------
//results
//------------------------------------------------------------------------
	//printf(";Node %d is %f", taskid, duration); //check individual time, if necessary
	
	if (taskid == 0) {
	printf("%d;%f\n", DIM, global);
	}
//------------------------------------------------------------------------

	MPI_Finalize();
}



