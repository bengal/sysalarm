################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../config.o \
../parse.o \
../sysalarm.o \
../util.o 

C_SRCS += \
../alarm_disk.c \
../alarm_tcp.c \
../config.c \
../old_config.c \
../parse.c \
../sysalarm.c \
../util.c 

OBJS += \
./alarm_disk.o \
./alarm_tcp.o \
./config.o \
./old_config.o \
./parse.o \
./sysalarm.o \
./util.o 

C_DEPS += \
./alarm_disk.d \
./alarm_tcp.d \
./config.d \
./old_config.d \
./parse.d \
./sysalarm.d \
./util.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


