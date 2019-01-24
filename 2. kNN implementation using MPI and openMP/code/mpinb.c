#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define totalPoints 10000
#define dimensions 784


float getDistance(float *point1,float *point2);
void swap(float *var1,float *var2);
void swap2(int *var1,int *var2);
float **alloc_2d_float(int rows, int cols);
int **alloc_2d_int(int rows, int cols);


int main(int argc, char** argv) {
	
	
	//
	//anoigma tou arxeiou gia diavasma
	FILE *myFile = fopen("myFile.txt", "r");
	//elegxos an den anikse
	if (myFile == 0){
		fprintf(stderr, "Failed to open inputs/control.txt .\n");
		return 1;
	}
	
	
	/********EDW KSEKINAEI TO MPI*********/
	// Initialize the MPI environment
    MPI_Init(NULL, NULL);
	
	
	
	
	int i,j;
	int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	
	
	
	//paradeigma: an processes=5 tote size=10000/5=2000 kai start=0,2000,4000,6000,8000
	//                                                        end=2000,4000,6000,8000,10000
	
	int k=30;
	int size=totalPoints/world_size;
	int start=world_rank*size; 		//arxi tou block
	int end=start+size-1;		   //telos tou block
	
	
	printf("Process %d takes the block %d-%d.\n",world_rank,start+1,end+1);
	

	
	float **block;
	block = alloc_2d_float(size,dimensions);
	
	float **kNNperBlock;
	kNNperBlock = alloc_2d_float(size,k); 
	
	int **pointPerkNN;
	pointPerkNN = alloc_2d_int(size,k);
	
	
	
	//tha ta diavazei ola alla tha pernaei sto block MONO auta pou prepei gia to kathe process(NA DW PWS NA MIN TA DIAVAZEI OLA!)
	
	float myvar;
	int scanfReturn;
	char comma;
	//int counter=0;
	
	//kathe process diavazei olo to arxio alla apothikeuei mono to block pou tou analogei
	for(i=0;i<totalPoints;i++){
		for(j=0;j<dimensions;j++){
			
			//den diavazei ton xaraktira meta to float eite ine , eite ine \n
			scanfReturn=fscanf(myFile,"%f%*c",&myvar);
			//elegxos an i scanf litourgise swsta
			if(scanfReturn==1){/*ola kala*/}
			else{ printf("error in %d,%d. \n",i,j); return 1;}
			
			//an anikei sto block pou prepei na diavasw tote to vazw sto block[][]
			if(i>=start && i<=end){
				
				block[i-start][j]=myvar;
				
			}
			if(i>end) { goto stop;}
		}
	}
	stop:{ /*H go to swsta xrhsimopoieitai edw giati prokeita gia emfoliasmenes for . H goto mpike gia kaliteri taxitita wste na min diavazei 
		xwris logo to diplo for enw i diergasia idi exei parei to block pou tis analogei*/ }
		float dist;
		int m,tempPoint;
		int l;
		
		double t1;
		MPI_Status status;
		MPI_Request req;
			
		
		//arxi xronometrisis
		MPI_Barrier(MPI_COMM_WORLD);
		if(world_rank==0) {
			t1 = MPI_Wtime(); 
		}
		
	//////////////////////KOMMATI POU STELNW PRWTA TO BLOCK KAI META KANW YPOLOGISMOUS////////////////////////
	if(world_rank==0){ //0 process
		//printf("goes in process 0\n");
		MPI_Isend(&(block[0][0]),size*dimensions, MPI_FLOAT, 1, 23, MPI_COMM_WORLD,&req);
		//printf("ends process 0\n");
	}
	else if(world_rank==world_size-1){//size-1 process
		//printf("goes in process %d\n",world_rank);
		MPI_Isend(&(block[0][0]),size*dimensions, MPI_FLOAT, 0, 23, MPI_COMM_WORLD,&req);
		
		//printf("ends process %d\n",world_rank);
	}
	else{//in between
		//printf("goes in process %d\n",world_rank);
		MPI_Isend(&(block[0][0]),size*dimensions, MPI_FLOAT, world_rank+1, 23, MPI_COMM_WORLD,&req);
	
		//printf("ends process %d\n",world_rank);
	}
		
		
		
		
		//arxikopoihsh megalo noumero se ola
		for(i=0;i<size;i++){
			for(j=0;j<k;j++){
				kNNperBlock[i][j]=9999;
				pointPerkNN[i][j]=-1;
			}
		}
		#pragma omp parallel for private(j,dist,tempPoint,m)
		//edw pairnei to arxiko block to kathe process kai vriskei ta kNN
		for(i=0;i<size;i++){
			for(j=0;j<size;j++){
				dist=getDistance(block[i],block[j]); //BLOCK ME BLOCK
				tempPoint=j+world_rank*size+1; //to world_rank mpainei gia na einai to swsto Point
				if(dist!=0){
					for(m=0;m<k;m++){
						if(dist<kNNperBlock[i][m]){ 
							swap(&dist,&kNNperBlock[i][m]);
							swap2(&pointPerkNN[i][m],&tempPoint); 
						}
					}
				}
			}
		}
		
		
	//edw tha apothikeuetai to block pou tha stelnete apo tis alles processes
	float **blockReceived;
	blockReceived = alloc_2d_float(size,dimensions); 
	
	float **blockToSend;
	blockToSend = alloc_2d_float(size,dimensions);
	
	
	//ring topology gia to arxiko block pou ixe kathe diergasia

	
	if(world_rank==0){ //0 process
		//printf("goes in process 0\n");
		MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_size-1, 23, MPI_COMM_WORLD, &status);
		//printf("ends process 0\n");
	}
	else if(world_rank==world_size-1){//size-1 process
		//printf("goes in process %d\n",world_rank);
		MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_rank-1, 23, MPI_COMM_WORLD, &status);
		
		//printf("ends process %d\n",world_rank);
	}
	else{//in between
		//printf("goes in process %d\n",world_rank);
		MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_rank-1, 23, MPI_COMM_WORLD, &status);
	
		//printf("ends process %d\n",world_rank);
	}
	
	MPI_Wait(&req, &status);
	
	
	
		for(i=0;i<size;i++){
			for(j=0;j<dimensions;j++){
				blockToSend[i][j]=blockReceived[i][j];
			}
		}
	
	
	
	
	//////KOMMATI POU KSANASTELNW PRWTA KAI META KANW YPOLOGISMOUS/////
	if(world_rank==0){ //0 process
		//printf("goes in process 0\n");
		MPI_Isend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, 1, 23, MPI_COMM_WORLD,&req);
		//printf("ends process 0\n");
	}
	else if(world_rank==world_size-1){//size-1 process
		//printf("goes in process %d\n",world_rank);
		MPI_Isend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, 0, 23, MPI_COMM_WORLD,&req);
		
		//printf("ends process %d\n",world_rank);
	}
	else{//in between
		//printf("goes in process %d\n",world_rank);
		MPI_Isend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, world_rank+1, 23, MPI_COMM_WORLD,&req);
	
		//printf("ends process %d\n",world_rank);
	}
	
	
	
	
	
	
	
	int receivedFrom;
	if(world_rank==0) receivedFrom=world_size-1;
	else receivedFrom=world_rank-1;
	
	
	
	#pragma omp parallel for private(j,dist,tempPoint,m)
	//EDW THA KANW KNN ME TA BLOCK POU PIRE I KATHE DIERGASIA STO blockReceived
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			dist=getDistance(block[i],blockReceived[j]); //BLOCK ME BLOCK RECEIVED
			tempPoint=j+size*receivedFrom+1;             //to receivedFrom einai apo pia pire
			for(m=0;m<k;m++){
				if(dist<kNNperBlock[i][m]) {
					swap(&dist,&kNNperBlock[i][m]);
					swap2(&pointPerkNN[i][m],&tempPoint); 
				}
			}
		}
	}

		int flagg;
   
	
	
	
	
	
	

	
	
	
	//EDW INE TO SIMIO POU THA YLOPOIEITAI TO RING TOPOLOGY KAI TA SEND RECEIVE 

	

	int TIMES;
	int temporary;


	
	//edw pleon stelnei to blockReceived to opoio exei parei apo tin proigoumeni process kai dexete to blockReceived apo tin proproigoumeni process
	for(TIMES=0;TIMES<world_size-2;TIMES++){
		

		
		if(world_rank==0){ //0 process
			//printf("goes in process 0\n");
			
			MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_size-1, 23, MPI_COMM_WORLD, &status);
			MPI_Wait(&req, &status);
			//aparaitito gia na doulepsei to mpi
			for(i=0;i<size;i++){
				for(j=0;j<dimensions;j++){
					blockToSend[i][j]=blockReceived[i][j];
				}
			}
			if(TIMES<world_size-3)
			MPI_Issend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, 1, 23, MPI_COMM_WORLD,&req);
			//printf("ends process 0\n");
		}
		else if(world_rank==world_size-1){//size-1 process
			//printf("goes in process %d\n",world_size-1);
			
			
			MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_rank-1, 23, MPI_COMM_WORLD, &status);
			MPI_Wait(&req, &status);
			//aparaitito gia na doulepsei to mpi
			for(i=0;i<size;i++){
				for(j=0;j<dimensions;j++){
					blockToSend[i][j]=blockReceived[i][j];
				}
			}
			if(TIMES<world_size-3)
			MPI_Issend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, 0, 23, MPI_COMM_WORLD,&req);
			//printf("ends process %d\n",world_size-1);
		}
		else{//in between
			//printf("goes in process %d\n",world_rank);
			
			
			MPI_Recv(&(blockReceived[0][0]), size*dimensions, MPI_FLOAT, world_rank-1, 23, MPI_COMM_WORLD, &status);
			MPI_Wait(&req, &status);
			
			//aparaitito gia na doulepsei to mpi
			for(i=0;i<size;i++){
				for(j=0;j<dimensions;j++){
					blockToSend[i][j]=blockReceived[i][j];
				}
			}
			if(TIMES<world_size-3)
			MPI_Issend(&(blockToSend[0][0]),size*dimensions, MPI_FLOAT, world_rank+1, 23, MPI_COMM_WORLD,&req);
			//printf("ends process %d\n",world_rank);
		}
		
	
	//algotithmos gia na vrei apo pio komvo proerxonte ta dedomena pou elave i diergasia 
			for(i=0;i<world_size-2;i++){
		receivedFrom=world_rank-2-TIMES;
		if(receivedFrom<0){
			temporary=-receivedFrom;
			receivedFrom=world_size-temporary;
		}
	}
		
			

		//ksanakanei KNN me block kai block received
			#pragma omp parallel for private(j,dist,tempPoint,m)
			for(i=0;i<size;i++){
			for(j=0;j<size;j++){
				dist=getDistance(block[i],blockReceived[j]); //BLOCK ME BLOCK RECEIVED
				tempPoint=j+1+receivedFrom*size;
				for(m=0;m<k;m++){
					if(dist<kNNperBlock[i][m]) {
						swap(&dist,&kNNperBlock[i][m]);
						swap2(&pointPerkNN[i][m],&tempPoint); 
					}
				}
			}
		}
		
		
		
		

		
	}
	
	
		
		//telos xronometrisis
		MPI_Barrier(MPI_COMM_WORLD);
		
		if(world_rank==0){
			
			double t2 = MPI_Wtime(); 
			t2=t2-t1;
			printf("Elapsed time is %lf!\n", t2);
		}	
	
	
	
	
	
	/////////////ELEGXOS GIA TO AN PERNANE TA POINTS KNN////////////////////
	/*
	if(world_rank==0){
		
			for(i=0;i<2;i++){
				printf("knnpoints from point(%d): %d  ---> ",world_rank,world_rank*size+1+i);
			for(j=0;j<k;j++){
				printf("%d,",pointPerkNN[i][j]);
			}
			printf("\n");
		}
	}
				if(world_rank==world_size-1){
			for(i=0;i<2;i++){
				printf("knnpoints from point(%d): %d  ---> ",world_rank,world_rank*size+1+i);
			for(j=0;j<k;j++){
				printf("%d,",pointPerkNN[i][j]);
			}
			printf("\n");
		}
	}
	
	if(world_rank==5){
			for(i=0;i<2;i++){
				printf("knnpoints from point(%d): %d  ---> ",world_rank,world_rank*size+1+i);
			for(j=0;j<k;j++){
				printf("%d,",pointPerkNN[i][j]);
			}
			printf("\n");
		}
	}
	
	*/
	
	///////////EDW THA KANW TIN EKSAKRIVWSI APOTELESMATWN//////////////////
	
	fclose(myFile);
	
	myFile = fopen("solutions.txt", "r");
	
	
	int myvar2;
	int errors=0;
	
		for(i=0;i<totalPoints;i++){
		for(j=0;j<k;j++){
			
			//den diavazei ton xaraktira meta to float eite ine , eite ine \n
			scanfReturn=fscanf(myFile,"%d%*c",&myvar2);
			//elegxos an i scanf litourgise swsta
			if(scanfReturn==1){/*ola kala*/}
			else{ printf("error in %d,%d. \n",i,j); return 1;}

			if(i>=start && i<=end){
				
				if(pointPerkNN[i-start][j]!=myvar2) errors++;
			}
			if(i>end) { goto stop2;}
		}
	}
	

	
	
	
	
	
	stop2:{/*me ton idio tropo ginete o elegxos kai edw ksexwrista apo kathe diergasia*/}
	
	
	
	
	
	//////////KOMMATI POU THA STELNW TO PLITHOS TWN ERRORS APO KATHE DIERGASIA STIN DIERGASIA 0 WSTE NA VGALW ENA POSOSTO ERRORS/////////////////
	
	if(world_rank==0){ //0 process
			int errorArray[world_size];
			errorArray[0]=errors;
			for(i=1;i<world_size;i++){
				MPI_Recv(&errorArray[i], 1, MPI_INT, MPI_ANY_SOURCE, 23, MPI_COMM_WORLD, &status);
			}
			int errorsCounter=0;
			for(i=0;i<world_size;i++) errorsCounter+=errorArray[i];
			printf("The number of errors is %d.\n",errorsCounter);	
		}
		else if(world_rank==world_size-1){//size-1 process
			MPI_Send(&errors,1, MPI_INT, 0, 23, MPI_COMM_WORLD);
		}
		else{//in between
			MPI_Send(&errors,1, MPI_INT, 0, 23, MPI_COMM_WORLD);
		}
	

	
