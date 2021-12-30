/*
 * mem_rw.h
 *
 *  Created on: Mar 15, 2021
 *      Author: ubuntu
 */

#ifndef SRC_MEM_RW_H_
#define SRC_MEM_RW_H_

int mem_read(int addr, int *value);
int mem_write(int addr, int value);

#endif /* SRC_MEM_RW_H_ */
