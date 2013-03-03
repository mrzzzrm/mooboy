################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/mem/io.c \
../src/core/mem/mbc.c 

OBJS += \
./src/core/mem/io.o \
./src/core/mem/mbc.o 

C_DEPS += \
./src/core/mem/io.d \
./src/core/mem/mbc.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/mem/%.o: ../src/core/mem/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


