

// 1: Includes
// ----------------------------------------------------------------------------
#include <mv_types.h>
#include <svuCommonShave.h>
//#include "effect.h"
#include "swcCdma.h"

//My includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


#include "harris.h"
#include "internal.h"
#include "myderivatives.c"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define MAX_LINE_WIDTH 512
#define MAX_LINE_HEIGHT 32 //image_height/shaves_used
//define width and height here as well and change functions interface!!!

#define CMX_FRAMES __attribute__((section(".cmx.data")))
#define DDR_FRAMES __attribute__((section(".ddr_direct.bss")))

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
// 4: Static Local Data
// ----------------------------------------------------------------------------
//u8 bufferedInLine[MAX_LINE_WIDTH]; //TO-DO : harris.c needs the whole image for the harris_init, not just one line
//u8 bufferedOutLine[MAX_LINE_WIDTH];

static harris_data hd; //change hd to statically allocated memory, instead of dynamically
float cornrs[SHAVE_HARRIS_NCORNERS][2] = {{0.0}};
static int  DDR_FRAMES ibuf[5*MAX_LINE_WIDTH*(MAX_LINE_HEIGHT+ LINE_PADDING)]; //5*npixels = 5*W*(SHAVE_HEIGHT+PADDING)
static float DDR_FRAMES fbuf[MAX_LINE_WIDTH*(MAX_LINE_HEIGHT+ LINE_PADDING) + SHAVE_HARRIS_MAX_CORNERS];
//static float cornerness[4800]; //npixels+HARRIS_MAX_CORNERS
//static float selcornerness[40];

int ncorn = 0;


u8 bufferedInFrame[MAX_LINE_WIDTH * (MAX_LINE_HEIGHT + LINE_PADDING)]; //now shave takes a frame of width*shave_height dimensions
//u8 bufferedOutFrame[MAX_LINE_WIDTH*(MAX_LINE_HEIGHT )];

dmaTransactionList_t __attribute__((section(".cmx.cdmaDescriptors"))) task1, task2;

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------
static float kth_smallest(float a[], int n, int k)
{
    int i,j,l,m;
    float x, tmp;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k];
        i=l;
        j=m;
        do {
            while (a[i]<x) i++;
            while (x<a[j]) j--;
            if (i<=j) {
                /* swap a[i], a[j] */
                tmp=a[i];
                a[i]=a[j];
                a[j]=tmp;
                i++; j--;
            }
        } while (i<=j);
        if (j<k) l=i;
        if (k<i) m=j;
    }
    return a[k];
}

/* Note: relminthresh is taken into account only when NEGATIVE! */
void harris_init(int imw, int imh, int n1, float relminthresh)
{

int npixels;


  /* allocate and fill in convolution mask */
  hd.Gmask=initGaussian(HARRIS_SIGMA, &(hd.Gmasksz));

  hd.width=imw;
  hd.height=imh; //imh has line_paddig added when passed as argument here
  hd.n1=n1;
  hd.relminthr=(relminthresh<0.0F)? -relminthresh : HARRIS_RELATIVE_MIN; //threshold for Harris Output Scores
  /* setup work arrays */
  npixels=imw*imh;
  hd.ibuf = ibuf;

  hd.gradx=hd.ibuf;
  hd.grady=hd.gradx+npixels;
  hd.gradx2=hd.grady+npixels;
  hd.grady2=hd.gradx2+npixels;
  hd.gradxy=hd.grady2+npixels;

   hd.fbuf = fbuf;
   hd.cornerness=fbuf;
   hd.selcornerness=hd.cornerness+npixels;


  return ;

}



