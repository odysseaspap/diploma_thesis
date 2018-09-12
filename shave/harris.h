
//GL MODIFICATIONS/////////////////////
#include <mv_types.h>
//#define USE_SQUARE_KERNEL_DERIVATIVES
//#define USE_SQUARE_KERNEL_BLUR
#define DO_NOT_USE_BORDER_PIXELS
//#define OUTPUT_THE_CORNERS_NOT_THE_IMAGE

//#define DO_FIXED_POINT_MODEL

#ifdef DO_FIXED_POINT_MODEL
#define HARAUX 16.0F //reduce harris K bits (see HAR1, HAR2). Now: 16 => truncate 4 bits. K'=16*K=0.64
#define HAR1 (1/HARAUX) //harris, for the truncation of derivatives
#define HAR2 fixwidth((0.04F*HARAUX),1,3,'s') // [7 bits: mean error~0.001] [3 bits: mean error~0.003]
#define CORN_INTG_BITS 22
#define CORN_FRAC_BITS -5 //negative values denote the intg LSB to truncate in FPGA (+ all frac bits)
#endif

///////////////////////////////////////

#ifndef	_HARRIS_H_
#define	_HARRIS_H_


#define HARRIS_NCORNERS           120 //1000
#define HARRIS_MAX_CORNERS        240 //20000
#define HARRIS_KAPPA              0.04F
#define HARRIS_SIGMA              1.0F //1.5F //0.7F
#define HARRIS_RELATIVE_MIN       0.035F //0.000005F //0.00001F //<--USE THIS, NOT THE AUX
#define HARRIS_RELATIVE_MIN_AUX   1 //-1e-05F

//Parallelizing Harris Corner Detection to many shaves
#define LINE_PADDING 0 //this must equal max kernel size
#define SHAVE_HARRIS_NCORNERS        10// HARRIS_NCORNERS/ SHAVES_USED    //1000
#define SHAVE_HARRIS_MAX_CORNERS     240 //HARRIS_MAX_CORNERS/ SHAVES_USED   //20000

#ifdef __cplusplus
extern "C" {
#endif

void HarrisCornerDetector(u8* inPlane,  int width, int height, int shaveNumber);
void harris_init(int imw, int imh, int n1, float relminthresh);
void  harris_finish(void *hdata);

//int harris_findCorners(void *hdata, unsigned char *img, float (*corners)[2]);
int harris_findCorners(unsigned char* img);

//int harris_describeCorners(void *hdata, float (*corners)[2], int ncorners, int maxori,
//                           float (**framesp)[VO_FRAMESNP], unsigned char **framedescsp, int *maxfrmsp);

static void harris_drawCorners(u8* img, int width, int height, float (*cornrs)[2], int ncorn);
#ifdef __cplusplus
}
#endif


#endif	// _HARRIS_H_
