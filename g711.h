/*
 * g711.h
 *
 *  Created on: Jul 8, 2022
 *      Author: phongdv
 */

#ifndef G711_H_
#define G711_H_

int linear2ulaw(int	pcm_val);
int ulaw2linear(unsigned char ulawbyte);
int convert_pcm_buf_2_ulaw_buf(short *in_buf, unsigned char *out_buf, int size);
int convert_ulaw_buf_2_pcm_buf(unsigned char *in_buf, short *out_buf, int size);

#endif /* G711_H_ */