int harris_findCorners(unsigned char *img)
{

int	x, xx, u0, v0;
int		width, height;
float	max_r, r_thd;
int		ncorners, nmaxima;
int		offwidth, offheight;
int		uoff, voff; //offsets used in the window that parses the frame, in two dimensions
float *row_2, *row_1, *row0, *row1, *row2;


#ifdef HARRIS_USE_TILLING
int twidth, ntiles_u, ntiles_v, ideal_num_per_tile, pass, toffset, tile_v,
    tstart_v, tend_v, tile_u, tstart_u, tend_u, num_per_tile;
float tarrea, this_tarrea, tstrengths[HARRIS_TILE_WIDTH2];
#endif /* HARRIS_USE_TILLING */

/* variables used for subpixel refinement */
//odys:this feature will be turned off
float spp, spc, spn, scp, scc, scn, snp, snc, snn;
float Pxx, Pxy, Pyy, Px, Py, ucorr, vcorr, detf;

/* pointers bellow point to memory allocated by init_harris() */
int    *gradx, *grady, *gradx2, *grady2, *gradxy, *gmask, gmasksz;
float  *cornerness, *selcornerness;

   row_2=row2=NULL; // suppress compiler warnings

   /* init pointers to global data */
   gradx=hd.gradx; grady=hd.grady;
   gradx2=hd.gradx2; grady2=hd.grady2; gradxy=hd.gradxy;
   hd.img=img;
   //printf("gradx address is %d\n", gradx);
   //printf("grady2 address is %d\n", grady2);


   cornerness=hd.cornerness;
   gmask=hd.Gmask;
   gmasksz=hd.Gmasksz;
   selcornerness=hd.selcornerness;

   width=hd.width; height=hd.height; // retrieve img dimensions



   /* compute 1st order intensity derivatives using gradx2, grady2 as working memory */
   voff=uoff=imgradient5_smo(img, width, height, gradx2, grady2, gradx, grady); // 5-tap smoothed

   //imgradient_bfill(img, width, height, uoff, voff, gradx, grady);              
   // complete gradient at borders
   offwidth=width - uoff;
   offheight=height - voff;

   /* compute squared derivatives. Note that uoff, vof, offwidth, offheight
    * are ignored in the loop below
    */
   for(x=width*height; x-->0;  ){
     int gx, gy;

    gx=gradx[x];
    gy=grady[x];
    gradx2[x]=gx*gx;
    grady2[x]=gy*gy;
    gradxy[x]=gx*gy;
   }


   /* smooth squared gradients with the Gaussian using GRADX as working memory.
    * Note that this assumes that sizeof(float)>sizeof(int)
    */
   x=imgblurg(gradx2, width, height, gmask, gmasksz, gradx); //(int *) cornerness
     imgblurg(grady2, width, height, gmask, gmasksz, gradx);
     imgblurg(gradxy, width, height, gmask, gmasksz, gradx);

    uoff+=x;
    voff+=x;
    offwidth-=x;
    offheight-=x;

    //keep corn and selcorn as they were. Define and use this from now on and
    static float mycornerness[MAX_LINE_WIDTH*(MAX_LINE_HEIGHT + LINE_PADDING)]; 


   /* compute the ``cornerness'' of each pixel */
   for(v0=voff, xx=voff*width, max_r=0.0F; v0<offheight; ++v0, xx+=width){
      for(u0=uoff, x=xx+uoff; u0<offwidth; ++u0, ++x){
        int gxx, gyy, gxy;
        int det, trace;
        float r;

        /* compute the elements of the second moment matrix */
        //x=v0*width + u0;
        gxx=gradx2[x];
        gyy=grady2[x];
        gxy=gradxy[x];

        det=gxx*gyy - gxy*gxy;
        trace=gxx + gyy;

        r=det - HARRIS_KAPPA*trace*trace; //original Harris
        if (r>max_r) max_r=r;
        else if(r<0.0F) r=0.0F;

        mycornerness[x]=r;
      }
   }
   /***/
   /* NOTICE NOTICE NOTICE NOTICE: gradxy is used as working memory bellow */

   /* pixels with high cornerness which is locally maximal are marked in gradxy which is used as working memory */
#if 0 // 5x5
   uoff+=2;
   voff+=2;
   offwidth-=2;
   offheight-=2;
#else // 3x3
   uoff+=1;
   voff+=1;
   offwidth-=1;
   offheight-=1;
#endif

   r_thd=max_r*(hd.relminthr);
   nmaxima=0;
   memset(gradxy, 0, width*height*sizeof(int)); // clear gradxy
   for(v0=voff, xx=voff*width; v0<offheight; ++v0, xx+=width){
      for(u0=uoff, x=xx+uoff; u0<offwidth; ++u0, ++x){
        float	d;

	d=mycornerness[x];
	if(d<r_thd) continue; // not strong enough

        /* non-maximum suppression: check whether this is a local maximum in a MXM window */
        row0=mycornerness + x; // start of the current row
        row_1=row0-width;
        row1 =row0+width;

#if 0 // 5x5 window
        row_2=row_1-width;
        row2 =row1 +width;

        if(d> row_2[-2] && d>row_2[-1] && d>row_2[0] && d>row_2[1] && d>row_2[2] &&
           d>=row_1[-2] && d>row_1[-1] && d>row_1[0] && d>row_1[1] && d>row_1[2] &&
           d>= row0[-2] && d>=row0[-1] &&               d> row0[1] && d> row0[2] &&
           d>= row1[-2] && d>=row1[-1] && d>=row1[0] && d> row1[1] && d> row1[2] &&
           d>= row2[-2] && d>=row2[-1] && d>=row2[0] && d>=row2[1] && d>=row2[2])
#else // 3x3 window
        if(d>row_1[-1] && d>row_1[0] && d>row_1[1] &&
           d>=row0[-1] &&               d> row0[1] &&
           d>=row1[-1] && d>=row1[0] && d>=row1[1])
#endif
        {
              if(nmaxima>=SHAVE_HARRIS_MAX_CORNERS){
            	  printf("Too many corners in harris_findCorners, increase \"SHAVE_HARRIS_MAX_CORNERS\" [%d]\n", SHAVE_HARRIS_MAX_CORNERS);

        }
#ifndef HARRIS_USE_TILLING
              selcornerness[nmaxima]=mycornerness[x];
#endif
              ++nmaxima;
              gradxy[x]=1;
        }
      }
    }

//printf("Detected %d corners having strength above relative min threshold\n", nmaxima);

/*------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------
------------------------------------------------------------------------------------*/

#if 0
//  printf("Found %d corners above threshold (max_r=%4.2lf rel=%0.4lf thd=%4.2lf)\n",nmaxima,max_r,hd->relminthr,r_thd);
  if(nmaxima>SHAVE_HARRIS_NCORNERS)
    harris_anms(gradxy, mycornerness, nmaxima, SHAVE_HARRIS_NCORNERS, width, height, uoff, voff);

#else
#ifndef HARRIS_USE_TILLING
  /* find the smallest cornerness value among the n1 strongest local maxima detected above */
  if(nmaxima>hd.n1)
    r_thd=kth_smallest(selcornerness, nmaxima, nmaxima-hd.n1-1);
  else // not enough maxima, accept all
    r_thd=0.0;

   /* pass1: mark in gradxy the locations of the n1 strongest corners */
   for(v0=voff, xx=voff*width, ncorners=0; v0<offheight; ++v0, xx+=width){
      for(u0=uoff, x=xx+uoff; u0<offwidth; ++u0, ++x){
        if(gradxy[x])
          if(mycornerness[x]>=r_thd)
            ++ncorners;
          else
            gradxy[x]=0;
      	  }
   }
  //printf("NCORNERS IS %d\n", ncorners);
#endif /* HARRIS_USE_TILLING */
#endif /* 0 */


    /* pass2: scan gradxy for remaining marked corners and store them after subpixel approximation */
   for(v0=voff, xx=voff*width, ncorners=0; v0<offheight; ++v0, xx+=width){
      for(u0=uoff, x=xx+uoff; u0<offwidth; ++u0, ++x){
        if(gradxy[x]){

          cornrs[ncorners  ][0]=u0;
          cornrs[ncorners++][1]=v0;


          if(ncorners==hd.n1) goto break2; //done when found all shave corners OR when shave slice is over (offheight, width)
        }
      }
   }

break2:
   return ncorners;
}

