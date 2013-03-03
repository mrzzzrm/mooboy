################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/flags/src/main.c 

OBJS += \
./test/flags/src/main.o 

C_DEPS += \
./test/flags/src/main.d 


# Each subdirectory must supply rules for building sources it contributes
test/flags/src/%.o: ../test/flags/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


