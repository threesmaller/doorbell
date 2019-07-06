#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

int GetState;
int fd_socket; 

//############################################################################
//
//	set_red_led
//
//############################################################################
void set_red_led(void)
{
	ambarella_gpio_set(MCU_MODE, 1);
	ambarella_gpio_set(MCU_RESET, 1);
	msleep(100);
	ambarella_gpio_set(MCU_RESET, 0);
	msleep(100);
	ambarella_gpio_set(MCU_RESET, 1);
	msleep(100);
}

//############################################################################
//
//	open_socket
//
//############################################################################
void open_socket();
{
	struct hostent *hp;
    struct sockaddr_in clientaddr;
	struct sockaddr_in servaddr;
	
	if ((fd_socket = socket(AF_INET, SOCK_DGRAM, 0)) <0) 
	{
		printf(stderr, "can't open socket device");
		exit(EXIT_FAILURE);
    }
	
	memset (&myaddr, 0, sizeof (myaddr)) ;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientaddr.sin_port = htons(0);
	
	if (bind(fd_socket, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) <0) 
	{
		printf(stderr, "bind failed");
		exit(1);
    }

	memset (&servaddr, 0, sizeof (servaddr)) ;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	hp = gethostbyname(SERV_IP);
	servaddr.sin_addr = *((struct in_addr *)hp->h_addr);	
}

//############################################################################
//
//	update_GetState
//
//############################################################################
int update_GetState(int fd, int GetState)
{	
	if (ioctl(fd, GPIOCTL_IOC_RD_BUTTON, &GetState) < 0)
		return -1;
	if( GetState == 0 )		//button release
	{
		send_count = 0;
		send_data = 0;
		check_socket = 0;
		close(fd_socket);
		if(ioctl(fd_pca9536, GPIOCTL_IOC_WR_LED_WHITE, 0) < 0)	//LED WHITE off
		{
			printf(stderr, "GPIOCTL_IOC_WR_LED_WHITE failed");
			exit(1);						
		}
	}
	else
	{
		char send_msg[2];
		char buf[2];
		int temp;
		if(check_socket == 0)
		{
			check_socket++;
			open_socket();
		}
			
		sprintf(send_msg,"%d",send_count);
		if( send_data == 0 && recv_count = 0)
		{
			if(send(fd_socket, send_msg, sizeof(send_count), 0) < 0)
			{
				printf(stderr, "send failed");
				exit(1);						
			}
			else
				send_data = 1;
		}
		else
		{
			if(recv(fd_socket, buf, sizeof(buf), 0) < 0)
			{
				printf(stderr, "send failed");
				exit(1);
			}
			temp = atoi(buf);
			if( temp == send_count )
			{
				send_count++;
				send_data = 0;
			}
			else
			{
				recv_count++;
				if( recv_count == 3 )
				{
					recv_count = 0;
					send_data = 0;
				}
			}
		}
		
	}
}
int main(int argc, char **argv)
{
    int fd_adc;
	int fd_gpioctl;
	
	int	check_v = 0;
	int send_data = 0;
	int send_count = 0;
	int recv_count = 0;
	int check_socket = 0;
	
	if ((fd_adc = open("/dev/adc", O_RDWR)) < 0) 
	{
    	printf(stderr, "can't open ADC device");
		exit(1);
    }
	
	if ((fd_gpioctl = open("/dev/gpioctl", O_RDWR)) < 0) 
	{
    	printf(stderr, "can't open GPIO ctrl device");
		exit(1);
    }
	

						
    while(1)
    {
        int ret;
        int data;		
		
        ret = read(fd_adc, &data, sizeof(data));

        if(ret != sizeof(data))
        {
            if(errno != EAGAIN)
                printf("Read ADC Device Faild!\n");
            continue;
        }
        else
        {
            if(data > 3500)		//voltage is > 3.5V
			{
				if (update_GetState(fd_gpioctl, &GetState) < 0)
				{
					printf(stderr, "GPIOCTL_IOC_RD_BUTTON failed");
					exit(1);						
				}	
			}
			else
			{
				if(check_v % 4 == 0)
				{
					if(ioctl(fd_gpioctl, GPIOCTL_IOC_WR_LED_RED, 1) < 0)	//LED RED on
					{
						printf(stderr, "GPIOCTL_IOC_WR_LED_RED failed");
						exit(1);						
					}
				}
				else
				{
					if(ioctl(fd_gpioctl, GPIOCTL_IOC_WR_LED_RED, 0) < 0)	//LED RED off
					{
						printf(stderr, "GPIOCTL_IOC_WR_LED_RED failed");
						exit(1);						
					}					
				}
				check_v++;
			}
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
