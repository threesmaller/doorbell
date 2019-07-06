#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "doorbell.h"

int GetState;
int fd_socket;
uint32_t send_data = 0;
uint32_t send_count = 0;
uint32_t recv_count = 0;
uint32_t check_socket = 0;

//############################################################################
//
//	open_socket
//
//############################################################################
int open_socket()
{
	struct hostent *hp;
    struct sockaddr_in clientaddr;
	struct sockaddr_in servaddr;
	
	if((fd_socket = socket(AF_INET, SOCK_DGRAM, 0)) <0) 
	{
		fprintf(stderr, "can't open socket device");
		return -1;
    }
	fcntl(fd_socket, F_SETFL, O_NONBLOCK);
	
	memset (&clientaddr, 0, sizeof (clientaddr)) ;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientaddr.sin_port = htons(0);
	
	if(bind(fd_socket, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) <0) 
	{
		fprintf(stderr, "bind failed");
		return -1;
    }

	memset (&servaddr, 0, sizeof (servaddr)) ;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	hp = gethostbyname(SERV_IP);
	servaddr.sin_addr = *((struct in_addr *)hp->h_addr);
	return 0;
}

//############################################################################
//
//	update_GetState
//
//############################################################################
int update_GetState(int fd, int GetState)
{	
	if(ioctl(fd, GPIOCTL_IOC_RD_BUTTON, &GetState) < 0)
		return -1;
	if(GetState == 0)		//button release
	{
		send_count = 0;
		send_data = 0;
		check_socket = 0;
		close(fd_socket);
		if(ioctl(fd, GPIOCTL_IOC_WR_LED_WHITE, 0) < 0)	//LED WHITE off
		{
			fprintf(stderr, "GPIOCTL_IOC_WR_LED_WHITE failed");
			return -1;					
		}
		else
			return 0;
		
	}
	else
	{
		char send_msg[2];
		char buf[2];
		unsigned int temp;
		int ret;
		if(check_socket == 0)
		{
			check_socket++;
			ret = open_socket();
			if(ret < 0)
				return -1;
		}
			
		sprintf(send_msg,"%d",send_count);
		if( send_data == 0 && recv_count == 0)
		{
			if(send(fd_socket, send_msg, sizeof(send_count), 0) < 0)
			{
				fprintf(stderr, "send failed");
				return -1;						
			}
			else
				send_data = 1;
		}
		else
		{
			if(recv(fd_socket, buf, sizeof(buf), 0) < 0)
			{
				fprintf(stderr, "send failed");
				return -1;
			}
			temp = atoi(buf);
			if(temp == send_count)
			{
				send_count++;
				send_data = 0;
			}
			else
			{
				recv_count++;
				if(recv_count == 3)
				{
					recv_count = 0;
					send_data = 0;
				}
			}
		}
	}
	return 0;
}
int main(int argc, char **argv)
{
    int fd_adc;
	int fd_gpioctl;
	
	uint32_t check_v = 0;
	
	
	if((fd_adc = open("/dev/adc", O_RDWR)) < 0) 
	{
    	fprintf(stderr, "can't open ADC device");
		return -1;
    }
	
	if((fd_gpioctl = open("/dev/gpioctl", O_RDWR)) < 0) 
	{
    	fprintf(stderr, "can't open GPIO ctrl device");
		return -1;
    }
				
    while(1)
    {
        int data;		
		
        read(fd_adc, &data, sizeof(data));

		if(data > 3500)		//voltage is > 3.5V
		{
			if(update_GetState(fd_gpioctl, GetState) < 0)
			{
				fprintf(stderr, "GPIOCTL_IOC_RD_BUTTON failed");
				return -1;					
			}	
		}
		else
		{
			if(check_v % 4 == 0)
			{
				if(ioctl(fd_gpioctl, GPIOCTL_IOC_WR_LED_RED, 1) < 0)	//LED RED on
				{
					fprintf(stderr, "GPIOCTL_IOC_WR_LED_RED failed");
					return -1;					
				}
			}
			else
			{
				if(ioctl(fd_gpioctl, GPIOCTL_IOC_WR_LED_RED, 0) < 0)	//LED RED off
				{
					fprintf(stderr, "GPIOCTL_IOC_WR_LED_RED failed");
					return -1;					
				}					
			}
			check_v++;
		}

		if( check_v > 0 )
			check_v = check_v % 4;
		usleep(125000);
    }

    close(fd_adc);
    close(fd_gpioctl);
	close(fd_socket);
    return 0;
}
