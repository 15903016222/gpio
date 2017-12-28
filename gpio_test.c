#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "gpio_test.h"

int main(int argc,char **argv) {
	int fd  = -1;
	int tmp = 0 ;
	int res = 0 ;

	fd = open (GPIO_NAME, O_RDWR);
	if (fd < 0) {
		perror ("open");
		return -1;
	}

	printf ("Press enter to set EIM_EB0 high:");
	getchar ();
	res = ioctl (fd, EIM_EB0_HIGH, &tmp);

	printf ("Press enter to set EIM_EB0 low:");
	getchar ();
	res = ioctl (fd, EIM_EB0_LOW, &tmp);

	printf ("Press enter to set EIM_EB1 high:");
	getchar ();
	res = ioctl (fd, EIM_EB1_HIGH, &tmp);
	printf ("Press enter to set EIM_EB1 low:");
	getchar ();
	res = ioctl (fd, EIM_EB1_LOW, &tmp);
	
	close (fd);
	return 0;
}
