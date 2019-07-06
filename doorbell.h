#ifndef __DOORBELL_H
#define __DOORBELL_H

#define GPIOCTL_DEV_MAJOR   		247
#define GPIOCTL_DEV_MINOR   		0
#define GPIOCTL_DEV_NAME    		"gpioctl"

#define SPRING_LOADED_BUTTON		GPIO(10)

#define LED_WHITE                   GPIO(11)
#define LED_RED                     GPIO(12)

/* IOCTL commands */
#define GPIOCTL_IOC_MAGIC           'k'

#define GPIOCTL_IOC_RD_BUTTON		_IOR(GPIOCTL_IOC_MAGIC, 1, long)
#define GPIOCTL_IOC_WR_LED_WHITE	_IOW(GPIOCTL_IOC_MAGIC, 2, long)
#define GPIOCTL_IOC_WR_LED_RED		_IOW(GPIOCTL_IOC_MAGIC, 3, long)

#define SERV_PORT                   13469
#define SERV_IP                     "test.ring.com"

#endif
