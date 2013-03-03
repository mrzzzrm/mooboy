################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/cpu/src/main.c 

OBJS += \
./test/cpu/src/main.o 

C_DEPS += \
./test/cpu/src/main.d 


# Each subdirectory must supply rules for building sources it contributes
test/cpu/src/%.o: ../test/cpu/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


