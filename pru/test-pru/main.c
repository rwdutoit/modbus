#include <stdint.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "resource_table_1.h"

#include <pru_ctrl.h>

volatile register uint32_t __R30;
volatile register uint32_t __R31;
/* Host-1 Interrupt sets bit 31 in register R31 */
#define HOST_INT			((uint32_t) 1 << 31)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19

/*
 * Using the name 'rpmsg-client-sample' will probe the RPMsg sample driver
 * found at linux-x.y.z/samples/rpmsg/rpmsg_client_sample.c
 *
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
//#define CHAN_NAME			"rpmsg-pru"

#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

#define OUT44_BIT               3
#define OUT43_BIT               2

#define DATA_BIT               0
#define CLK_BIT                1
#define PRU_OCP_RATE_10MS      (200 * 1000 * 10)

char payload[RPMSG_BUF_SIZE];

/*
 * main.c
 */
void main(void)
{
	struct pru_rpmsg_transport transport;
	uint16_t src, dst, len;
	volatile uint16_t counter;
	volatile uint8_t *status;

volatile uint8_t dsc_data,dsc_clk, timeout,clk_high,i;
i=0;
clk_high=0;
counter=0;

	/* Allow OCP master port access by the PRU so the PRU can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	/* Make sure the Linux drivers are ready for RPMsg communication */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

	/* Initialize the RPMsg transport structure */
	pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

	/* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);
	while (1) {
		/* Check bit 30 of register R31 to see if the ARM has kicked us */
		if (__R31 & HOST_INT) {
			/* Clear the event status */
			CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
			/* Receive all available messages, multiple messages can be sent per kick */
			while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
uint8_t out =0;
if(payload[0]=='1')
{
out = 1;
	__R30 = (__R30 | (1<<OUT44_BIT) );
	__R30 = (__R30 | (1<<OUT43_BIT) );
}
else
{
out = 0;
        __R30 = (__R30 & ~(1<<OUT44_BIT) );
        __R30 = (__R30 & ~(1<<OUT43_BIT) );
}

        payload[counter++] = 'R';
        payload[counter++] = '3';
        payload[counter++] = '1';
        payload[counter++] = '=';
payload[counter++] = '0';
payload[counter++] = 'b';
			for(i=0;i<32;i++)
			{
                        dsc_data = (__R31 & (1u << i));
                        if(!(dsc_data > 0))//inverse for inputs
			//if ((__R31 & dsc_data) > 0)
                                payload[counter++] = '0';//48; //0
                        else
                                payload[counter++] = '1';//49; //1
			if(i==3 || i==7 || i==11 || i==15)
				payload[counter++] = ' ';//32; //' '
			}
        payload[counter++] = '\n';//10; //LF
        payload[counter++] = '\r';//13; //CR

        payload[counter++] = '4';
        payload[counter++] = '5';
        payload[counter++] = '=';
			dsc_data = (__R31 & (1u << DATA_BIT));
			if(!(dsc_data > 0))//inverse for inputs
                        //if ((__R31 & dsc_data) > 0)
                                payload[counter++] = '0';//48; //0
                        else
                                payload[counter++] = '1';//49; //1
	payload[counter++] = ' ';
        payload[counter++] = '4';
        payload[counter++] = '6';
        payload[counter++] = '=';
                        dsc_clk = (__R31 & (1u << CLK_BIT));
                        if(!(dsc_clk > 0)) //inverse for inputs
                        //if ((__R31 & dsc_data) > 0)
                                payload[counter++] = '0';//48; //0
                        else
                                payload[counter++] = '1';//49; //1
        payload[counter++] = '\n';//10; //LF
        payload[counter++] = '\r';//13; //CR
        payload[counter++] = 'o';
        payload[counter++] = 'u';
        payload[counter++] = 't';
        payload[counter++] = '=';
        payload[counter++] = 48 + out;
				/* Echo the message back to the same address from which we just received */
				pru_rpmsg_send(&transport, dst, src, payload, counter);
			}
		}
	}
}