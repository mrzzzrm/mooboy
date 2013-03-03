################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/cpu.c \
../src/core/emu.c \
../src/core/loader.c \
../src/core/mem.c 

OBJS += \
./src/core/cpu.o \
./src/core/emu.o \
./src/core/loader.o \
./src/core/mem.o 

C_DEPS += \
./src/core/cpu.d \
./src/core/emu.d \
./src/core/loader.d \
./src/core/mem.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/%.o: ../src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


