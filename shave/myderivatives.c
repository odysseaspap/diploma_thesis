#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mv_types.h>

#include "harris.h"
#include "internal.h"


static int imgradient5_smo(unsigned char *img, int w, int h, int *wrkx, int *wrky, int *gradx, int *grady)
{
//printf("called imgradient5_smo\n");
int i, j;
unsigned char *pimg; // img+idx
int *pwrk; // wrkx+idx or wrky+idx;
int idx, idxw;
//const int w2=w<<1; //2*w;
int w2=w<<1;


/* normalization constants */
float gnorm=1.0F/26.0F; // 1/sum([1 6 12 6 1])
float dnorm=1.0F/10.0F; // 1/sum([-2 -1 0 1 2].*[-1 -3 0 3 1])
//printf("after normalization constants\n");
  /* The following 5-tap kernels are employed below:
   * derivative: [-1 -3 0 3 1]
   * smoothing:   [1 6 12 6 1] == [1 6 2*6 6 1]
   *
   * Note that in order to minimize floating point operations, the kernels
   * have integer coefficients and are not normalized
   */

  /* horizontal convolutions */
  for(i=idxw=0; i<h; ++i, idxw+=w){
    //idxw=i*w;
	  //printf("i is %d and idxw is %d \n",i, idxw);
    for(j=2, idx=idxw+2; j<w-2; ++j, ++idx){
      //idx=i*w+j;
      pimg=img+idx;
      /* derivative of Gaussian */
      wrkx[idx]=(-pimg[-2]+pimg[+2]) + (-pimg[-1]+pimg[+1])*3;
      wrkx[idx]=(int)(wrkx[idx]*dnorm);

      /* Gaussian */
      wrky[idx]=(pimg[-2]+pimg[+2]) + 6*((pimg[-1]+pimg[+1]) + (*pimg<<1));
      wrky[idx]=(int)(wrky[idx]*gnorm);
    }
  }

  /* vertical convolutions */
  for(i=2, idxw=2*w; i<h-2; ++i, idxw+=w){
    //idxw=i*w;
    for(j=0, idx=idxw; j<w; ++j, ++idx){
      //idx=i*w+j;
      /* Gaussian */
      pwrk=wrkx+idx;
      //printf("%c\n", pwrk[i]);

      gradx[idx]=(pwrk[-w2]+pwrk[+w2]) + 6*((pwrk[-w] +pwrk[+w] ) + (*pwrk<<1));
      gradx[idx]=(int)(gradx[idx]*gnorm);
      //printf("la\n");

      /* derivative of Gaussian */
      pwrk=wrky+idx;
      grady[idx]=(-pwrk[-w2]+pwrk[+w2]) + (-pwrk[-w] +pwrk[+w] )*3;
      grady[idx]=(int)(grady[idx]*dnorm);
    }
  }
//printf("end of imgradient5_smo\n");
  return 2;
}

/* compute the derivatives near image edges using forward and backward finite differences */
static void imgradient_bfill(unsigned char *img, int width, int height, int uoff, int voff, int *gradx, int *grady)
{
	printf("called imgradient_bfill\n");
//register int i, j;
//register unsigned char *pimg;
//register int *gx, *gy;

int i, j;
unsigned char *pimg;
int *gx, *gy;



#ifdef DO_NOT_USE_BORDER_PIXELS  //GL mod (FPGA-like)
printf("Not using border pixels\n");
return;
#endif

  /* Note that a neighbor in the next column and row exists for every
   * pixel except those on the very last column and row. Thus, forward
   * differences can be used for the derivatives at these pixels
   * (differences with left and below neighbors for gx and gy, respectively)
   */

  /* first voff rows excluding pixels at last column */
  for(j=0; j<voff; ++j){
    pimg=img+j*width;
    gx=gradx+j*width;
    gy=grady+j*width;
    for(i=0; i<width-1; ++i){
      gx[i]=pimg[i+1]     - pimg[i];
      gy[i]=pimg[i+width] - pimg[i];
    }
  }

  /* left uoff & right uoff-1 columns */
  for(j=voff; j<height-voff; ++j){
    pimg=img+j*width;
    gx=gradx+j*width;
    gy=grady+j*width;
    for(i=0; i<uoff; ++i){
      gx[i]=pimg[i+1]     - pimg[i];
      gy[i]=pimg[i+width] - pimg[i];
    }
    for(i=width-uoff; i<width-1; ++i){
      gx[i]=pimg[i+1]     - pimg[i];
      gy[i]=pimg[i+width] - pimg[i];
    }
  }

  /* voff-1 rows before last excluding last column */
  for(j=height-voff; j<height-1; ++j){
    pimg=img+j*width;
    gx=gradx+j*width;
    gy=grady+j*width;
    for(i=0; i<width-1; ++i){
      gx[i]=pimg[i+1]     - pimg[i];
      gy[i]=pimg[i+width] - pimg[i];
    }
  }

  /* following loops use a combination of fwd and bwd differences */
  /* last column except last pixel */
  for(j=0; j<height-1; ++j){
    pimg=img+j*width;
    gx=gradx+j*width;
    gy=grady+j*width;

    /* bwd difference for gx and fwd for gy */
    gx[width-1]=                      pimg[width-1] - pimg[width-1-1];
    gy[width-1]=pimg[width-1+width] - pimg[width-1];
  }
  /* last row except last pixel */
  pimg=img+(height-1)*width;
  gx=gradx+(height-1)*width;
  gy=grady+(height-1)*width;
  for(i=0; i<width-1; ++i){
    /* fwd difference for gx and bwd for gy */
    gx[i]=pimg[i+1] - pimg[i];
    gy[i]=            pimg[i] - pimg[i-width];
  }

  /* pixel at (width, height), bwd differences */
  pimg=img+height*width-1;
  gx=gradx+height*width-1;
  gy=grady+height*width-1;
  gx[0]=pimg[0] - pimg[-1];
  gy[0]=pimg[0] - pimg[-width];
}

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif /* M_PI */

