#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>


int main(int argc, char **argv)
{
	printf("CAN Sockets Demo\r\n");

	int s; 
	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Socket");
		return 1;
	}

    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);
    

    struct sockaddr_can addr;
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return 1;
	}

    struct can_frame command;
    command.can_id = 0x141;
    command.can_dlc = 8;
    unsigned char cmd[] = {0xA5, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00};
    memcpy(&command.data, &cmd, command.can_dlc);
    
    struct can_frame rcv;
    
    int i = 0;
    while (i < 100000) {
        if (write(s, &command, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("Error in sending the command");
            return 1;
        }
    }

    
    // while (read (s, (void *)&rcv, sizeof(rcv)) < 0) {}
    // sleep(10);
    // unsigned int rotation = 0;
    
    // rotation += (rcv.data[7] << 8) + rcv.data[6];
    
    // printf("Current rotation is: %u\n", rotation);
    // printf("Bytes are: %02X %02X \n", rcv.data[6], rcv.data[7]);
    

	if (close(s) < 0) {
		perror("Close");
		return 1;
	}

	return 0;
}