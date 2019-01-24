/* Meleziadis Ioannis AEM 8760
   Exercise 4-Parallel code
  
   compile with gcc parallel.c -o parallel -O3 -lm -fopenmp
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>

int n ;

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
 
// BubbleSort that sorts pageRank and websites
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

	// filling outinks   
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
	scanfReturn = scanf("%lf",&d);
	
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
	

	//Main Algorithm
	double shift[n];
	int iterations = 0;
	
	// vector that has the pageranks
	double pageRank[n];
	
	for(i=0;i<n;i++){
		shift[i] = 0;
		pageRank[i] = 1.0/n;
	}
	
	double error = 1; // minimum error
	
	double temp3, temp4 ;
	
	int threads; // number of threads to create
	printf("The number of nodes is %d\n",n);
	printf("Give the number of threads :");
	scanf("%d",&threads);
	
	int perfect = 0; 
	
	
	int chunk = ceil((float)n/threads); // ceil is used so that if the division n/threads is not perfect then the algorithm still works
	
	if(ceil((float)n/threads) == (float)n/threads) perfect = 1;
	 // 1 if perfect 0 if imperfect
	
	printf("d = %f and threads = %d\n",d,threads);
	printf("Is a perfect division ? : %d\n",perfect);
	printf("n is %d threads is %d chunksize is %d\n",n,threads,chunk);
	int threadid,start,end;
	int flag = 1;
	omp_set_dynamic(0);     // explicitly disable dynamic teams
	omp_set_num_threads(threads);
	
	gettimeofday (&startwtime, NULL);
	
	if(perfect == 0){
	
		while(error >= 0.000001) {
			
			iterations++;
			error = 0.0;
			
			for(i=0; i<n; i++)
			{			
				
				shift[i] = 0; //giati alliws prosthetei stis proigoumenes times kai o algorithmos vgainei lathos
				#pragma omp parallel shared(pageRank,A)private(j,temp3,threadid,start,end) 
				{
					
					threadid = omp_get_thread_num();
					//printf("threadid = %d \n",threadid);
					//printf("threadid = %d and number of threads is %d\n",threadid,threads);
					start = threadid * chunk;
					end = start + chunk ;
					temp3 = 0.0;
					//printf(" thread %d start is %d end is %d chunksize is %d\n",threadid,start,end,chunk);
					
					if(threadid == threads-1) { // if it's the last thread then check if j >= n 
						flag = 1;
					}
					if(flag ==1){ // if it is the last thread
						for(j=start; j<end; j++){
							if(j < n) temp3 += A[i][j]*pageRank[j]; 
						}
					}
					else{
						for(j=start; j<end; j++){
							temp3 += A[i][j]*pageRank[j];
						}
					}
					#pragma omp critical
					shift[i] -= temp3;
				}
				
			shift[i] = (1 / A[i][i]) * (shift[i] + b[i]);
			pageRank[i] = pageRank[i] + shift[i];
			error +=fabs(shift[i]);
			
			}
			//printf("The error is : %f\n", error);
		}
	}
	else {
		while(error >= 0.000001) {
			
			iterations++;
			error = 0.0;
			
			for(i=0; i<n; i++)
			{			
				
				shift[i] = 0; // cause else is wrong
				#pragma omp parallel shared(pageRank,A) private(j,temp3,threadid,start,end) 
				{
					
					threadid = omp_get_thread_num();
					//printf("threadid = %d \n",threadid);
					//printf("threadid = %d and number of threads is %d\n",threadid,threads);
					start = threadid * chunk;
					end = start + chunk ;
					temp3 = 0.0;
					//printf(" thread %d start is %d end is %d chunksize is %d\n",threadid,start,end,chunk);
					
					for(j=start; j<end; j++){
						temp3 += A[i][j]*pageRank[j];
					}
					
					#pragma omp critical
					shift[i] -= temp3;
				}
				
			shift[i] = (1 / A[i][i]) * (shift[i] + b[i]);
			pageRank[i] = pageRank[i] + shift[i];
			error +=fabs(shift[i]);
			
			}
			//printf("The error is : %f\n", error);
		}
	}

	// stop counting time
	gettimeofday (&endwtime, NULL);
    seq_time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6+ endwtime.tv_sec - startwtime.tv_sec);
    printf( "Parallel time with %d threads is = %f seconds\n",threads, seq_time );         
	
	//return 0;
	
	// sort websites using their pagerank
	bubbleSort(pageRank, websites, n);
	
	// printing the 10 biggest sites
	printf("The 10 biggest sites are :\n");
	for(i=0;i<10;i++) {
		printf("%d(%f) : %s\n",i+1,pageRank[i],websites[i]);
	}
	
	printf("The number of iterations is : %d\n", iterations);
	
	return 0;
	
}