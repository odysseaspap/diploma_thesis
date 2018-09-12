
// 1: Includes
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "leon_internal.h"
//#include <swcFrameTypes.h>
#include <OsDrvTimer.h>
#include <OsDrvSvu.h>
#include "rtems_config.h"
#include "DrvTempSensor.h"
#include "brdMv0198.h"
#include "brdMv0182.h"
#include "Board182Api.h" //auto!




// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------
#define FRAME_WIDTH    512
#define FRAME_HEIGHT   384
#define SHAVES_USED     12
#define SHAVE_HEIGHT 	32
#define SHAVE_HARRIS_NCORNERS        10// HARRIS_NCORNERS/ SHAVES_USED    //1000


#define CMX_FRAMES __attribute__((section(".cmx.data")))
#define DDR_FRAMES __attribute__((section(".ddr.data"))) //.ddr_direct.bss ?

//Power Measurement
typedef struct
{
    tyAdcResultAllRails power;
    fp32 avgmW;
    fp32 avgmA;
} Prof;

static tyBrd198Handle powerMonHandle; //auto!!
static I2CM_Device* i2c; //auto!!
static Prof test; //auto!!?

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
extern u8  inputFrame; //points to the raw image data that is embedded in the sparc elf file
extern u32 brthYASM0_start; //shave entry points
extern u32 brthYASM1_start;
extern u32 brthYASM2_start;
extern u32 brthYASM3_start;
extern u32 brthYASM4_start;
extern u32 brthYASM5_start;
extern u32 brthYASM6_start;
extern u32 brthYASM7_start;
extern u32 brthYASM8_start;
extern u32 brthYASM9_start;
extern u32 brthYASM10_start;
extern u32 brthYASM11_start;


extern int brthYASM0_ncorn; //each shave has its own ncorn and cornrs[SHAVE_HARRIS_NCORNERS][2] variables
extern float brthYASM0_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM1_ncorn;
extern float brthYASM1_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM2_ncorn;
extern float brthYASM2_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM3_ncorn;
extern float brthYASM3_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM4_ncorn;
extern float brthYASM4_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM5_ncorn;
extern float brthYASM5_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM6_ncorn;
extern float brthYASM6_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM7_ncorn;
extern float brthYASM7_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM8_ncorn;
extern float brthYASM8_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM9_ncorn;
extern float brthYASM9_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM10_ncorn;
extern float brthYASM10_cornrs[SHAVE_HARRIS_NCORNERS][2];

extern int brthYASM11_ncorn;
extern float brthYASM11_cornrs[SHAVE_HARRIS_NCORNERS][2];


//extern int ncorn; //corners found by shaves, will be printed by leon. shave var accessed by leon.

//u32 entryPoints[SHAVES_USED] = {
  //      (u32)&brthYASM0_start,
	//	(u32)&brthYASM1_start,
		//(u32)&brthYASM2_start,
//		(u32)&brthYASM3_start
//};

u32 entryPoints[SHAVES_USED] = {
        (u32)&brthYASM0_start,
        (u32)&brthYASM1_start,
        (u32)&brthYASM2_start,
		(u32)&brthYASM3_start,
		(u32)&brthYASM4_start,
		(u32)&brthYASM5_start,
		(u32)&brthYASM6_start,
		(u32)&brthYASM7_start,
		(u32)&brthYASM8_start,
		(u32)&brthYASM9_start,
		(u32)&brthYASM10_start,
		(u32)&brthYASM11_start
};

// 4: Static Local Data
// ----------------------------------------------------------------------------

static swcShaveUnit_t EffectShaves[SHAVES_USED] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; //{0, 1, 2};
static osDrvSvuHandler_t handler[12];
static tyTimeStamp  g_ticks;
static u64 passed_cycles;
static u64 time_elapsed; //u64

//static u8 outputFrame[FRAME_WIDTH * FRAME_HEIGHT]; //CMX!!!
//outputFrame is too big for CMX ?

static frameBuffer CMX_FRAMES inBuffer[SHAVES_USED], outBuffer[SHAVES_USED]; //CMX!!!


// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
static void showPower(fp32 temp); //auto!
static void initPowerMeasurement(I2CM_Device* i2cBus); //auto!!




// 6: Functions Implementation
// ----------------------------------------------------------------------------
static void showPower(fp32 temp) //auto!!!
{
    s32 k;

    fp32 avgmW = 0;
    fp32 avgmA = 0;
    fp32 avgDdrMw = 0;
    fp32 avgDdrMa = 0;
    fp32 coreMw;
    fp32 coreMa;

    // Compute all averages across all samples
    fp32 ddrMw;
    fp32 ddrMa;

    avgmW += test.power.totalMilliWatts;
    avgmA += test.power.totalMilliAmps;

    Brd198GetDdrPowerAndCurrent(&powerMonHandle, &test.power, &ddrMw, &ddrMa);

    avgDdrMw += ddrMw;
    avgDdrMa += ddrMa;

    test.avgmW = avgmW ;
    test.avgmA = avgmA ;

    coreMw = test.avgmW - avgDdrMw;
    coreMa = test.avgmA - avgDdrMa;

    printf("Core mW = %7.03f, DDR mW = %7.03f, Temp=%2.1fC\n", coreMw, avgDdrMw, temp);
}

