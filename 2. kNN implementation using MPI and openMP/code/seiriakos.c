#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define totalPoints 10000
#define dimensions 784


float getDistance(float *point1,float *point2);
void swap(float *var1,float *var2);
void swap2(int *var1,int *var2);
float **alloc_2d_float(int rows, int cols);
int **alloc_2d_int(int rows, int cols);


int main(int argc, char** argv) {
	

	//anoigma tou arxeiou gia diavasma
	FILE *myFile = fopen("myFile.txt", "r");
	//elegxos an den anikse
	if (myFile == 0){
		fprintf(stderr, "Failed to open inputs/control.txt .\n");
		return 1;
	}
	
	int i,j;
	int k=30;
	int size=totalPoints;
	
	
	
	float **block;
	block = alloc_2d_float(totalPoints,dimensions);
	
	float **kNNperBlock;
	kNNperBlock = alloc_2d_float(totalPoints,k); 
	
	int **pointPerkNN;
	pointPerkNN = alloc_2d_int(totalPoints,k);
	
	float myvar;
	int scanfReturn;
	char comma;
	
	printf("before scanning\n");
	for(i=0;i<totalPoints;i++){
		for(j=0;j<dimensions;j++){
			
			//den diavazei ton xaraktira meta to float eite ine , eite ine \n
			scanfReturn=fscanf(myFile,"%f%*c",&myvar);
			//elegxos an i scanf litourgise swsta
			if(scanfReturn==1){/*ola kala*/}
			else{ printf("error in %d,%d. \n",i,j); return 1;}
				block[i][j]=myvar;
		}
	}
	
	printf("after scanning\n");
	
struct timeval startwtime, endwtime;
double seq_time;

gettimeofday (&startwtime, NULL);
	
float dist;
int m,tempPoint;
int l;
	
	
//arxikopoihsh megalo noumero se ola
for(i=0;i<totalPoints;i++){
	for(j=0;j<k;j++){
		kNNperBlock[i][j]=9999;
		pointPerkNN[i][j]=-1;
	}
}
	
	printf("before main knn\n");
	
	//algorithmos kNN 
	for(i=0;i<totalPoints;i++){
		for(j=0;j<totalPoints;j++){
			dist=getDistance(block[i],block[j]); //BLOCK ME BLOCK
			tempPoint=j+1; //to world_rank mpainei gia na einai to swsto Point
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

	
	printf("after main knn\n");
	
	fclose(myFile);
	
	myFile = fopen("solutions.txt", "r");
	
	//tipwnei gia eksakrivwsi apotelesmatwn
	for(i=0;i<totalPoints;i++){
			printf("knnpoints from point(%d): %d  ---> ",0,1+i);
			for(j=0;j<k;j++){
				printf("%d,",pointPerkNN[i][j]);
			}
			printf("\n");
	}
	
	
	
	
	
	
	
	
	
gettimeofday (&endwtime, NULL);
	
	
seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);

printf("Recursive wall clock time = %f\n", seq_time);
	
	
	
	
	return 0;
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

