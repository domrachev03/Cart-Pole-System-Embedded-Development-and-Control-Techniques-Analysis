#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

unsigned int temperature = 0;
unsigned int torque = 0;
unsigned int speed = 0;
unsigned int position = 0;

canid_t _can_id;
__u8 _can_dlc; 

int sendctl(int s, struct can_frame *command) {
	if (write(s, command, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            return 1;
        }
	struct can_frame reply;
	while(read(s, (void *)&reply, sizeof(reply)) < 0) {}
	if (reply.data[0] != command->data[0]) {
		return 1;
	}
	temperature = reply.data[1];
	torque = (reply.data[3] << 8) + reply.data[2];
	speed = (reply.data[5] << 8) + reply.data[4];
	position = (reply.data[7] << 8) + reply.data[6];

	return 0;
}

int set_speed(int s, unsigned int speed) {
	struct can_frame command;
	command.can_id = _can_id;
	command.can_dlc = _can_dlc;

	unsigned char buffer[4];
	
	buffer[0] = (speed >> 24) & 0xFF;
	buffer[1] = (speed >> 16) & 0xFF;
	buffer[2] = (speed >> 8) & 0xFF;
	buffer[3] = speed & 0xFF;
	
	unsigned char cmd[] = {0xA2, 0x00, 0x00, 0x00, buffer[3], buffer[2], buffer[1], buffer[0]};
	memcpy(command.data, &cmd, command.can_dlc);

	return sendctl(s, &command);
}

int init_socket(char* device_name, canid_t can_id, __u8 can_dlc){
	int s;

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		return 1;
	}
	struct ifreq ifr;
	strcpy(ifr.ifr_name, "slcan0");
	ioctl(s, SIOCGIFINDEX, &ifr);

	struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		return 1;
	}
	
	_can_id = can_id;
	_can_dlc = can_dlc;

	return s;
}

int turn_off(int s) {
	struct can_frame command;
	command.can_id = _can_id;
	command.can_dlc = _can_dlc;
	unsigned char cmd[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(command.data, &cmd, command.can_dlc);
	
	if (write(s, &command, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		return 1;	
	}
	struct can_frame reply;
	while(read(s, (void *)&reply, sizeof(reply)) < 0) {}

	return reply.data[0] != cmd[0];
}

int main(int argc, char **argv)
{

	int s = init_socket("slcan0", 0x141, 8);
	
	if (set_speed(s, 65536) != 0) {
		return 1;
	}

   	printf("Temperature: %d\nTorque: %d\nVelocity: %d\nPosition: %d\n", temperature, torque, speed, position); 	
	sleep(5);	
	if (turn_off(s) != 0) {
		perror("Stop motor failed!");
		return 1;
	}
	
	if (close(s) < 0) {
		perror("Close");
		return 1;
	}

	return 0;
}
