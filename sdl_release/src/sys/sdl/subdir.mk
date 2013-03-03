################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sys/sdl/sdl.c 

OBJS += \
./src/sys/sdl/sdl.o 

C_DEPS += \
./src/sys/sdl/sdl.d 


# Each subdirectory must supply rules for building sources it contributes
src/sys/sdl/%.o: ../src/sys/sdl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