#define CORNER_BLOB_HLF_SZ 1

static void harris_drawCorners(u8 *img, int width, int height, float (*cornrs)[2], int ncorn)
{
  int i, j, k;
  int x, y;

  //printf("%d\n", ncorn);
  for(k=0; k<ncorn; ++k){
    x=(int)cornrs[k][0]; y=(int)cornrs[k][1];
    if (x<0 || x>width || y<0 || y>height) printf("weird point ");
    else{
    for(i=-CORNER_BLOB_HLF_SZ; i<=CORNER_BLOB_HLF_SZ; ++i)
      for(j=-CORNER_BLOB_HLF_SZ; j<=CORNER_BLOB_HLF_SZ; ++j)
        img[(y+j)*width+x+i]=255;
    img[y*width+x]=0;
    }
    //printf("%g %g\n", cornrs[k][0], cornrs[k][1]);
  }
}


void HarrisCornerDetector(u8* inPlane, int width, int shave_height, int shaveNumber)
{
	//printf("Got into HarrisCornerDetector \n");
	u8* inAddress;
    u8* outAddress;
    dmaTransactionList_t *ref1, *ref2; 
    int i;
    u32 id1 = dmaInitRequester(1);



   // int  lineIndex = shaveNumber * (shave_height);// 1st shave takes 0 to 10, 2nd takes 10 to 20...etc
    //printf("INDEX is %d\n", lineIndex);


   int upper_offset = 0, lower_offset = 0;
   if (shaveNumber == 0)
	   lower_offset = LINE_PADDING; //first shave reads 1st image slice and LINE_PADDING pixels from the next slice
   else if (shaveNumber == 11)
	   upper_offset = LINE_PADDING; //last shave reads last image slice and LINE_PADDING pixels from the previous slice
   else{
	   lower_offset =  LINE_PADDING;
	  // upper_offset = LINE_PADDING;
   }


    inAddress = (u8*) ((u32) inPlane + width*(shave_height - upper_offset)*shaveNumber);
   // outAddress = (u8*) ((u32) outPlane + lineIndex*width); FOR NOW JUST PRINT CORNERS TO STDOUT




    ref1 = dmaCreateTransaction(id1, &task1, inAddress, &bufferedInFrame[0], width*(shave_height + upper_offset + lower_offset)); //bring width*shave_height, not just one line
    dmaStartListTask(ref1);
    dmaWaitTask(ref1);

    //call harris_init to initialize the global variable hd
    harris_init(width, shave_height + LINE_PADDING,  SHAVE_HARRIS_NCORNERS, HARRIS_RELATIVE_MIN_AUX); //the last two parameters are defined in harris.h. Does NOT change image buffer
    
    ncorn = harris_findCorners(bufferedInFrame); //1st parameter is the result of harris_init, mainly the ibuf and fbuf ; Does NOT change image buffer, uses ibuf and fbuf
   

   return;
}
