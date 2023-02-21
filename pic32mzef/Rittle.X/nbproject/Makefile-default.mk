#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../core/fatfs/source/ff.c ../../core/fatfs/source/ffsystem.c ../../core/fatfs/source/ffunicode.c ../../core/main.c ../../core/ride.c ../../core/rittle.c ../../core/rsc.c ../../core/rvm.c ../../core/xmem.c ../drivers/drv_lcd4.c ../drivers/drv_lcds.c ../drivers/drv_trspi.c ../drivers/drv_ssd196.c ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c ../plibs/usb/usb/src/dynamic/usb_device.c ../plibs/usb/usb/src/dynamic/usb_device_cdc.c ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c ../diskio.c ../platform.c ../sd_spi.c ../addons.c ../tokens.c ../usb_descriptors.c ../uconsole.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/794007692/ff.o ${OBJECTDIR}/_ext/794007692/ffsystem.o ${OBJECTDIR}/_ext/794007692/ffunicode.o ${OBJECTDIR}/_ext/1853363519/main.o ${OBJECTDIR}/_ext/1853363519/ride.o ${OBJECTDIR}/_ext/1853363519/rittle.o ${OBJECTDIR}/_ext/1853363519/rsc.o ${OBJECTDIR}/_ext/1853363519/rvm.o ${OBJECTDIR}/_ext/1853363519/xmem.o ${OBJECTDIR}/_ext/239857660/drv_lcd4.o ${OBJECTDIR}/_ext/239857660/drv_lcds.o ${OBJECTDIR}/_ext/239857660/drv_trspi.o ${OBJECTDIR}/_ext/239857660/drv_ssd196.o ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o ${OBJECTDIR}/_ext/1785746338/usb_device.o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o ${OBJECTDIR}/_ext/1472/diskio.o ${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/1472/sd_spi.o ${OBJECTDIR}/_ext/1472/addons.o ${OBJECTDIR}/_ext/1472/tokens.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/uconsole.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/794007692/ff.o.d ${OBJECTDIR}/_ext/794007692/ffsystem.o.d ${OBJECTDIR}/_ext/794007692/ffunicode.o.d ${OBJECTDIR}/_ext/1853363519/main.o.d ${OBJECTDIR}/_ext/1853363519/ride.o.d ${OBJECTDIR}/_ext/1853363519/rittle.o.d ${OBJECTDIR}/_ext/1853363519/rsc.o.d ${OBJECTDIR}/_ext/1853363519/rvm.o.d ${OBJECTDIR}/_ext/1853363519/xmem.o.d ${OBJECTDIR}/_ext/239857660/drv_lcd4.o.d ${OBJECTDIR}/_ext/239857660/drv_lcds.o.d ${OBJECTDIR}/_ext/239857660/drv_trspi.o.d ${OBJECTDIR}/_ext/239857660/drv_ssd196.o.d ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o.d ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o.d ${OBJECTDIR}/_ext/1785746338/usb_device.o.d ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o.d ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o.d ${OBJECTDIR}/_ext/1472/diskio.o.d ${OBJECTDIR}/_ext/1472/platform.o.d ${OBJECTDIR}/_ext/1472/sd_spi.o.d ${OBJECTDIR}/_ext/1472/addons.o.d ${OBJECTDIR}/_ext/1472/tokens.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d ${OBJECTDIR}/_ext/1472/uconsole.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/794007692/ff.o ${OBJECTDIR}/_ext/794007692/ffsystem.o ${OBJECTDIR}/_ext/794007692/ffunicode.o ${OBJECTDIR}/_ext/1853363519/main.o ${OBJECTDIR}/_ext/1853363519/ride.o ${OBJECTDIR}/_ext/1853363519/rittle.o ${OBJECTDIR}/_ext/1853363519/rsc.o ${OBJECTDIR}/_ext/1853363519/rvm.o ${OBJECTDIR}/_ext/1853363519/xmem.o ${OBJECTDIR}/_ext/239857660/drv_lcd4.o ${OBJECTDIR}/_ext/239857660/drv_lcds.o ${OBJECTDIR}/_ext/239857660/drv_trspi.o ${OBJECTDIR}/_ext/239857660/drv_ssd196.o ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o ${OBJECTDIR}/_ext/1785746338/usb_device.o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o ${OBJECTDIR}/_ext/1472/diskio.o ${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/1472/sd_spi.o ${OBJECTDIR}/_ext/1472/addons.o ${OBJECTDIR}/_ext/1472/tokens.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/uconsole.o

