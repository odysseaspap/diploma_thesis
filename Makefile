# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
#APPNAME ?= $(notdir $(shell pwd))
MV_SOC_REV      ?= ma2150
# ------------------------------[ Build overrides ]--------------------------------#
# overrides that need to be set before including generic.mk

# Ensure that the we are using the correct rtems libs etc
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt
 
#LEON_ENDIAN =

# ------------------------------[ Components used ]--------------------------------#
#Component lists: 
#No components used by ParallelizeOneAlgo app
#Choosing if this project has shave components or not
SHAVE_COMPONENTS = no

#-------------------------------[ Settings ]---------------------------------------#
MV_SOC_PLATFORM ?= myriad2
#-------------------------------[ Local shave applications sources ]--------------------#
MYRIAD = myriad2Asm

#Choosing C sources the brthYASM application on shave
ParallelizeOneAlgoApp = shave/brthYASM
SHAVE_C_SOURCES_brthYASM = $(wildcard $(DirAppRoot)/shave/*.c)
#Choosing ASM sources for the shave brthYASM app on shave
#SHAVE_ASM_SOURCES_brthYASM = $(wildcard $(DirAppRoot)/shave/$(MYRIAD)/*.asm)

#Generating list of required generated assembly files for the ParallelizeOneAlgo app on shave
SHAVE_GENASMS_brthYASM = $(patsubst %.c,$(DirAppObjBase)%.asmgen,$(SHAVE_C_SOURCES_brthYASM))
#Generating required objects list from sources
SHAVE_brthYASM_OBJS = $(patsubst $(DirAppObjBase)%.asmgen,$(DirAppObjBase)%_shave.o,$(SHAVE_GENASMS_brthYASM)) 
#\					$(patsubst %.asm,$(DirAppObjBase)%_shave.o,$(SHAVE_ASM_SOURCES_brthYASM))

#update clean rules with our generated files
PROJECTCLEAN += $(SHAVE_GENASMS_brthYASM) $(SHAVE_brthYASM_OBJS)
#Uncomment below to reject generated shave as intermediary files (consider them secondary)
PROJECTINTERM += $(SHAVE_GENASMS_brthYASM)

#--------------------------[ Shave applications section ]--------------------------#
SHAVE_APP_LIBS = $(ParallelizeOneAlgoApp).mvlib
SHAVE0_APPS = $(ParallelizeOneAlgoApp).shv0lib
SHAVE1_APPS = $(ParallelizeOneAlgoApp).shv1lib
SHAVE2_APPS = $(ParallelizeOneAlgoApp).shv2lib
SHAVE3_APPS = $(ParallelizeOneAlgoApp).shv3lib
SHAVE4_APPS = $(ParallelizeOneAlgoApp).shv4lib
SHAVE5_APPS = $(ParallelizeOneAlgoApp).shv5lib
SHAVE6_APPS = $(ParallelizeOneAlgoApp).shv6lib
SHAVE7_APPS = $(ParallelizeOneAlgoApp).shv7lib
SHAVE8_APPS = $(ParallelizeOneAlgoApp).shv8lib
SHAVE9_APPS = $(ParallelizeOneAlgoApp).shv9lib
SHAVE10_APPS = $(ParallelizeOneAlgoApp).shv10lib
SHAVE11_APPS = $(ParallelizeOneAlgoApp).shv11lib


# ------------------------------[ Tools overrides ]--------------------------------#
# Hardcode tool version here if needed, otherwise defaults to latest stable release
#MV_TOOLS_VERSION ?=

# Verbose or not, uncomment for more verbosity
#ECHO ?=

RAWDATAOBJECTFILES += $(DirAppObjDir)/testframe.o 

# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MV_COMMON_BASE  ?= ../../../common

# Include the generic Makefile
include $(MV_COMMON_BASE)/generic.mk

#-------------------------------[ Local shave applications build rules ]------------------#
#Describe the rule for building the ParallelizeOneAlgoApp application. Simple rule specifying 
#which objects build up the said application. The application will be built into a library
ENTRYPOINTS = -e start  --gc-sections
$(ParallelizeOneAlgoApp).mvlib : $(SHAVE_brthYASM_OBJS) $(PROJECT_SHAVE_LIBS)
	$(ECHO) $(LD)  $(ENTRYPOINTS) $(MVLIBOPT) $(SHAVE_brthYASM_OBJS) $(PROJECT_SHAVE_LIBS) $(CompilerANSILibs) -o $@

# -------------------------------- [ Build Options ] ------------------------------ #
# App related build options 

# Extra app related options
#CCOPT			+= -DDEBUG

#MY_RESOURCE = $(MV_EXTRA_DATA)/DunLoghaire_80x60.yuv
MY_RESOURCE = HarrisCorner_512x384_8bpp.raw
#MY_RESOURCE = test_160x120_8bpp.raw
#MY_RESOURCE = test_80x60_8bpp.raw


# ------------------------------[ Extra Rules ]-------------------------------------------#

DDR_DATA = .ddr.data
REVERSE_BYTES=

# Add to MVASM include path
$(DirAppObjDir)/testframe.o: $(MY_RESOURCE) Makefile
	@mkdir -p $(dir $@)
	$(OBJCOPY) -I binary $(REVERSE_BYTES) --rename-section .data=.ddr.data \
	--redefine-sym  _binary_$(subst /,_,$(subst .,_,$<))_start=inputFrame \
	-O elf32-sparc -B sparc $< $@

TEST_TYPE        := AUTO
TEST_TAGS        := "MA2100, MA2150, TCL_MA2100,TCL_MA2150, MA2450"