/* initialize a 1D Gaussian kernel of sigma s.
 * The kernel coeffcients are returned in newly allocated memory
 * whereas the size of the kernel is stored in *kernsz
 *
 * Note that to avoid floating point operations, the computed kernel
 * is unnormalized and approximated by integer coefficients
 */
static int *initGaussian(float s, int *kernsz)
{
int x;
int n, n2;

float Gn, G;

  /* kernel size is ceil(3*s)*2+1, which for e.g. s=1 yields a 7-tap filter. s=0.5 yields a 5-tap filter */
  *kernsz=n2=(int)ceil(3.0*s)*2+1; // result is 7 for s = 1.0F
  static int foo[7] = {0};

  n=n2>>1; // n2/2

  //kern=(int *)malloc(n2*sizeof(int));
 // if(!kern){
   // fprintf(stderr, "memory allocation request failed in initGaussian()\n");
    //exit(1);
  //};

  x=n; Gn=(float)(1.0/(s*sqrt(2.0*M_PI))*expf(-x*x/(2.0F*s*s)));
  for(x=-n; x<=n; ++x){
    G=(float)(1.0/(s*sqrt(2.0*M_PI))*expf(-x*x/(2.0F*s*s)));
    foo[x+n]=(int)(G/Gn + 0.5F); // notice that Gn<=G(x) for all x, hence the ratio is >=1
  }


  return foo;
}


static int imgblurg(int *img, int w, int h, int *kern, int kernsz, int *wrk) //img here is gradx2 or grady2 or gradxy
{
int i, j, k, sum;
int *psrc; // img+idx or wrk+idx
int idxw, idx;
int kernsz2=kernsz>>1; // kernsz/2
float kernscl;

  /* Gaussian kernel is separable and symmetric */
//for (i=0;i<7;++i) printf("%d, ",kern[i]);

  /* symmetry allows us to use only upper half of kernel */
  kern+=kernsz2; // now kern[k]==kern[-k]
  /* compute the scale factor that normalizes the kernel to 1 */
  for(k=kernsz2+1, sum=kern[0]; --k>0;  ) // for(k=1; k<=kernsz2; ++k) reversed
    sum+=(kern[k]<<1); // 2*kern[k]
  kernscl=1.0F/sum;
  //printf("sum is %d and kernscl is %f \n", sum, kernscl);

  /* separability: convolve horizontally ... */
  for(i=0, idxw=0; i<h; ++i){
	idxw=i*w;
    //printf("idxw is %d\n",idxw);
    for(j=kernsz2, idx=idxw+kernsz2; j<w-kernsz2; ++j){
      idx=i*w+j;
      psrc=img+idx;
      //printf("psrc is %d\n", psrc);
      //printf("i is  %d and j is %d and img[%d] is %d and psrc[%d] is %d \n", i, j, j, img[j], j , psrc[j]);
      sum=(*psrc)*kern[0]; //this works fine!
      //if(idx>440)
    	// printf(" i is %d and j %d is and idx is %d and sum is %d\n",i,j, idx, sum);
      for(k=kernsz2+1; --k>0;  ){ // for(k=1; k<=kernsz2; ++k) reversed   //this works fine too
        sum+=(psrc[-k]+psrc[+k])*kern[k];
        //printf("PSRC points now to address %d\n", psrc);
        //if(idx>440)
        	//printf("k is %d and psrc[-k] is %d and psrc[+k] is %d and kern[k] is %d and sum is %d\n", k , psrc[-k], psrc[+k], kern[k], sum);
      }
     wrk[idx]=(int)(sum*kernscl); //this fucks things up!
     //wrk[idx]=1; //test
     // if(idx>440)
   //  printf("wrk[%d] is %d\n", idx, wrk[idx]);

    }
  }
  /* ... then vertically */
  for(i=kernsz2, idxw=kernsz2*w; i<h-kernsz2; ++i, idxw+=w){
    //idxw=i*w;
    for(j=0, idx=idxw; j<w; ++j, ++idx){
      //idx=i*w+j;
      psrc=wrk+idx;
      sum=(*psrc)*kern[0];
      for(k=kernsz2+1; --k>0;  ) // for(k=1; k<=kernsz2; ++k) reversed
        sum+=(psrc[-k*w]+psrc[+k*w])*kern[k];
      img[idx]=(int)(sum*kernscl);
    }
  }

  return kernsz2;
}



