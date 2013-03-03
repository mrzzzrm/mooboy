################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/cli/cli.c 

OBJS += \
./src/sys/cli/cli.o 

C_DEPS += \
./src/sys/cli/cli.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/cli/%.o: ../src/sys/cli/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


