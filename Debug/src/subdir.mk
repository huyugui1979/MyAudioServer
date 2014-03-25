################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/audio_client.cpp \
../src/audio_hall.cpp \
../src/audio_log.cpp \
../src/audio_player.cpp \
../src/audio_room.cpp \
../src/audio_server.cpp \
../src/main.cpp \
../src/message_center.cpp 

OBJS += \
./src/audio_client.o \
./src/audio_hall.o \
./src/audio_log.o \
./src/audio_player.o \
./src/audio_room.o \
./src/audio_server.o \
./src/main.o \
./src/message_center.o 

CPP_DEPS += \
./src/audio_client.d \
./src/audio_hall.d \
./src/audio_log.d \
./src/audio_player.d \
./src/audio_room.d \
./src/audio_server.d \
./src/main.d \
./src/message_center.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


