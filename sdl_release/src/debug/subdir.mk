################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/debug/break.c \
../src/debug/debug.c \
../src/debug/disasm.c \
../src/debug/dump.c \
../src/debug/int.c \
../src/debug/mon.c \
../src/debug/monitor.c \
../src/debug/run.c \
../src/debug/sym.c \
../src/debug/utils.c 

OBJS += \
./src/debug/break.o \
./src/debug/debug.o \
./src/debug/disasm.o \
./src/debug/dump.o \
./src/debug/int.o \
./src/debug/mon.o \
./src/debug/monitor.o \
./src/debug/run.o \
./src/debug/sym.o \
./src/debug/utils.o 

C_DEPS += \
./src/debug/break.d \
./src/debug/debug.d \
./src/debug/disasm.d \
./src/debug/dump.d \
./src/debug/int.d \
./src/debug/mon.d \
./src/debug/monitor.d \
./src/debug/run.d \
./src/debug/sym.d \
./src/debug/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/debug/%.o: ../src/debug/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