static void initPowerMeasurement(I2CM_Device* i2cBus) //auto!!
{
    s32 retVal;

    i2c = i2cBus;
    if (i2c == NULL)
    {
        printf("I2C not initialized for MV0198\n");
    }

    retVal = Brd198Init(&powerMonHandle, i2c, NULL);
    if (retVal != DRV_BRD198_DRV_SUCCESS)
    {
        printf("Board 198 init error\n");
        exit(-1);
    }
}

void reduceBaselinePower()
{
    s32 i;

    // Disable all the SHAVE and USB ISLANDS
    for (i=0;i<12;i++)
        DrvCprPowerTurnOffIsland(POWER_ISLAND_SHAVE_0 + i);
    DrvCprPowerTurnOffIsland(POWER_ISLAND_USB);

    u32 upaClocksToTurnOff = ~(0
            | DEV_UPA_CDMA
            | DEV_UPA_CTRL
            | (1 << UPA_MTX)
            );
    u32 auxClocksToTurnOff = ~(0
            | AUX_CLK_MASK_DDR_REF
            | AUX_CLK_MASK_DDR_CORE_CTRL
            | AUX_CLK_MASK_DDR_CORE_PHY
            | AUX_CLK_MASK_UART
            | (1<< CSS_AUX_TSENS)
            );
    u32 mssClocksToTurnOff = 0
            | DEV_MSS_AXI_BRIDGE
            | DEV_MSS_MXI_DEFSLV
            | DEV_MSS_AXI_MON
            | DEV_MSS_NAL
            | DEV_MSS_CIF0
            | DEV_MSS_CIF1
            | DEV_MSS_LCD
            ;
    u64 cssClocksToTurnOff = ~(0
          | DEV_CSS_AON
          | DEV_CSS_DSS_BUS
          | DEV_CSS_DSS_BUS_AAXI
          | DEV_CSS_LAHB2SHB
          | DEV_CSS_SAHB2MAHB
          | DEV_CSS_LAHB_CTRL
          | DEV_CSS_APB4_CTRL
          | DEV_CSS_MAHB_CTRL
          | DEV_CSS_APB1_CTRL
          | DEV_CSS_APB3_CTRL
          | DEV_CSS_SAHB_CTRL
          | DEV_CSS_UART
          | DEV_CSS_JTAG
          | DEV_CSS_LOS_L2C
          | DEV_CSS_MSS_MAS
          | DEV_CSS_UPA_MAS
          | DEV_CSS_I2C2
          | DEV_CSS_GPIO
          );

    // Turn off unneeded clocks
    DrvCprSysDeviceAction(UPA_DOMAIN,     DISABLE_CLKS, upaClocksToTurnOff);
    DrvCprSysDeviceAction(UPA_DOMAIN,     ASSERT_RESET, upaClocksToTurnOff);
    DrvCprSysDeviceAction(MSS_DOMAIN,     DISABLE_CLKS, mssClocksToTurnOff);
    DrvCprSysDeviceAction(MSS_DOMAIN,     ASSERT_RESET, mssClocksToTurnOff);
    DrvCprSysDeviceAction(CSS_AUX_DOMAIN, DISABLE_CLKS, auxClocksToTurnOff);
    DrvCprSysDeviceAction(CSS_AUX_DOMAIN, ASSERT_RESET, auxClocksToTurnOff);
    DrvCprSysDeviceAction(CSS_DOMAIN,     DISABLE_CLKS, cssClocksToTurnOff);
}



