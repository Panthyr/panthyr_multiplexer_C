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
FINAL_IMAGE=${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS
SUB_IMAGE_ADDRESS_COMMAND=--image-address $(SUB_IMAGE_ADDRESS)
else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=main.c uart.c hardware.c LSM9DS1.c utils.c interrupts.c I2C1.c ST_LSM9DS1.c Sensirion_SHT31.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/main.o ${OBJECTDIR}/uart.o ${OBJECTDIR}/hardware.o ${OBJECTDIR}/LSM9DS1.o ${OBJECTDIR}/utils.o ${OBJECTDIR}/interrupts.o ${OBJECTDIR}/I2C1.o ${OBJECTDIR}/ST_LSM9DS1.o ${OBJECTDIR}/Sensirion_SHT31.o
POSSIBLE_DEPFILES=${OBJECTDIR}/main.o.d ${OBJECTDIR}/uart.o.d ${OBJECTDIR}/hardware.o.d ${OBJECTDIR}/LSM9DS1.o.d ${OBJECTDIR}/utils.o.d ${OBJECTDIR}/interrupts.o.d ${OBJECTDIR}/I2C1.o.d ${OBJECTDIR}/ST_LSM9DS1.o.d ${OBJECTDIR}/Sensirion_SHT31.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/main.o ${OBJECTDIR}/uart.o ${OBJECTDIR}/hardware.o ${OBJECTDIR}/LSM9DS1.o ${OBJECTDIR}/utils.o ${OBJECTDIR}/interrupts.o ${OBJECTDIR}/I2C1.o ${OBJECTDIR}/ST_LSM9DS1.o ${OBJECTDIR}/Sensirion_SHT31.o

# Source Files
SOURCEFILES=main.c uart.c hardware.c LSM9DS1.c utils.c interrupts.c I2C1.c ST_LSM9DS1.c Sensirion_SHT31.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ128GB204
MP_LINKER_FILE_OPTION=,--script=p24FJ128GB204.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/235217e0766fd9211023452e95a8e9bd6efaf736 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart.o: uart.c  .generated_files/flags/default/b6a7c211f4bdbf3b47db3976fcf171b8497a677b .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart.o.d 
	@${RM} ${OBJECTDIR}/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart.c  -o ${OBJECTDIR}/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hardware.o: hardware.c  .generated_files/flags/default/c263a72ca16aab50897ed7d9e424f982d9459649 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hardware.o.d 
	@${RM} ${OBJECTDIR}/hardware.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hardware.c  -o ${OBJECTDIR}/hardware.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hardware.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/LSM9DS1.o: LSM9DS1.c  .generated_files/flags/default/d804da3b685088d793a4cbd7632f407c82f1eec1 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LSM9DS1.o.d 
	@${RM} ${OBJECTDIR}/LSM9DS1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  LSM9DS1.c  -o ${OBJECTDIR}/LSM9DS1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/LSM9DS1.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/utils.o: utils.c  .generated_files/flags/default/6fed2d974af5bcb818b660eeaf75bd03c5eb0b4e .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/utils.o.d 
	@${RM} ${OBJECTDIR}/utils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  utils.c  -o ${OBJECTDIR}/utils.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/utils.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/interrupts.o: interrupts.c  .generated_files/flags/default/3417a602658fd5af67405d554e27a0315b7aa28f .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupts.o.d 
	@${RM} ${OBJECTDIR}/interrupts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  interrupts.c  -o ${OBJECTDIR}/interrupts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/interrupts.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/I2C1.o: I2C1.c  .generated_files/flags/default/8683e09231dc6c401210c6a30d2b6ed2d894b470 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/I2C1.o.d 
	@${RM} ${OBJECTDIR}/I2C1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  I2C1.c  -o ${OBJECTDIR}/I2C1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/I2C1.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/ST_LSM9DS1.o: ST_LSM9DS1.c  .generated_files/flags/default/ee7b8e724fe45ab41ff3337067c8dc42ddc116be .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ST_LSM9DS1.o.d 
	@${RM} ${OBJECTDIR}/ST_LSM9DS1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ST_LSM9DS1.c  -o ${OBJECTDIR}/ST_LSM9DS1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/ST_LSM9DS1.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/Sensirion_SHT31.o: Sensirion_SHT31.c  .generated_files/flags/default/9a27af0710682bcf18f9d16247f0d542917a02fc .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Sensirion_SHT31.o.d 
	@${RM} ${OBJECTDIR}/Sensirion_SHT31.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Sensirion_SHT31.c  -o ${OBJECTDIR}/Sensirion_SHT31.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/Sensirion_SHT31.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/21241733867b37e921f0ed3b97d2ea3a09d31ad7 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart.o: uart.c  .generated_files/flags/default/ba8839eb033f9f7723af9d273f1ba4b02b8ed170 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart.o.d 
	@${RM} ${OBJECTDIR}/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart.c  -o ${OBJECTDIR}/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/hardware.o: hardware.c  .generated_files/flags/default/c6c87f34453b1f6ecffb881c299908c50cbb291e .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hardware.o.d 
	@${RM} ${OBJECTDIR}/hardware.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hardware.c  -o ${OBJECTDIR}/hardware.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/hardware.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/LSM9DS1.o: LSM9DS1.c  .generated_files/flags/default/1c9eee77f580bb34ef06f2dc02dde47b5578bff3 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/LSM9DS1.o.d 
	@${RM} ${OBJECTDIR}/LSM9DS1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  LSM9DS1.c  -o ${OBJECTDIR}/LSM9DS1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/LSM9DS1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/utils.o: utils.c  .generated_files/flags/default/bf997e135fe95287f891b75c3c3181128a89f145 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/utils.o.d 
	@${RM} ${OBJECTDIR}/utils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  utils.c  -o ${OBJECTDIR}/utils.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/utils.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/interrupts.o: interrupts.c  .generated_files/flags/default/ea4513248d11a774363867eb89f7b594512416f1 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupts.o.d 
	@${RM} ${OBJECTDIR}/interrupts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  interrupts.c  -o ${OBJECTDIR}/interrupts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/interrupts.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/I2C1.o: I2C1.c  .generated_files/flags/default/62d5aa5470dfe3bc711a3ca5378ef1606e89ba01 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/I2C1.o.d 
	@${RM} ${OBJECTDIR}/I2C1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  I2C1.c  -o ${OBJECTDIR}/I2C1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/I2C1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/ST_LSM9DS1.o: ST_LSM9DS1.c  .generated_files/flags/default/e8b52d55c2aa878fc4abc661a4599e760c5ce0e7 .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ST_LSM9DS1.o.d 
	@${RM} ${OBJECTDIR}/ST_LSM9DS1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ST_LSM9DS1.c  -o ${OBJECTDIR}/ST_LSM9DS1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/ST_LSM9DS1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/Sensirion_SHT31.o: Sensirion_SHT31.c  .generated_files/flags/default/a156c1b90aaa5a2c1e31410fa9f6d8c9042718cc .generated_files/flags/default/11e7e54691ad743698939ed6ca9c3d8ec227808e
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Sensirion_SHT31.o.d 
	@${RM} ${OBJECTDIR}/Sensirion_SHT31.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Sensirion_SHT31.c  -o ${OBJECTDIR}/Sensirion_SHT31.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/Sensirion_SHT31.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -ffunction-sections -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG   -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}\\xc16-bin2hex ${DISTDIR}/Hypermaq_Topside.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
