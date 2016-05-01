/**
 * @file   driver_test.c
 * @author Marcos Canales Mayo
 * @date   1 May 2016
 * @version 1.0
 */
#include <stdio.h>

int main(int argc, char* argv[]){
	int fd;
	char buf[256];
	fd = fopen("/dev/gps", "r");
	fread(buf, 1, 256, fd);
	printf("%s", buf);
}