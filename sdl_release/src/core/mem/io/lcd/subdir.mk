################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/mem/io/lcd/maps.c \
../src/core/mem/io/lcd/obj.c \
../src/core/mem/io/lcd/tile.c 

OBJS += \
./src/core/mem/io/lcd/maps.o \
./src/core/mem/io/lcd/obj.o \
./src/core/mem/io/lcd/tile.o 

C_DEPS += \
./src/core/mem/io/lcd/maps.d \
./src/core/mem/io/lcd/obj.d \
./src/core/mem/io/lcd/tile.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/mem/io/lcd/%.o: ../src/core/mem/io/lcd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


