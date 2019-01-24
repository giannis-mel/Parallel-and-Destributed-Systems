/* Meleziadis Ioannis AEM 8760 
   Exercise 4 - Serial code 
   
   compile with  gcc serial.c -o serial -lm
   */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

int n ;
int true = 1, false = 0;



void swap(double *xp, double *yp)
{
    double temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void swapStrings(char *str1, char *str2) 
{ 
  char *temp = (char *)malloc((strlen(str1) + 1) * sizeof(char)); 
  strcpy(temp, str1); 
  strcpy(str1, str2); 
  strcpy(str2, temp); 
  free(temp); 
}
 
//BubbleSort that sorts pageRank and websites
void bubbleSort(double* arr,char** web, int n){
   int i, j;
   for (i = 0; i < n-1; i++){      
       for (j = 0; j < n-i-1; j++){ 
		   if (arr[j] <= arr[j+1]) {
			   swap(&arr[j], &arr[j+1]);
			   swapStrings(web[j],web[j+1]);
		   }
	   }
   }
}



int main(int argc, char** argv) {
	
	struct timeval startwtime, endwtime;
	double seq_time;
	int connections;
	int i, j, scanfReturn;
	
	
	// reading data from hollins.dat 
	FILE *myFile = fopen("hollins.dat", "r");
	if (myFile == 0){
		fprintf(stderr, "Failed to open holins.dat .\n");
		return 1;
	}
	
	// receive number of websites and connections
	scanfReturn=fscanf(myFile,"%d %d ",&n ,&connections );
	
	char *websites[n]; 
    for (i=0; i<n; i++) 
         websites[i] = (char *)malloc( 100 * sizeof(char)); 
	
	int *numbers = (int *)malloc(n * sizeof(int));
	
	// creating arrays websites(has the names of websites) and numbers(has the index)
	for(i=0; i<n; i++){
		scanfReturn = fscanf(myFile,"%d ",&numbers[i]);
		if(scanfReturn!=1) return 1;
		// reads text until newline 
		scanfReturn = fscanf(myFile,"%[^\n]", websites[i]);
		if(scanfReturn!=1) return 1;
	}
	
	
	// from here on the websites are saved
	// 	for(i=0;i<n;i++)
	// 		printf("%d. %s\n",numbers[i],websites[i]);
	
	// creating adjacency matrix S
	double **S = (double **)malloc(n * sizeof(double *)); 
    for (i=0; i<n; i++) 
         S[i] = (double *)malloc(n * sizeof(double)); 

	for(i = 0; i < n; i++){ 
        for(j = 0; j < n; j++){ 
        	S[i][j] = 0.0;
		}
	}
	
	// creating matrix A
	double **A = (double **)malloc(n * sizeof(double *)); 
    for (i=0; i<n; i++) 
         A[i] = (double *)malloc(n * sizeof(double)); 

	for(i = 0; i < n; i++){ 
        for(j = 0; j < n; j++){ 
        	A[i][j] = 0.0;
		}
	}
	
	
	// reading the connections
	int temp1, temp2;
	
	for(i=0 ;i<connections ;i++ ) {
		scanfReturn=fscanf(myFile,"%d %d", &temp1, &temp2); // website a has a link to website b
		if(scanfReturn!=2) return 1;
		//printf("%d-%d\n",temp1,temp2);
	    S[temp1-1][temp2-1] = 1.0; // the i-th line vector has the connections of i-th website
	}
	
	fclose(myFile);
	// from here on S is full
	
	// creation of S-transpose for later use
	double **St = (double **)malloc(n * sizeof(double *)); 
    for (i=0; i<n; i++) 
         St[i] = (double *)malloc(n * sizeof(double)); 
	for(i = 0; i < n; i++){ 
        for(j = 0; j < n; j++){ 
        	St[i][j] = 0.0;
		}
	}
	
	// outlinks vector has the number of references to other websites
	int outlinks[n];

	// initialize to zero
	for (i=0; i<n; i++) {
		outlinks[i] = 0;
	}

	// filling outlinks   
	for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
			if (S[i][j] == 1.0) {
				outlinks[i]++;
			}
		}
	}


	// making the graph matrix stochastic
	for (i=0; i<n; i++){
		if(outlinks[i] == 0){ // if no links to other pages
			// then it's a dangling page
			for (j=0; j<n; j++){
				S[i][j] = (double)1.0/n; // random jump to another website (uniform)
			}
		}
		else{
			for (j=0; j<n; j++){
				if (S[i][j] != 0.0) {
					S[i][j] = S[i][j] / outlinks[i];
				}
			}
		}
	}
	
	double d; // dumping factor
	printf("Choose a value for the dumping factor d :");
	scanf("%lf",&d);
	

	// transpose S
	for (i=0; i<n; i++){
		for (j=0; j<n; j++){
			St[j][i] = S[i][j];
		}
	}

	// now we work with St = S+p*(d^T)
	// I want to create A = I-d*St
	
	// S is now I array
	for(i=0; i<n; i++){
		for(j=0; j<n; j++){
			if(i==j) S[i][j] = 1.0;
			else S[i][j] = 0.0;
		}
	}
	
	// A = I-d*St
	for(i=0; i<n; i++){
		for(j=0; j<n; j++){
			A[i][j] = S[i][j] - d * St[i][j];
		}
	}
	
	// initialize vector b
	double b[n];
	for(i=0; i<n; i++){
		b[i] = (1.0-d) * (1.0/n) ;
	}
	
	// A*x = b
	// A = I - d(St + pdT)
	// x = pageRank
	// b = (1-d) * p

	
	// Main Algorithm
	
	double shift[n];
	int iterations = 0;
	
	// vector that has the pageranks
	double pageRank[n];
	
	// initialize the pageRank vector
	for(i=0; i<n; i++) {
		pageRank[i] = (double)1/n;
	}
	
	
	for(i=0;i<n;i++){
		shift[i] = 0;
	}
	
	
	double error = 1; // minimum error
	double temp3 ;
	
	// start counting time
	gettimeofday (&startwtime, NULL);
	
	while(error >= 0.000001) {
		
		iterations++;
		error = 0.0;
		
		for(i=0; i<n; i++)
		{			
			temp3 = 0;
			for(j=0; j<n; j++){
				temp3 += A[i][j] * pageRank[j];
			}
			
			shift[i] = (1 / A[i][i]) * (b[i] - temp3) ;
			pageRank[i] = pageRank[i] + shift[i];
			error += fabs(shift[i]); // adding how much it moved
			
		}
		//printf("The error is : %f\n", error);
	}

	// stop counting time
	gettimeofday (&endwtime, NULL);
    seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6+ endwtime.tv_sec - startwtime.tv_sec);
    printf( "Non parallel time = %f seconds\n", seq_time );         
	
	//return 0 ;
	
	// sort websites using their pagerank
	bubbleSort(pageRank, websites, n);
	
	// printing the 10 biggest sites
	printf("The 10 biggest sites are :\n");
	for(i=0;i<10;i++) {
		printf("%d(%lf) : %s\n",i+1,pageRank[i],websites[i]);
	}
	
	printf("The number of iterations is : %d\n", iterations);
	
	return 0;
	
}