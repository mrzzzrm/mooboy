################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/mem/mbc/rtc.c 

OBJS += \
./src/core/mem/mbc/rtc.o 

C_DEPS += \
./src/core/mem/mbc/rtc.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/mem/mbc/%.o: ../src/core/mem/mbc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


