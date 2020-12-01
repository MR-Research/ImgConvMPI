#include <stdio.h>
#include <stdlib.h>

__global__ void kernelConv(double *a, double *b, double *c, int w, int h, int filtsize) {
    int i = threadIdx.x+blockDim.x*blockIdx.x;
    int j = threadIdx.y+blockDim.y*blockIdx.y;
    int k,l, cont = 0, abspos;
    double sum = 0;    
    abspos = i+(j*w); 
    for (l = 0; l < filtsize; l++){              
        for (k = 0; k < filtsize; k++){                              
            sum += b[(abspos+l)+(k*w)] * a[cont];                      
            cont++;
        }
    }    
    c[i+(j*w)] = sum;      
}


float imgConvDevice(double *a, double *b, double *c, int filtsize, int w, int h) {
    double *aD, *bD, *cD; 
    cudaEvent_t start, stop; 
    float t; 
    int wb = (int)floor((double)w/filtsize*1.0), hb = (int)floor((double)h/filtsize*1.0);
    dim3 bloques(wb,hb); 
    dim3 hilos(filtsize, filtsize); 
    printf("Grid (%d,%d)\n", wb-1, hb-1); 
    cudaEventCreate(&start); cudaEventCreate(&stop); 
    //cudaEventRecord(start, 0);
    cudaMalloc(&aD, filtsize*filtsize*sizeof(double *)); 
    cudaMalloc(&bD, w*h*sizeof(double *));
    cudaMalloc(&cD, w*h*sizeof(double *));
    
    cudaMemcpy(aD, a, filtsize*filtsize*sizeof(double *), cudaMemcpyDefault); 
    cudaMemcpy(bD, b, w*h*sizeof(double *), cudaMemcpyDefault);

    cudaEventRecord(start, 0);     
    kernelConv<<<bloques, hilos>>>(aD, bD, cD, w, h, filtsize);
    cudaEventRecord(stop, 0); 

    cudaMemcpy(c, cD, w*h*sizeof(double *), cudaMemcpyDefault);


    cudaFree(aD); cudaFree(bD);
    //cudaEventRecord(stop, 0); 
    cudaEventSynchronize(stop);


    cudaEventElapsedTime(&t, start, stop); 
    cudaEventDestroy(start); cudaEventDestroy(stop); 
    return t; 
}


int main(int argc, char **argv) { // parameters: gpu deivce, image path, image width, image height, filter path, resulting image path, filter size
    
    if (argc < 6){
        printf("Error: number of agrguments incorrect.");
        return 0;
    }
    int d = atoi(argv[1]); 
    const char *imgpath = argv[2];
    const char *filterpath = argv[3];
    const char *imgrespath = argv[4];
    const char *imgw = argv[5];
    const char *imgh = argv[6];
    const char *filtsize = argv[7];      

    FILE *pf, *pimg, *pres;
    int i, j, fs, imw, imh, wc, hc;
    float t;
    double *a;
    double *b;
    double *c;
    double *tmp;  
    
    fs = atoi(filtsize);
    imw = atoi(imgw);
    imh = atoi(imgh);
    wc = imw - (fs-1);
    hc = imh - (fs-1);     
    cudaSetDevice(d%3); 
    cudaHostAlloc(&a, atoi(filtsize)*atoi(filtsize)*sizeof(double *), cudaHostAllocDefault);      
    cudaHostAlloc(&b, atoi(imgw)*atoi(imgh)*sizeof(double *), cudaHostAllocDefault);  
    cudaHostAlloc(&c, atoi(imgw)*atoi(imgh)*sizeof(double *), cudaHostAllocDefault);  
    
    //Reading the filter from a file
    pf = fopen (filterpath, "r");
    if (pf == NULL){
        printf("Error loading filter\n");
        return 0;
    }   
    for(i = 0, tmp = a; i < atoi(filtsize)*atoi(filtsize); ++i){
            fscanf(pf, "%lf", tmp++);
    }
    fclose (pf); 
    //Image loading
    pimg = fopen (imgpath, "r");
    if (pimg == NULL){
        printf("Error loading image\n");
        return 0;
    }   
    for(i = 0, tmp = b; i < atoi(imgw)*atoi(imgh); ++i){
            fscanf(pimg, "%lf", tmp++);
    }
    fclose (pimg);    
    
    t = imgConvDevice(a, b, c, fs, imw, imh); 

    //Image writting
    pres = fopen (imgrespath, "w");
    if (pimg == NULL){
        printf("Error loading file to write\n");
        return 0;
    }   
    tmp = c;
    //for(i = 1, tmp = c; i < atoi(imgw)*atoi(imgh); ++i){            
    for (i = 0; i < hc; i++){    
        for (j = 0; j < wc; j++)        
            if (j == wc-1){
                fprintf(pres, "%lf", *tmp);
                fprintf(pres, "\n");     
                tmp = tmp + fs;   
            } else {
                fprintf(pres, "%lf,", *tmp);
                tmp++;
            }            
    }
    fclose (pres);     
    printf("Run time: %f s\n", t/1000);
    cudaFreeHost(a); 
    cudaFreeHost(b);
    cudaFreeHost(c);
}


