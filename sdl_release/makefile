################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include tools/disasm/subdir.mk
-include test/mem/src/subdir.mk
-include test/mem/obj/Debug/src/util/subdir.mk
-include test/mem/obj/Debug/src/sys/cli/subdir.mk
-include test/mem/obj/Debug/src/core/cpu/subdir.mk
-include test/mem/obj/Debug/src/core/subdir.mk
-include test/mem/obj/Debug/src/subdir.mk
-include test/flags/src/subdir.mk
-include test/cpu/src/subdir.mk
-include test/cpu/obj/Debug/src/util/subdir.mk
-include test/cpu/obj/Debug/src/core/cpu/subdir.mk
-include test/cpu/obj/Debug/src/core/subdir.mk
-include test/cpu/obj/Debug/src/subdir.mk
-include src/util/subdir.mk
-include src/sys/sdl/subdir.mk
-include src/sys/cli/subdir.mk
-include src/debug/subdir.mk
-include src/core/mem/mbc/subdir.mk
-include src/core/mem/io/lcd/subdir.mk
-include src/core/mem/io/subdir.mk
-include src/core/mem/subdir.mk
-include src/core/cpu/subdir.mk
-include src/core/subdir.mk
-include src/subdir.mk
-include rom/subdir.mk
-include build/bin/sdl_release/src/util/subdir.mk
-include build/bin/sdl_release/src/sys/sdl/subdir.mk
-include build/bin/sdl_release/src/sys/cli/subdir.mk
-include build/bin/sdl_release/src/debug/subdir.mk
-include build/bin/sdl_release/src/core/mem/mbc/subdir.mk
-include build/bin/sdl_release/src/core/mem/io/lcd/subdir.mk
-include build/bin/sdl_release/src/core/mem/io/subdir.mk
-include build/bin/sdl_release/src/core/mem/subdir.mk
-include build/bin/sdl_release/src/core/cpu/subdir.mk
-include build/bin/sdl_release/src/core/subdir.mk
-include build/bin/sdl_release/src/subdir.mk
-include build/bin/sdl_debug/src/util/subdir.mk
-include build/bin/sdl_debug/src/sys/sdl/subdir.mk
-include build/bin/sdl_debug/src/sys/cli/subdir.mk
-include build/bin/sdl_debug/src/debug/subdir.mk
-include build/bin/sdl_debug/src/core/mem/mbc/subdir.mk
-include build/bin/sdl_debug/src/core/mem/io/lcd/subdir.mk
-include build/bin/sdl_debug/src/core/mem/io/subdir.mk
-include build/bin/sdl_debug/src/core/mem/subdir.mk
-include build/bin/sdl_debug/src/core/cpu/subdir.mk
-include build/bin/sdl_debug/src/core/subdir.mk
-include build/bin/sdl_debug/src/subdir.mk
-include build/bin/cli_release/src/util/subdir.mk
-include build/bin/cli_release/src/sys/cli/subdir.mk
-include build/bin/cli_release/src/debug/subdir.mk
-include build/bin/cli_release/src/core/mem/mbc/subdir.mk
-include build/bin/cli_release/src/core/mem/io/lcd/subdir.mk
-include build/bin/cli_release/src/core/mem/io/subdir.mk
-include build/bin/cli_release/src/core/mem/subdir.mk
-include build/bin/cli_release/src/core/io/lcd/subdir.mk
-include build/bin/cli_release/src/core/io/subdir.mk
-include build/bin/cli_release/src/core/etc/subdir.mk
-include build/bin/cli_release/src/core/cpu/subdir.mk
-include build/bin/cli_release/src/core/subdir.mk
-include build/bin/cli_release/src/subdir.mk
-include build/bin/cli_release/debug/subdir.mk
-include build/bin/cli_debug/src/util/subdir.mk
-include build/bin/cli_debug/src/sys/cli/subdir.mk
-include build/bin/cli_debug/src/debug/subdir.mk
-include build/bin/cli_debug/src/core/mem/mbc/subdir.mk
-include build/bin/cli_debug/src/core/mem/io/lcd/subdir.mk
-include build/bin/cli_debug/src/core/mem/io/subdir.mk
-include build/bin/cli_debug/src/core/mem/subdir.mk
-include build/bin/cli_debug/src/core/io/lcd/subdir.mk
-include build/bin/cli_debug/src/core/io/subdir.mk
-include build/bin/cli_debug/src/core/etc/subdir.mk
-include build/bin/cli_debug/src/core/cpu/subdir.mk
-include build/bin/cli_debug/src/core/subdir.mk
-include build/bin/cli_debug/src/subdir.mk
-include build/bin/cli_debug/debug/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: gbemu

# Tool invocations
gbemu: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc  -o "gbemu" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) gbemu
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
