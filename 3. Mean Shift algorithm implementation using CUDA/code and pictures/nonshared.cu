#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <cuda.h>
#include <cuda_runtime.h> 
#include <cuda_runtime_api.h>
#include <string.h>

float s = 1;
#define points 600
#define dimensions 2



//gia compile : nvcc nonshared.cu -o nonshared -O3 -lm 
//gia run: ./nonshared 0.5









//sinartisi pou epistrefei tin apostasi 2 simiwn
__device__ float distanceFunction(float point1[2],float point2[2]){

  float distance;
  int i;
  distance=0;

  for(i=0;i<dimensions;i++){
    distance+=(point1[i]-point2[i])*(point1[i]-point2[i]);
  }

  distance = sqrt(distance);
  return distance;
}

//sinartisi pou epistrefei to varos
__device__ float kernelFunction(float pointYk[2],float arrayXi[2]){

  float s=1;
  float returnWeight=0;
  float distance=distanceFunction(pointYk,arrayXi);
  distance*=distance;

  returnWeight=exp(-((distance)/(2*(s*s))));
  return returnWeight;
}

//epistrefei tin apostasi pou kineitai to moved
__device__ float movedDistance(float moved[2]){
  float distance;
  int i;
  distance=0;

  for(i=0;i<dimensions;i++){
    distance+=(moved[i])*(moved[i]);
  }

  distance = sqrt(distance);
  return distance;
}













__global__ void shiftingFunction(float *Ykplus1, float *Yk, float *X, float e){
	
	
	
	int index = blockIdx.x *blockDim.x +  threadIdx.x;
	//printf("index is %d\n",index);
		
 int i,j;

  /*//tipwnei tis 2 diastaseis kathe simiou (elegxos)
  for(i=0;i<dimensions;i++){
    printf("%f ",pointYk[i]);
  }
  printf("\n");
*/


  int s=1;
  
  float arithmitis[dimensions];
  for(i=0;i<dimensions;i++) { arithmitis[i]=0;}
  float paronomastis=0;
  float weightFromGaussian=0;
  float check,S=s*s;

  float Ypoint[2],Xpoint[2],moved[2];
  


  
  moved[0]=9;moved[1]=9;
  
  
  while(movedDistance(moved)>=e){
	  
	    //auto ine to simio pou kathorizete apo to index
	  Ypoint[0]=Yk[index*2];
      Ypoint[1]=Yk[index*2+1];
	  
	  paronomastis=0;
      weightFromGaussian=0;
	  for(i=0;i<dimensions;i++) { arithmitis[i]=0;}
	  
	//ipologizw gia kathe simio tou X an tha mpei sti sinartisi
	for(i=0;i<points;i++){
		
		
		Xpoint[0]=X[i*2];
		Xpoint[1]=X[i*2+1];
		
		check=distanceFunction(Ypoint,Xpoint);
		
		//printf("distance(%d)=%f and s^2=%f\n",i,check,S);
		
		if(check<=S && check>0) 
		{
		weightFromGaussian=kernelFunction(Ypoint,Xpoint);
		for(j=0;j<dimensions;j++){
			arithmitis[j]+= weightFromGaussian*X[i*dimensions+j];
		}
		paronomastis+=weightFromGaussian;
		}
		else { }
	}

	for(j=0;j<dimensions;j++){
		Ykplus1[index*2+j]=arithmitis[j]/paronomastis;
	}


	for(j=0;j<dimensions;j++){
		moved[j]=Ykplus1[index*2+j]-Yk[index*2+j];
		Yk[index*2+j]=Ykplus1[index*2+j];
	}


  }





  
}

