/*//////////////////KWDIKAS POU XRISIMOPOIITHKE GIA THN EKSAKRIVWSI TIS SWSTIS LEITOURGIAS TOU PROGRAMMATOS////////////////////
	//tipwnei ta knnperBlock gia tin rank4
	if(world_rank==4){
		for(i=0;i<size;i++){
			for(j=0;j<k;j++){
			printf("kNNperBlock[%d][%d]=%lf\n",i,j,kNNperBlock[i][j]);
		}
		}
	/*
	
	if(world_rank==0){
		printf("Printing the first 4 dimensions of the 3 first points(1,2,3) world=%d\n",world_rank);
		for(i=0;i<3;i++){
			for(j=0;j<4;j++){
				printf("%f[%d][%d]",block[i][j],i,j);
			}
			printf("\n");
		}
		
		printf("Printing the first 4 dimensions of the 2 last points(1999,2000) world=%d\n",world_rank);
		for(i=1998;i<2000;i++){
			for(j=0;j<4;j++){
				printf("%f[%d][%d]",block[i][j],i,j);
			}
			printf("\n");
		}
		
		
	}
	
	if(world_rank==1){
		printf("Printing the first 4 dimensions of the 3 first points(2001,2002,2003)world=%d\n",world_rank);
		for(i=0;i<3;i++){
			for(j=0;j<4;j++){
				printf("%f",block[i][j]);
			}
			printf("\n");
		}
		
		
		printf("Printing the first 4 dimensions of the 2 last points(3999,4000) world=%d\n",world_rank);
		for(i=1998;i<2000;i++){
			for(j=0;j<4;j++){
				printf("%f[%d][%d]",block[i][j],i,j);
			}
			printf("\n");
		}
	}
	
	if(world_rank==2){
		printf("Printing the first 4 dimensions of the 3 first points(4001,4002,4003) world=%d\n",world_rank);
		for(i=0;i<3;i++){
			for(j=0;j<4;j++){
				printf("%f",block[i][j]);
			}
			printf("\n");
		}
		
		printf("Printing the first 4 dimensions of the 2 last points(5999,6000) world=%d\n",world_rank);
		for(i=1998;i<2000;i++){
			for(j=0;j<4;j++){
				printf("%f[%d][%d]",block[i][j],i,j);
			}
			printf("\n");
		}
	}
	
	*/
