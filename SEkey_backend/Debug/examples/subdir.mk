################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../examples/device_init.cpp \
../examples/digest_algorithms.cpp \
../examples/encryption_algorithms.cpp \
../examples/hello_world.cpp \
../examples/manual_key_management.cpp \
../examples/sefile_example.cpp \
../examples/sekey_example.cpp \
../examples/selink_example.cpp 

OBJS += \
./examples/device_init.o \
./examples/digest_algorithms.o \
./examples/encryption_algorithms.o \
./examples/hello_world.o \
./examples/manual_key_management.o \
./examples/sefile_example.o \
./examples/sekey_example.o \
./examples/selink_example.o 

CPP_DEPS += \
./examples/device_init.d \
./examples/digest_algorithms.d \
./examples/encryption_algorithms.d \
./examples/hello_world.d \
./examples/manual_key_management.d \
./examples/sefile_example.d \
./examples/sekey_example.d \
./examples/selink_example.d 


# Each subdirectory must supply rules for building sources it contributes
examples/%.o: ../examples/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -DSQLITE_TEMP_STORE=3 -std=c++17 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