void InitTestBuffers()
{
    int i;
    for (i = 0; i < SHAVES_USED; i++)
    {
        u32 sh_width;
        u32 sh_height;

        sh_width = FRAME_WIDTH;
        sh_height = FRAME_HEIGHT / SHAVES_USED;

        inBuffer[i].spec.width = sh_width;
        inBuffer[i].spec.height = sh_height;
        inBuffer[i].spec.stride = sh_width;
        inBuffer[i].spec.type = RAW8;
        inBuffer[i].spec.bytesPP = 1;




        //inBuffer[i].p1 = (u8*)((u32)&inputFrame + sh_width * sh_height * i);
        inBuffer[i].p1 = (u8*)((u32)&inputFrame);

        //inBuffer[i].p2 = (u8*)((u32)&inputFrame + FRAME_WIDTH * FRAME_HEIGHT + (sh_width * sh_height / 4) * i);
        //inBuffer[i].p3 = (u8*)((u32)&inputFrame + FRAME_WIDTH * FRAME_HEIGHT + FRAME_WIDTH * FRAME_HEIGHT / 4 + (sh_width * sh_height / 4) * i);

        outBuffer[i].spec.width = sh_width;
        outBuffer[i].spec.height = sh_height;
        outBuffer[i].spec.stride = sh_width;
        outBuffer[i].spec.type = RAW8;
        outBuffer[i].spec.bytesPP = 1;

        //outBuffer[i].p1 = (u8*)((u32)&outputFrame[0] + sh_width * sh_height * i);
        //outBuffer[i].p2 = (u8*)((u32)&outputFrame[0] + FRAME_WIDTH * FRAME_HEIGHT + (sh_width * sh_height / 4) * i);
        //outBuffer[i].p3 = (u8*)((u32)&outputFrame[0] + FRAME_WIDTH * FRAME_HEIGHT + FRAME_WIDTH * FRAME_HEIGHT / 4 + (sh_width * sh_height / 4) * i);
    }

    return;
}


