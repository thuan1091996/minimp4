/*
 * g711.c
 *
 *  Created on: Jul 8, 2022
 *      Author: phongdv
 */
#define ZEROTRAP                /* turn on the trap as per the MIL-STD */
#define BIAS 0x84               /* define the add-in bias for 16 bit samples */
#define CLIP 32635

unsigned char linear2ulaw(short sample)
{
    int sign, exponent, mantissa;
    unsigned char ulawbyte;
    static int exp_lut[256] =
        { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
    };

    /* Get the sample into sign-magnitude. */
    sign = (sample >> 8) & 0x80;        /* set aside the sign */
    if (sign != 0)
    {
        sample = -sample;       /* get magnitude */
    }
    if (sample > CLIP)
    {
        sample = CLIP;          /* clip the magnitude */
    }

    /* Convert from 16 bit linear to ulaw. */
    sample = sample + BIAS;
    exponent = exp_lut[(sample >> 7) & 0xFF];
    mantissa = (sample >> (exponent + 3)) & 0x0F;
    ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
    if (ulawbyte == 0)
    {
        ulawbyte = 0x02;        /* optional CCITT trap */
    }
#endif

    return ulawbyte;
}

int ulaw2linear(unsigned char ulawbyte)
{
    static int exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
    int sign, exponent, mantissa, sample;

    ulawbyte = ~ulawbyte;
    sign = (ulawbyte & 0x80);
    exponent = (ulawbyte >> 4) & 0x07;
    mantissa = ulawbyte & 0x0F;
    sample = exp_lut[exponent] + (mantissa << (exponent + 3));
    if (sign != 0)
    {
        sample = -sample;
    }

    return sample;
}

int convert_pcm_buf_2_ulaw_buf(short *in_buf, unsigned char *out_buf, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		*(out_buf + i) = linear2ulaw(*(in_buf + i));
	}
	return 0;
}

int convert_ulaw_buf_2_pcm_buf(unsigned char *in_buf, short *out_buf, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		*(out_buf + i) = ulaw2linear(*(in_buf + i));
	}
	return 0;
}