int main(int argc, char **argv){

  //anoigma tou arxeiou gia diavasma
  FILE *myFile = fopen("eisodos.txt", "r");
  //elegxos an den anikse
  if(myFile==0){
    fprintf(stderr,"Failed to open eisodos.txt.\n");
    return 1;
  }

  
  /*
  if(freopen("stdout.txt","w",stdout)==0) {
	fprintf(stderr,"Failed to write in stdout.txt.\n");
    return 1;
  }*/
  
  
  //oi pinakes tha einai 2d->1d
  
  //pinakes tou host
  float *arrayStatic, *arrayYk /*,*arrayYkplus1*/;
  //pinakes tou device
  float *deviceArrayStatic, *deviceArrayYk, *deviceArrayYkplus1;
  
  
  
  //megethos pinakwn
  int nBytes=points*dimensions*sizeof(float);
  
  //malloc gia pinakes tou host
  arrayStatic=(float *)malloc(nBytes);
  arrayYk=(float *)malloc(nBytes);
  //arrayYkplus1=(float *)malloc(nBytes);
  
  //malloc gia pinakes tou device
  cudaMalloc((void **)&deviceArrayStatic,nBytes);
  cudaMalloc((void **)&deviceArrayYk,nBytes);
  cudaMalloc((void **)&deviceArrayYkplus1,nBytes);
  
  
  int i,j,scanfReturn;
  float myvar;
  
  //diavasma tou pinaka apo to arxeio dataset.txt
  for(i=0;i<points;i++){
	for(j=0;j<dimensions;j++){
      //den diavazei ton xaraktira meta to float eite ine , eite ine \n
      scanfReturn=fscanf(myFile,"%f%*c",&myvar);

      //elegxos an i scanf litourgise swsta
      if(scanfReturn==1){/*ola kala*/}
      else{ printf("error in %d,%d. \n",i,j); return 1;}

      //ekxwrisi timwn ston pinaka arrayStatic
      arrayStatic[i*dimensions+j]=myvar;
      //stin arxi ta simia tou arrayYk einai idia me ta sima tou arrayStatic
      arrayYk[i*dimensions+j]=myvar;
    }
  }

  printf("Finished reading from file\n");
  fclose(myFile);
  
  //metafora apo tous pinakes tou host stous pinakes tou device
  cudaMemcpy(deviceArrayStatic,arrayStatic,nBytes,cudaMemcpyHostToDevice);
  cudaMemcpy(deviceArrayYk,arrayYk,nBytes,cudaMemcpyHostToDevice);
  
  
  struct timeval startwtime, endwtime;
  float seq_time;
  //xronos arxis
  gettimeofday (&startwtime, NULL);
  
  
  //pairnw stathera kai tin vazw sto e
  const char* text = argv[1];
  float e=atof(text); 
  
  
  printf("e=%f \nBefore shiftingFunction\n",e);

  
  //edw ginetai o algorithmos MeanShift
  shiftingFunction<<< 6, 100 >>>(deviceArrayYkplus1,deviceArrayYk,deviceArrayStatic,e);
  cudaDeviceSynchronize();
	
 
	
  //telos metrisis tou xronou 
  gettimeofday (&endwtime, NULL); 
  seq_time = (float)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
  printf("After shiftingFunction\nclock time = %f\n", 1000*seq_time);
  
  cudaMemcpy(arrayYk,deviceArrayYk,nBytes,cudaMemcpyDeviceToHost);
  //apo edw kai pera exw ston arrayYk ta apotelesmata
  
  
    FILE *f = fopen("output.txt", "w");
    if (f == NULL){
      printf("Error opening file!\n");
      exit(1);
    }

    for(i=0;i<points;i++){
      for(j=0;j<dimensions;j++){
        fprintf(f,"%f ",arrayYk[i*dimensions+j]);
      }
      fprintf(f,"\n");
    }


	

    fclose(f);
	
	
	//EDW NA KANW TON ELEGXO ORTHOTITAS TOU ALGORITHMOU
    
	FILE *f2=fopen("eksodos.txt", "r");
	
	
	float myvar2;
	int errors=0;
	
		for(i=0;i<points;i++){
		for(j=0;j<dimensions;j++){
			
			//den diavazei ton xaraktira meta to float eite ine , eite ine \n
			scanfReturn=fscanf(f2,"%f%*c",&myvar2);
			//elegxos an i scanf litourgise swsta
			if(scanfReturn==1){/*ola kala*/}
			else{ printf("error in %d,%d. \n",i,j); return 1;}

			if(fabs(myvar2-arrayYk[i*dimensions+j])>0.5) errors++;
		}
	}
	
	
	printf("The number of errors is %d \n",errors);
	
    

  
  
  return 0;
}