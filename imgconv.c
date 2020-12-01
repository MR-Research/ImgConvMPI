/*    Image convolution algorithm implemented using MPI
      Written by David Cantu
      November 2020
      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.
 
      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
 
      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stb_image.h"
#include "stb_image_write.h"

#define MASTER 0               
#define FROM_MASTER 1          
#define FROM_WORKER 2          

int main (int argc, char *argv[])
{
   int	numtasks,         
	taskid,                
	numworkers,            
	source,                
	dest,                  
	mtype,                 
	rows,                  
	averow, extra, offset, 
	i, j, k, rc, l, m;
   MPI_Status status;
   FILE *pf;

	//Time and date of the calculation
   time_t now;
   struct tm *local;
   int hours, minutes, seconds, day, month, year;

   //Obtain current time for testbench reports
   time(&now);
   local = localtime(&now);
	hours = local->tm_hour;	  	
	minutes = local->tm_min;	 	
	seconds = local->tm_sec;	 	

	day = local->tm_mday;			
	month = local->tm_mon + 1;   	
	year = local->tm_year + 1900;	  
	
	int w, h, bpp, cc = 3, wc, hc, crop_img_size;
	double start, end, *tmp, *a, sum;
   unsigned char *image, *gray_img, *new_img_gray, *res_img_gray;
   size_t img_size, gray_img_size;
   int gray_channels = 1, filts, offs;   

	char *imgpath = argv[1];
	char *filterpath = argv[2];
	char *imgrespath = argv[3];
	char *filtsize = argv[4];
   filts = atoi(filtsize);
   offs = filts;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
   MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
   if (numtasks < 2 ) {
      printf("Need at least two MPI workers.\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      exit(1);
   }
   numworkers = numtasks-1; 


   // Master task
   if (taskid == MASTER)
   {
      if (argc < 5){
         printf("Incorrect number of arguments\n");
         exit(1);
      }      
      //printf("-----Image convolution using MPI-------\n");
      //printf("Number of workers (excluding master): %d\n", numworkers);
      //printf("Image: %s, filter: %s, result: %s, filter size: %s\n", imgpath, filterpath, imgrespath, filtsize);      
      //printf("MPI started with %d tasks.\n", numtasks);  

      //Image loading
      //printf("Worker %d: Image loading\n", taskid);
      image = stbi_load(imgpath, &w, &h, &bpp, cc);   
                    
      img_size = w * h * cc;
      gray_img_size = w * h;
      wc = w - (filts-1);
      hc = h - (filts-1);     
      crop_img_size = wc* hc;
      //Memory allocation
      a = malloc(filts*filts*sizeof(double *));
      gray_img = malloc(gray_img_size);
      new_img_gray = malloc(gray_img_size);
      res_img_gray = malloc(crop_img_size);      

      //Reading the filter from a file
      //printf("Worker %d: Reading the filter from a file\n", taskid);
      pf = fopen (filterpath, "r");
      if (pf == NULL){
         return 0;
      }      
      for(i = 0, tmp = a; i < atoi(filtsize)*atoi(filtsize); ++i){
               fscanf(pf, "%lf", tmp++);
      }         
      fclose (pf);     
      //Grayscale conversion
      //printf("Worker %d: Grayscale conversion\n", taskid);
      for(i = 0; i < gray_img_size; i++) {
         unsigned char *pt = image+(i*3);
         unsigned char *pgt = gray_img+i;            
         *pgt = (uint8_t)((*pt + *(pt + 1) + *(pt + 2))/3.0);
      }

      // Send data to workers
      //printf("Worker %d: Send data to the worker tasks\n", taskid);
      averow = h/numworkers;
      extra = h%numworkers;
      offset = 0;
      mtype = FROM_MASTER;

      /* Measure start time */
      double start = MPI_Wtime();            
      for (dest=1; dest<=numworkers; dest++)
      {
         rows = (dest <= extra) ? averow+1 : averow;  	
         //printf("Worker %d: Sending %d rows to task %d offset=%d\n", taskid, rows, dest, offset);
         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&w, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(&h, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
         MPI_Send(gray_img+(offset*w), (rows+offs)*w, MPI_UNSIGNED_CHAR, dest, mtype,
                   MPI_COMM_WORLD);
         MPI_Send(a, filts*filts, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
         offset = offset + rows;
      }

      // Receive results
      mtype = FROM_WORKER;
      for (i=1; i<=numworkers; i++)
      {
         source = i;
         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(new_img_gray+(offset*w), rows*w, MPI_DOUBLE, source, mtype, 
                  MPI_COMM_WORLD, &status);
         //printf("Worker %d, Received results from task %d\n", taskid, source);
         //printf("Worker %d, offset: %d, rows: %d, received size: %d\n", taskid, offset, rows, offset*w);
      }

      // Measure finish time
      double finish = MPI_Wtime();
      //Write resulting image   
      for (i = 0; i < hc; i++){
         for (j = 0; j < wc; j++){
            //if (i < hc && j < wc){
               *(res_img_gray+(i*wc)+j) = *(new_img_gray+(i*w)+j);
            //}
         }
      }
	   stbi_write_jpg(imgrespath, wc, hc, 1, res_img_gray, wc);
      //Free memory
	   stbi_image_free(image);    
	   free(gray_img);
      free(res_img_gray);
	   free(new_img_gray); 
      free(a);
      //printf("Done in %f seconds.\n", finish - start);
	   //Print to stdout with csv format, for testbench
	   printf("%02d-%02d-%d_%02d:%02d,%s,%s,%s,%d,%f\n", day, month, year, minutes, seconds, imgpath, filterpath, filtsize, numtasks, finish - start);      
   }


   // Workers  
   if (taskid > MASTER)
   {
      //printf("Worker task %d\n", taskid);
      mtype = FROM_MASTER;
      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      //printf("Worker %d, offset: %d\n", taskid, offset);
      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      //printf("Worker %d, rows: %d\n", taskid, rows);

      MPI_Recv(&w, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      MPI_Recv(&h, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
      //printf("Worker %d, image dimensions received: %d x %d\n", taskid, w, h);
      //Allocate memory for convolution and grayscale image
      a = malloc(filts*filts*sizeof(double *));   
      gray_img_size = (rows+offs)*w;
      gray_img = malloc(gray_img_size);
      new_img_gray = calloc(rows,w); 

      //printf("Worker %d, finished allocating memory\n", taskid);
      MPI_Recv(gray_img, (rows+offs)*w, MPI_UNSIGNED_CHAR, MASTER, mtype, MPI_COMM_WORLD, &status);
      //printf("Received Grayscale image portion, size: %d\n", rows*w);
      MPI_Recv(a, filts*filts, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
      //printf("Received convolution matrix, size: %d\n", filts*filts);
      //printf("offset(%d) x rows(%d): %d\n", offset, rows, offset*rows);
      for (k=0; k<rows; k++){
         for (i=0; i<w; i++)
         {
            if (i+filts <= w){
               sum = 0;
               for (l = 0; l < filts; l++){
                  for (m = 0; m < filts; m++){
                     sum += *(gray_img+(k*w)+i+(w*(l))+m) * *(a+(l*filts)+m);                                    
                     //*(new_img_gray+(k*w)+i) = *(gray_img+(k*w)+i+(w*(l))+m);
                  }
               }
               *(new_img_gray+(k*w)+i) = (uint8_t)sum;               
               //*(new_img_gray+(k*w)+i) = 0;
            }
         }
      }

      mtype = FROM_WORKER;
      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
      MPI_Send(new_img_gray, rows*w, MPI_UNSIGNED_CHAR, MASTER, mtype, MPI_COMM_WORLD); 
   }
   MPI_Finalize();  
}
