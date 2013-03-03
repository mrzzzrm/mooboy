################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rom/filltest.c \
../rom/hello.c \
../rom/simple.c 

OBJS += \
./rom/filltest.o \
./rom/hello.o \
./rom/simple.o 

C_DEPS += \
./rom/filltest.d \
./rom/hello.d \
./rom/simple.d 


# Each subdirectory must supply rules for building sources it contributes
rom/%.o: ../rom/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


