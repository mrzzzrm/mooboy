################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/mem/io/joy.c \
../src/core/mem/io/lcd.c 

OBJS += \
./src/core/mem/io/joy.o \
./src/core/mem/io/lcd.o 

C_DEPS += \
./src/core/mem/io/joy.d \
./src/core/mem/io/lcd.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/mem/io/%.o: ../src/core/mem/io/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


