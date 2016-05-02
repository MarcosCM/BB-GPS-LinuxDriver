/**
 * @file   driver_test.c
 * @author Marcos Canales Mayo
 * @date   1 May 2016
 * @version 1.0
 */

#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	int fd;
	char buf[8];
	fd = open("/dev/gps", O_RDONLY);
	read(fd, buf, 8);
	printf("%s\n", buf);
	close(fd);
}