
#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "printd.h"
#include "xmodem.h"

#define XMDM_SOH     0x01  /**< Start of heading */
#define XMDM_EOT     0x04  /**< End of text */
#define XMDM_ACK     0x06  /**< Acknowledge  */
#define XMDM_NAK     0x15  /**< negative acknowledge */
#define XMDM_CAN     0x18  /**< Cancel */
#define XMDM_ESC     0x1b  /**< Escape */
#define CRC16POLY  0x1021  /**< CRC 16 polynom */
#define PKTLEN_128    128  /**< Packet length */


static int8_t error;



static uint8_t xmodem_GetChar(int (*rx)(uint8_t *c), int block)
{
	int r;
	uint8_t c;
	volatile int n = 0;

	do {
		r = rx(&c);
		if(r) return(c);
		if(n++ == 10000000) {
			error = 1;
			return 0;
		}
	} while(block);

	return 0;
}


static uint16_t xmodem_GetCrc(int8_t uc, uint16_t crc)
{
	uint16_t uwCmpt;
	crc = crc ^ (int32_t) uc << 8;
	for (uwCmpt= 0; uwCmpt < 8; uwCmpt++)
	{
		if (crc & 0x8000)
			crc = crc << 1 ^ CRC16POLY;
		else
			crc = crc << 1;
	}
	return (crc & 0xFFFF);
}


static uint16_t xmodem_Getbytes(int (*rx)(uint8_t *c), uint8_t *data, uint32_t dwLength)
{
	uint16_t crc = 0;
	uint32_t dwCpt;
	uint8_t c;

	for (dwCpt = 0; dwCpt < dwLength; ++dwCpt)
	{
		c = xmodem_GetChar(rx, 1);
		if (error)
			return 1;
		crc = xmodem_GetCrc(c, crc);
		*data++ = c;
	}
	return crc;
}


static int32_t xmodem_GetPacket(xmodem_rxfn rxfn, uint8_t sno, void (*tx)(uint8_t c), int (*rx)(uint8_t *c))
{
	uint8_t seq[2];
	uint16_t crc, crc2;
	xmodem_Getbytes(rx, seq, 2);
	static uint8_t data[PKTLEN_128];

	crc2 = xmodem_Getbytes(rx, data, PKTLEN_128);
	if(error) return (-1);

	crc  = (unsigned short)xmodem_GetChar(rx, 1) << 8;
	if(error == 1) return (-1);
	crc += (unsigned short)xmodem_GetChar(rx, 1);
	if(error == 1) return (-1);

	if ((crc != crc2) || (seq[0] != sno) || (seq[1] != (uint8_t) ((~(uint32_t)sno)&0xff)))
	{
		tx(XMDM_CAN);
		return(-1);
	}

	rxfn(data, sizeof(data));
	return(0);
}


size_t xmodem_receive(xmodem_rxfn rxfn, void (*tx)(uint8_t c), int (*rx)(uint8_t *c))
{
	uint8_t c = 0;
	int32_t done;
	uint8_t sno = 0x01;
	size_t len = 0;

	/* Wait and put 'C' till start xmodem transfer */
		
	volatile int n = 0;

	while(c == 0)
	{
		if((n++ % 500000) == 0) {
			tx('C');
		}
		
		c = xmodem_GetChar(rx, 0);
	}

	/* Begin to receive the data */

	error = 0;
	done = 0;

	while(done == 0) {

		switch(c) 
		{   

			case XMDM_SOH:
				done = xmodem_GetPacket(rxfn, sno, tx, rx);

				if(error) {
					return 0;
				}

				if (done == 0) {
					sno++;
				}

				tx(XMDM_ACK);
				break;

			case XMDM_EOT:
				tx(XMDM_ACK);
				done = len;
				break;

			case XMDM_CAN:
			case XMDM_ESC:
			default:
				done = -1;
				break;
		}

		c = (int8_t)xmodem_GetChar(rx, 1);

		if(error) return 0;
	}

	return len;
}


/*
 * End
 */

