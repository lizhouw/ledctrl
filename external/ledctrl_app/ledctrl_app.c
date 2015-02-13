#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_NAME "/dev/ledctrl_dev"


int main(int argc, char** argv)
{
    int   fd = -1;
    char  led_status = 0;
    int   i;

    fd = open(DEVICE_NAME, O_RDWR);
    if(-1 == fd){
        printf("Fail to open device %s.\n", DEVICE_NAME);
        return -1;
    }

    for( i = 0 ; i < 1000 ; i++ ){
        read(fd, &led_status, 1);
        printf("The LED is %s\n", led_status ? "ON" : "OFF");
        sleep(1);

        led_status = !led_status;
        write(fd, &led_status, 1);
        printf("Let LED %s\n", led_status ? "ON" : "OFF");
    }

    close(fd);
    return 0;
}