# Source Files
SOURCEFILES=../../core/fatfs/source/ff.c ../../core/fatfs/source/ffsystem.c ../../core/fatfs/source/ffunicode.c ../../core/main.c ../../core/ride.c ../../core/rittle.c ../../core/rsc.c ../../core/rvm.c ../../core/xmem.c ../drivers/drv_lcd4.c ../drivers/drv_lcds.c ../drivers/drv_trspi.c ../drivers/drv_ssd196.c ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c ../plibs/usb/usb/src/dynamic/usb_device.c ../plibs/usb/usb/src/dynamic/usb_device_cdc.c ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c ../diskio.c ../platform.c ../sd_spi.c ../addons.c ../tokens.c ../usb_descriptors.c ../uconsole.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MZ2048EFH064
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/794007692/ff.o: ../../core/fatfs/source/ff.c  .generated_files/flags/default/cf3425872337433fc44363d1ef37fba5a112e5d7 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ff.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ff.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ff.o.d" -o ${OBJECTDIR}/_ext/794007692/ff.o ../../core/fatfs/source/ff.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/794007692/ffsystem.o: ../../core/fatfs/source/ffsystem.c  .generated_files/flags/default/cdba09bf7ba92eed10d25ae10572bde52da306cb .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffsystem.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffsystem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ffsystem.o.d" -o ${OBJECTDIR}/_ext/794007692/ffsystem.o ../../core/fatfs/source/ffsystem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/794007692/ffunicode.o: ../../core/fatfs/source/ffunicode.c  .generated_files/flags/default/7dd186871b47e7d4da4c6a6013d45a91336857f3 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffunicode.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffunicode.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ffunicode.o.d" -o ${OBJECTDIR}/_ext/794007692/ffunicode.o ../../core/fatfs/source/ffunicode.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/main.o: ../../core/main.c  .generated_files/flags/default/710e1f68654389cbaf2fccd26fb47ac2aa216974 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/main.o.d" -o ${OBJECTDIR}/_ext/1853363519/main.o ../../core/main.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/ride.o: ../../core/ride.c  .generated_files/flags/default/d3e5ef7e092ffd6a3ff2c26a91adf33ed0b517d4 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/ride.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/ride.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/ride.o.d" -o ${OBJECTDIR}/_ext/1853363519/ride.o ../../core/ride.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rittle.o: ../../core/rittle.c  .generated_files/flags/default/7ae2cbb3b740847313dbd0fdbaa42fc88b9241e4 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rittle.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rittle.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rittle.o.d" -o ${OBJECTDIR}/_ext/1853363519/rittle.o ../../core/rittle.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rsc.o: ../../core/rsc.c  .generated_files/flags/default/eb4c08087e695d966408e4399af8dfbb7c9f9fb1 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rsc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rsc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rsc.o.d" -o ${OBJECTDIR}/_ext/1853363519/rsc.o ../../core/rsc.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rvm.o: ../../core/rvm.c  .generated_files/flags/default/28fc18e1076fe32dba0c0b5b06ba88f660616f60 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rvm.o.d" -o ${OBJECTDIR}/_ext/1853363519/rvm.o ../../core/rvm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/xmem.o: ../../core/xmem.c  .generated_files/flags/default/90ee0b3cc0c7f10048cc1c18a8819948eb350385 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/xmem.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/xmem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/xmem.o.d" -o ${OBJECTDIR}/_ext/1853363519/xmem.o ../../core/xmem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_lcd4.o: ../drivers/drv_lcd4.c  .generated_files/flags/default/db075f2f992949ccbc1163ac7460db88ca0a4465 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcd4.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcd4.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_lcd4.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_lcd4.o ../drivers/drv_lcd4.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_lcds.o: ../drivers/drv_lcds.c  .generated_files/flags/default/69c9b65676905be3724aac45e67911af1c84a9f .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcds.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcds.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_lcds.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_lcds.o ../drivers/drv_lcds.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_trspi.o: ../drivers/drv_trspi.c  .generated_files/flags/default/10faf2d235959370e9c7d0bcd2368ee030b48e2f .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_trspi.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_trspi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_trspi.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_trspi.o ../drivers/drv_trspi.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_ssd196.o: ../drivers/drv_ssd196.c  .generated_files/flags/default/44abf36381537ddaecb3193170191377af1f1982 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_ssd196.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_ssd196.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_ssd196.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_ssd196.o ../drivers/drv_ssd196.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1478003949/drv_usbhs.o: ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c  .generated_files/flags/default/e1d44d0b2fadc32ad21f6e92a336db1bcc9c84b7 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1478003949" 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o.d 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1478003949/drv_usbhs.o.d" -o ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o: ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c  .generated_files/flags/default/355ed7fee2faa9bbebb959e9aa29451756c28f7b .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1478003949" 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o.d" -o ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device.o: ../plibs/usb/usb/src/dynamic/usb_device.c  .generated_files/flags/default/5e0963bc9357c8d3305362e4d702d92660b914e2 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device.o ../plibs/usb/usb/src/dynamic/usb_device.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o: ../plibs/usb/usb/src/dynamic/usb_device_cdc.c  .generated_files/flags/default/f4ec7c7dd7c50eb7caf23c0995ce9361863a24b0 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o ../plibs/usb/usb/src/dynamic/usb_device_cdc.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o: ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c  .generated_files/flags/default/bdb6fe03b43cdbf69c5e6df32e2384b91205ea09 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/diskio.o: ../diskio.c  .generated_files/flags/default/41fa4f4709e55829aeab9f0147f9d14ddfcfd573 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/diskio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/diskio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/diskio.o.d" -o ${OBJECTDIR}/_ext/1472/diskio.o ../diskio.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  .generated_files/flags/default/e21875a36cd53dbce5da5419d0c35a46567cdba6 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/sd_spi.o: ../sd_spi.c  .generated_files/flags/default/7ef5e00bde1dc0124df7fd579b0cc4187c2b966b .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/sd_spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/sd_spi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/sd_spi.o.d" -o ${OBJECTDIR}/_ext/1472/sd_spi.o ../sd_spi.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/addons.o: ../addons.c  .generated_files/flags/default/85fd13b86c338957ad8d4a570e0b5c8c529fb22e .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/addons.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/addons.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/addons.o.d" -o ${OBJECTDIR}/_ext/1472/addons.o ../addons.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/tokens.o: ../tokens.c  .generated_files/flags/default/58f444e1df481c3d42c3d1a443e5b6ac33a28bb3 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/tokens.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/tokens.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/tokens.o.d" -o ${OBJECTDIR}/_ext/1472/tokens.o ../tokens.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  .generated_files/flags/default/307af21f2556ee18c39432fc8cce7e35a467936c .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/uconsole.o: ../uconsole.c  .generated_files/flags/default/fbd282c5665c9ae1477fe6a90bf98d73116f7264 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/uconsole.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/uconsole.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/uconsole.o.d" -o ${OBJECTDIR}/_ext/1472/uconsole.o ../uconsole.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
else
${OBJECTDIR}/_ext/794007692/ff.o: ../../core/fatfs/source/ff.c  .generated_files/flags/default/464e9acc9ee6bcc986f233fbdec4cc6cc88004f8 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ff.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ff.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ff.o.d" -o ${OBJECTDIR}/_ext/794007692/ff.o ../../core/fatfs/source/ff.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/794007692/ffsystem.o: ../../core/fatfs/source/ffsystem.c  .generated_files/flags/default/1691b59156b752736b7054960cdb9509eceb5c0 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffsystem.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffsystem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ffsystem.o.d" -o ${OBJECTDIR}/_ext/794007692/ffsystem.o ../../core/fatfs/source/ffsystem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/794007692/ffunicode.o: ../../core/fatfs/source/ffunicode.c  .generated_files/flags/default/b547344a6e4ce5bfa7c261f285cb2e49e18ed818 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/794007692" 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffunicode.o.d 
	@${RM} ${OBJECTDIR}/_ext/794007692/ffunicode.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/794007692/ffunicode.o.d" -o ${OBJECTDIR}/_ext/794007692/ffunicode.o ../../core/fatfs/source/ffunicode.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/main.o: ../../core/main.c  .generated_files/flags/default/b0f545fa419011d4d20733d0bf301adbf73227e7 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/main.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/main.o.d" -o ${OBJECTDIR}/_ext/1853363519/main.o ../../core/main.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/ride.o: ../../core/ride.c  .generated_files/flags/default/901ffa59cbbd273b1f69bb2d294cd85afd322e .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/ride.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/ride.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/ride.o.d" -o ${OBJECTDIR}/_ext/1853363519/ride.o ../../core/ride.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rittle.o: ../../core/rittle.c  .generated_files/flags/default/f02bc21d2828ffb372afcb3027ea99524e86fc86 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rittle.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rittle.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rittle.o.d" -o ${OBJECTDIR}/_ext/1853363519/rittle.o ../../core/rittle.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rsc.o: ../../core/rsc.c  .generated_files/flags/default/cfc5f2a7d60007ac0b0a5a4d0bcc34c8114428f1 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rsc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rsc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rsc.o.d" -o ${OBJECTDIR}/_ext/1853363519/rsc.o ../../core/rsc.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/rvm.o: ../../core/rvm.c  .generated_files/flags/default/481bb2b9bf75885b3ec9bb6d9de41448a0b4e1be .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/rvm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/rvm.o.d" -o ${OBJECTDIR}/_ext/1853363519/rvm.o ../../core/rvm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1853363519/xmem.o: ../../core/xmem.c  .generated_files/flags/default/18c9fc467dccdeb03c802da5912c52c736d340e0 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1853363519" 
	@${RM} ${OBJECTDIR}/_ext/1853363519/xmem.o.d 
	@${RM} ${OBJECTDIR}/_ext/1853363519/xmem.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1853363519/xmem.o.d" -o ${OBJECTDIR}/_ext/1853363519/xmem.o ../../core/xmem.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_lcd4.o: ../drivers/drv_lcd4.c  .generated_files/flags/default/cade28ece69cb56e8add9489f9e6e1e3fe5035b3 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcd4.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcd4.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_lcd4.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_lcd4.o ../drivers/drv_lcd4.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_lcds.o: ../drivers/drv_lcds.c  .generated_files/flags/default/1ee9ae823d96179e8e203dafde24c9c36afdf8fd .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcds.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_lcds.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_lcds.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_lcds.o ../drivers/drv_lcds.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_trspi.o: ../drivers/drv_trspi.c  .generated_files/flags/default/e2889ab39163482069d40b312ffd75ee1ec8191e .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_trspi.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_trspi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_trspi.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_trspi.o ../drivers/drv_trspi.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/239857660/drv_ssd196.o: ../drivers/drv_ssd196.c  .generated_files/flags/default/23771c37f740ce0e79e9a0bcbf2a6f7df5e35622 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/239857660" 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_ssd196.o.d 
	@${RM} ${OBJECTDIR}/_ext/239857660/drv_ssd196.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/239857660/drv_ssd196.o.d" -o ${OBJECTDIR}/_ext/239857660/drv_ssd196.o ../drivers/drv_ssd196.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1478003949/drv_usbhs.o: ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c  .generated_files/flags/default/e141aa6e5734fa89ad481c83da4dd431e330da67 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1478003949" 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o.d 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1478003949/drv_usbhs.o.d" -o ${OBJECTDIR}/_ext/1478003949/drv_usbhs.o ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o: ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c  .generated_files/flags/default/21062d2f2f3c1e83abfd3f664d19815badaa6dba .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1478003949" 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o.d" -o ${OBJECTDIR}/_ext/1478003949/drv_usbhs_device.o ../plibs/usb/driver/usb/usbhs/src/dynamic/drv_usbhs_device.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device.o: ../plibs/usb/usb/src/dynamic/usb_device.c  .generated_files/flags/default/8a86c9e7840f63081393c107f9b6ba66e09c70ea .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device.o ../plibs/usb/usb/src/dynamic/usb_device.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o: ../plibs/usb/usb/src/dynamic/usb_device_cdc.c  .generated_files/flags/default/494eb634b1f70b22fd254a2767dca238fdb4f9c1 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc.o ../plibs/usb/usb/src/dynamic/usb_device_cdc.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o: ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c  .generated_files/flags/default/eceac8376624d0fcb857d4af30e38732599cc140 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1785746338" 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o.d 
	@${RM} ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o.d" -o ${OBJECTDIR}/_ext/1785746338/usb_device_cdc_acm.o ../plibs/usb/usb/src/dynamic/usb_device_cdc_acm.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/diskio.o: ../diskio.c  .generated_files/flags/default/877a655ec947cc6d00abd05e9153006f6642957f .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/diskio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/diskio.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/diskio.o.d" -o ${OBJECTDIR}/_ext/1472/diskio.o ../diskio.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  .generated_files/flags/default/60f3f1e9d1c8e7571f157ba2927074a61e9b5e19 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/sd_spi.o: ../sd_spi.c  .generated_files/flags/default/d032a8e58639409ecc40a9a9aa9520f78b476226 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/sd_spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/sd_spi.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/sd_spi.o.d" -o ${OBJECTDIR}/_ext/1472/sd_spi.o ../sd_spi.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/addons.o: ../addons.c  .generated_files/flags/default/39a3cd47c77202164f4a50042ea63c099d3fc453 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/addons.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/addons.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/addons.o.d" -o ${OBJECTDIR}/_ext/1472/addons.o ../addons.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/tokens.o: ../tokens.c  .generated_files/flags/default/3231a850f4f8278ecd59cfabc47741f980b8390d .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/tokens.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/tokens.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/tokens.o.d" -o ${OBJECTDIR}/_ext/1472/tokens.o ../tokens.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  .generated_files/flags/default/4206b27a7c94cbd54cda62558fd9de9e76548714 .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
