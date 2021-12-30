/*
 * mem_rw.c
 *
 *  Created on: Mar 15, 2021
 *      Author: Kostya A.
 *
 *	Description: memory read write functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

int mem_read(int addr, int *value)
{
	int fd, ret=0;

	unsigned page_addr, page_offset;
	void *ptr;
	unsigned page_size=sysconf(_SC_PAGESIZE);

	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		printf("%s @ line %d: Error opening /dev/mem \n", __func__, __LINE__);
		return 1;
	}

	/* mmap the device into memory */
	page_addr = (addr & (~(page_size-1)));
	page_offset = addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);


	/* Read value from the device register */
	*value = *((unsigned *)(ptr + page_offset));

	if (munmap(ptr, page_size) < 0) {
		printf("%s @ line %d: Error deallocate memory \n", __func__, __LINE__);
		ret=2;
		}

	close(fd);

	return ret;
}

int mem_write(int addr, int value)
{
	int fd, ret=0;

	unsigned page_addr, page_offset;
	void *ptr;
	unsigned page_size=sysconf(_SC_PAGESIZE);

	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		printf("%s @ line %d: Error opening /dev/mem \n", __func__, __LINE__);
		return 1;
	}

	/* mmap the device into memory */
	page_addr = (addr & (~(page_size-1)));
	page_offset = addr - page_addr;
	ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);

	/* write value to the device register */
	*((unsigned *)(ptr + page_offset)) = value;

	if (munmap(ptr, page_size) < 0) {
		printf("%s @ line %d: Error deallocate memory \n", __func__, __LINE__);
		ret=2;
	}

	close(fd);

	return ret;
}