void POSIX_Init (void *args)
{
    UNUSED(args);

    int i;
    u32 running;
    s32 sc;
    fp32 avgFps;
    s32 boardStatus; //auto!
    fp32 temperature; //auto!


    sc = initClocksAndMemory();
    if(sc)
        exit(sc);

    printf ("RTEMS POSIX Started\n");  /* initialise variables */

    DrvTempSensorInit(); //auto!!

    boardStatus = BoardInitialise(0);

    if (boardStatus != B_SUCCESS)
    {
        printf("Error: board initialization failed with %d status\n", boardStatus);
        exit(-1);
    }

    InitTestBuffers();

    sc = OsDrvSvuInit();
    if(sc)
        exit(sc);
    sc = OsDrvTimerInit();
    if(sc)
        exit(sc);
    initPowerMeasurement(gAppDevHndls.i2c2Handle); //auto!
    sc = OsDrvTimerStartTicksCount(&g_ticks);
    if(sc)
        exit(sc);

    for (i = 0; i < SHAVES_USED; i++)
    {
        sc = OsDrvSvuOpenShave(&handler[i], EffectShaves[i], OS_MYR_PROTECTION_SEM);
        if (sc == OS_MYR_DRV_SUCCESS)
        {
            sc = OsDrvSvuResetShave(&handler[i]);
            if(sc)
                exit(sc);
            sc = OsDrvSvuSetAbsoluteDefaultStack(&handler[i]);
            if(sc)
                exit(sc);
            //sc = OsDrvSvuStartShaveCC(&handler[i], entryPoints[i], "iii", (u32)&inBuffer[i], (u32)&outBuffer[i], i); //shaves will write and return these buffers
            sc = OsDrvSvuStartShaveCC(&handler[i], entryPoints[i], "ii", (u32)&inBuffer[i],  i); //shaves will write and return these buffers

            if(sc)
                exit(sc);
           // printf("Leon kicked shave %d\n", i);
         
        }
        else {
            printf("cannot open shave %d\n", i);
            exit(sc);
        }
    }

    sc = OsDrvSvuWaitShaves(SHAVES_USED, handler, OS_DRV_SVU_WAIT_FOREVER, &running);
    if(sc)
        exit(sc);
    sc = OsDrvTimerGetElapsedTicks(&g_ticks, &passed_cycles);
    if(sc)
        exit(sc);

    DrvTimerSleepMicro(2000);// small wait 2ms before sampling power rails - to ensure pipe is really active
    u32 ret = Brd198SampleAllRails(&powerMonHandle,&(test.power));

    if (ret != DRV_BRD198_DRV_SUCCESS)
    {
         printf("Error reading power rails: %d\n", ret);
         exit(-1);
    }

    //OpipeWait(&opF.p);

    DrvTempSensorGetTemp(&temperature); //auto!!

    for (i = 0; i < SHAVES_USED; i++){
        sc = OsDrvSvuCloseShave(&handler[i]);
        if(sc)
            exit(sc);
    }
    printf("\n Completed Harris Corner Detection.\n");

    int total_ncorn = brthYASM0_ncorn +
    				  brthYASM1_ncorn +
					  brthYASM2_ncorn +
					  brthYASM3_ncorn +
					  brthYASM4_ncorn +
					  brthYASM5_ncorn +
					  brthYASM6_ncorn +
					  brthYASM7_ncorn +
					  brthYASM8_ncorn +
					  brthYASM9_ncorn +
					  brthYASM10_ncorn +
					  brthYASM11_ncorn;


    printf("Got %d corners in total\n", total_ncorn);


    avgFps = (fp32) (DrvCprGetClockFreqKhz(SYS_CLK, NULL) * 1000) / (float) (passed_cycles); //(frameTime was here

    printf("Average Frames Per Second: %.03f fps, Total Leon Cycles Elapsed: %llucc, ", avgFps, passed_cycles); //%08ucc


    time_elapsed = DrvTimerTicksToMs(passed_cycles);
    //printf("\nLeon Cycles Elapsed %d \n", (u32)(passed_cycles));


    printf("\nTime  Elapsed %d ms \n", (u32)(time_elapsed));
    showPower(temperature);

    printf("SHAVE 1 got %d corners: \n", brthYASM0_ncorn);
    for(i=0; i<brthYASM0_ncorn; ++i)
         printf("%.4f %.4f\n", brthYASM0_cornrs[i][0], brthYASM0_cornrs[i][1]); //this is always 6.0 because for the specific img size and shaves_used, offheight is 7

    printf("SHAVE 2 got %d corners: \n", brthYASM1_ncorn);
    for(i=0; i<brthYASM1_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM1_cornrs[i][0], brthYASM1_cornrs[i][1] + SHAVE_HEIGHT); //think about LINE_PADDING as well!!

    printf("SHAVE 3 got %d corners: \n", brthYASM2_ncorn);
    for(i=0; i<brthYASM2_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM2_cornrs[i][0], brthYASM2_cornrs[i][1] + SHAVE_HEIGHT*2);

    printf("SHAVE 4 got %d corners: \n", brthYASM3_ncorn);
    for(i=0; i<brthYASM3_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM3_cornrs[i][0], brthYASM3_cornrs[i][1] + SHAVE_HEIGHT*3);

    printf("SHAVE 5 got %d corners: \n", brthYASM4_ncorn);
    for(i=0; i<brthYASM4_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM4_cornrs[i][0], brthYASM4_cornrs[i][1] + SHAVE_HEIGHT*4);

    printf("SHAVE 6 got %d corners: \n", brthYASM5_ncorn);
    for(i=0; i<brthYASM5_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM5_cornrs[i][0], brthYASM5_cornrs[i][1] + SHAVE_HEIGHT*5);

    printf("SHAVE 7 got %d corners: \n", brthYASM6_ncorn);
    for(i=0; i<brthYASM6_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM6_cornrs[i][0], brthYASM6_cornrs[i][1] + SHAVE_HEIGHT*6);

    printf("SHAVE 8 got %d corners: \n", brthYASM7_ncorn);
    for(i=0; i<brthYASM7_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM7_cornrs[i][0], brthYASM7_cornrs[i][1] + SHAVE_HEIGHT*7);

    printf("SHAVE 9 got %d corners: \n", brthYASM8_ncorn);
    for(i=0; i<brthYASM8_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM8_cornrs[i][0], brthYASM8_cornrs[i][1] + SHAVE_HEIGHT*8);

    printf("SHAVE 10 got %d corners: \n", brthYASM9_ncorn);
    for(i=0; i<brthYASM9_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM9_cornrs[i][0], brthYASM9_cornrs[i][1] + SHAVE_HEIGHT*9);

    printf("SHAVE 11 got %d corners: \n", brthYASM10_ncorn);
    for(i=0; i<brthYASM10_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM10_cornrs[i][0], brthYASM10_cornrs[i][1] + SHAVE_HEIGHT*10);

    printf("SHAVE 12 got %d corners: \n", brthYASM11_ncorn);
    for(i=0; i<brthYASM11_ncorn; ++i)
             printf("%.4f %.4f\n", brthYASM11_cornrs[i][0], brthYASM11_cornrs[i][1] + SHAVE_HEIGHT*11);


    //printf("\nLeon executed %lu cycles in total\n\n",(u32)(passed_cycles));




    printf("To check the result use:\n");
    //printf("save outputFrame (80*60*3/2) image_80x60_P420.yuv\n");
    printf("savefile  imagename.imgtype outputFrame [<img_size>] \n");


    exit(0);
}

// User extension to be able to catch abnormal terminations
static void Fatal_extension(
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
)
{
    switch(the_source)
    {
    case RTEMS_FATAL_SOURCE_EXIT:
        if(the_error)
            printk("Exited with error code %d\n", the_error);
        break; // normal exit
    case RTEMS_FATAL_SOURCE_ASSERT:
        printk("%s : %d in %s \n",
               ((rtems_assert_context *)the_error)->file,
               ((rtems_assert_context *)the_error)->line,
               ((rtems_assert_context *)the_error)->function);
        break;
    case RTEMS_FATAL_SOURCE_EXCEPTION:
        rtems_exception_frame_print((const rtems_exception_frame *) the_error);
        break;
    default:
        printk ("\nSource %d Internal %d Error %d  0x%X:\n",
                the_source, is_internal, the_error, the_error);
        break;
    }
}
