#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
//#include <cuda.h>
//#include <cuda_runtime.h> 
//#include <cuda_runtime_api.h>
//#include <string.h>


/*compile with : gcc seir2.c -o seir2 -lm */
/*run with : ./seir2 1 */


float s = 1;
#define points 600
#define dimensions 2

///////////////FUNCTIONS/////////////////////

//sinartisi pou epistrefei tin apostasi 2 simiwn
float distanceFunction(float *point1,float *point2){

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
float kernelFunction(float *pointYk,float *arrayXi){

  float returnWeight=0;
  float distance=distanceFunction(pointYk,arrayXi);
  distance*=distance;

  returnWeight=exp(-((distance)/(2*(s*s))));
  return returnWeight;
}

//epistrefei tin apostasi pou kineitai to moved
float movedDistance(float *moved){
  float distance;
  int i;
  distance=0;

  for(i=0;i<dimensions;i++){
    distance+=(moved[i])*(moved[i]);
  }

  distance = sqrt(distance);
  return distance;
}
























void shiftFunction(float *Ykplus1,float* Yk,float *X,float e,int index){

  int i,j;

  /*//tipwnei tis 2 diastaseis kathe simiou (elegxos)
  for(i=0;i<dimensions;i++){
    printf("%f ",pointYk[i]);
  }
  printf("\n");
*/


  float arithmitis[dimensions];
  for(i=0;i<dimensions;i++) { arithmitis[i]=0;}
  float paronomastis=0;
  float weightFromGaussian=0;
  float check,S=s*s;

  float Ypoint[2],Xpoint[2],moved[2];
  

  //stin arxi ta vazw megalo noumero wste na ksekinisi sigoura to while()
  moved[0]=9;moved[1]=9;
  
  //oso den ikanopoieitai to kritirio siglisis tote kane shift to simio index
  while(movedDistance(moved)>=e){
	  
	  //auto ine to simio pou kathorizete apo to index
	  Ypoint[0]=Yk[index*2];
      Ypoint[1]=Yk[index*2+1];
	  
	  //ta midenizw ola giati prokeitai gia kainourio shift
	  paronomastis=0;
      weightFromGaussian=0;
	  for(i=0;i<dimensions;i++) { arithmitis[i]=0;}
	  
	//ipologizw gia kathe simio tou X an tha mpei sti sinartisi
	for(i=0;i<points;i++){
		
		//edw mpainoun ola ta simia tou X me tin seira gia na dw an tha mpoun ston ypologismo tou Ykplus1
		Xpoint[0]=X[i*2];
		Xpoint[1]=X[i*2+1];
		
		check=distanceFunction(Ypoint,Xpoint);
		
		//printf("distance(%d)=%f and s^2=%f\n",i,check,S);
		//elegxos  an tha simperilavw to simio Xpoint ston ipologismo tou arithmiti kai paronomasti
		if(check<=S && check>0) 
		{
		//ypologismos tou varous pou exei to simio pou anikei sto bandwidth(check<s^2) kai analoga ananewnw arithmiti kai paronomasti
		weightFromGaussian=kernelFunction(Ypoint,Xpoint);
		for(j=0;j<dimensions;j++){
			arithmitis[j]+= weightFromGaussian*X[i*dimensions+j];
		}
		paronomastis+=weightFromGaussian;
		}
		else {/*does nothing*/ }
	}

	//xrisimopoiw arithmiti kai paronomasti gia na fortwsw ti nea timi tou simiou index ston pinaka Ykplus1
	for(j=0;j<dimensions;j++){
		Ykplus1[index*2+j]=arithmitis[j]/paronomastis;
	}

	//ypologizw to posi apostasi egine shift (pinakas moved) kai vazw to Yk sto Y wste na ksekinisei swsta to epomeno shift
	for(j=0;j<dimensions;j++){
		moved[j]=Ykplus1[index*2+j]-Yk[index*2+j];
		Yk[index*2+j]=Ykplus1[index*2+j];
	}


  }


//edw ftanei mono otan exei ikanopoihthei to kritirio siglisis e




}









int main(int argc, char **argv){

  //anoigma tou arxeiou gia diavasma
  FILE *myFile = fopen("eisodos.txt", "r");
  //elegxos an den anikse
  if(myFile==0){
    fprintf(stderr,"Failed to open eisodos.txt.\n");
    return 1;
  }
  
  
  if(argc!=2){ printf("Parameter needed\nexample: ./seir2 0.5\n"); return 1; }
  
  
  /*an thelw na pernaw ta apotelesmata stin eksodo
  if(freopen("stdout.txt","w",stdout)==0) {
	fprintf(stderr,"Failed to write in stdout.txt.\n");
    return 1;
  }
  */
  
  
  //posa bytes einai oi monodiastatoi pinakes(xrisimopoieitai sto malloc)
  int nBytes=points*dimensions*sizeof(float*);
  
  //edw diavazontai ta dedomena
  float *arrayStatic;
  arrayStatic=(float*)malloc(nBytes);

  //edw tha ginete epanaliptika to MeanShift diladi ine to y(k)
  float *arrayYk;
  arrayYk=(float*)malloc(nBytes);

  //edw tha girnaei to y(k+1)
  float *arrayYkplus1;
  arrayYkplus1=(float*)malloc(nBytes);

  int i,j,scanfReturn;
  float myvar;
  
  //diavasma tou pinaka apo to arxeio eisodos.txt
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
  
  struct timeval startwtime, endwtime;
  float seq_time;

  gettimeofday (&startwtime, NULL);

  
  const char* text = argv[1];
  
  float e=atof(text); //mikri stathera gia na kserw pote stamataei o algorithmos
  
  
  
  
  
  
  
  
  
  for(i=0;i<points;i++){

	  shiftFunction(arrayYkplus1,arrayYk,arrayStatic,e,i);
	  
  }
  
  

	gettimeofday (&endwtime, NULL);
	
	
    seq_time = (float)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);

	//xronos se milliseconds
	printf("wall clock time = %f\n", 1000*seq_time);

    /*kathws vgainei apo edw exw ston arrayYk ta simia sta opoia kateliksan ola ta simia tou arxikou sinolou*/

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
    
    
    FILE* f2=fopen("eksodos.txt", "r");
	
	
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
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  