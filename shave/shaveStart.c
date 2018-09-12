

// 1: Includes
// ----------------------------------------------------------------------------
//#include <swcFrameTypes.h>
#include <svuCommonShave.h>
#include "effect.h"
#include "harris.h"
#include "internal.h"
#include <stdlib.h>
#include <stdio.h>
//#include "effect.c"


// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
// 4: Static Local Data
// ----------------------------------------------------------------------------
// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------
//int ncorn;
//void * hp;
//float (*cornrs)[2];

//call functions defined in harris.h and implemented in effect.c
//similar function to main in harris.c
void start(frameBuffer* inFrame,  int shave_number) //each shave takes a part of the image, with dimensions img_width* (img_height/shaves_used)
{

	//printf("OK %d \n", shave_number);
	//Brightness(inFrame->p1, outFrame->p1, inFrame->spec.width, inFrame->spec.height);
    //CopyPlane(inFrame->p2, outFrame->p2, inFrame->spec.width / 2, inFrame->spec.height / 2);
    //CopyPlane(inFrame->p3, outFrame->p3, inFrame->spec.width / 2, inFrame->spec.height / 2);

	HarrisCornerDetector(inFrame->p1, inFrame->spec.width, inFrame->spec.height, shave_number); //dma transaction and processing LINE - BY - LINE

	//harris_drawCorners(inFrame->p1, inFrame->spec.width, inFrame->spec.height, cornrs, ncorn); //updates the img buffer, which is the inFrame->p1


    SHAVE_HALT;

    return;
}
