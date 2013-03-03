################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/cpu/chunks.c \
../src/core/cpu/ops.c \
../src/core/cpu/timers.c 

OBJS += \
./src/core/cpu/chunks.o \
./src/core/cpu/ops.o \
./src/core/cpu/timers.o 

C_DEPS += \
./src/core/cpu/chunks.d \
./src/core/cpu/ops.d \
./src/core/cpu/timers.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/cpu/%.o: ../src/core/cpu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