/*	
		if(world_rank==4){
		printf("Printing the first 4 dimensions of the 3 first points(8001,8002,8003) world=%d\n",world_rank);
		for(i=0;i<3;i++){
			for(j=0;j<4;j++){
				printf("%f",block[i][j]);
			}
			printf("\n");
		}
		
		printf("Printing the first 4 dimensions of the 2 last points(9999,10000) world=%d\n",world_rank);
		for(i=1998;i<2000;i++){
			for(j=0;j<4;j++){
				printf("%f[%d][%d]",block[i][j],i,j);
			}
			printf("\n");
		}
	}
	
*/
	
	
	/******EDW TELEIWNEI TO MPI******/
	// Finalize the MPI environment.
    MPI_Finalize();
	
	
	
	
}








//sinartisi gia ypologismo apostasis metaksi 2 simiwn(grammwn)
float getDistance(float *point1,float *point2){
	
	
	
	float distance;
	int i;
	distance=0;
	
	
	for(i=0;i<dimensions;i++){
		distance+=(point1[i]-point2[i])*(point1[i]-point2[i]);
	}
	distance = sqrt(distance);
	return distance;
}


//swapping function for floats
void swap(float *var1,float *var2){
	float temp;
   temp  = *var1;
   *var1 = *var2;
   *var2 = temp;
}

//swapping function for ints
void swap2(int *var1,int *var2){
	int temp;
   temp  = *var1;
   *var1 = *var2;
   *var2 = temp;
}

//allocating 2d array se sinexomens theseis mnimis gia na mporw na to steilw apo MPI
//source https://stackoverflow.com/questions/5901476/sending-and-receiving-2d-array-over-mpi
float **alloc_2d_float(int rows, int cols) {
	int i;
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

int **alloc_2d_int(int rows, int cols) {
	int i;
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}