${OBJECTDIR}/_ext/1472/uconsole.o: ../uconsole.c  .generated_files/flags/default/9eef875aa70a5837766fc054e04a4362d0a54c8e .generated_files/flags/default/6829b0aeba8885902dde10b5e844e90dd094891a
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/uconsole.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/uconsole.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -DPIC32MZEF -DXMEM_SIZE=503000 -MP -MMD -MF "${OBJECTDIR}/_ext/1472/uconsole.o.d" -o ${OBJECTDIR}/_ext/1472/uconsole.o ../uconsole.c    -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fgnu89-inline   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g   -mprocessor=$(MP_PROCESSOR_OPTION) -O3 --fill=0xFF -o dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -legacy-libc  -fgnu89-inline $(COMPARISON_BUILD)      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=_min_stack_size=8192,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,-s,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION) -O3 --fill=0xFF -o dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)  -legacy-libc  -fgnu89-inline $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_stack_size=8192,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,-s,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml 
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} --write-sla 
	@echo Normalizing hex file
	@"C:/Program Files/Microchip/MPLABX/v5.50/mplab_platform/platform/../mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files/Microchip/MPLABX/v5.50/mplab_platform/platform/../mplab_ide/modules/../../dat/en_msgs.txt" dist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.hex -odist/${CND_CONF}/${IMAGE_TYPE}/Rittle.X.${IMAGE_TYPE}.hex

endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
