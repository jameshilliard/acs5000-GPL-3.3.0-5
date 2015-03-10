#ifndef UART_FRAME_MODE
#define UART_FRAME_MODE

#define FRAMEMODE_PAGESIZE     4096 //as for 08-Ago-2002
#define FRAMEMODE_OTHERUSE     512  // the original value was 8, change due to I2C driver
#define FRAMEMODE_TOTALBUFF    8

                            //FRAMEMODE_FRAMELEN cannot be greater than 512
#define FRAMEMODE_FRAMELEN 	((FRAMEMODE_PAGESIZE - FRAMEMODE_OTHERUSE) / FRAMEMODE_TOTALBUFF)
#define FRAMEMODE_IDLEWORDS	  3 //they last longer than an actual character !!!
                                //Modbus RTU framming requires 3,5 silent characters

                            //receive buffers are more critical...
#define FRAMEMODE_TXBUFFERS (FRAMEMODE_TOTALBUFF / 3)
#define FRAMEMODE_RXBUFFERS (FRAMEMODE_TOTALBUFF - FRAMEMODE_TXBUFFERS)

struct frame_mode_stats {
		int success;		// Number of frames correctly received
		int error;			// Number of frames received with errors
};

struct frame_mode_cfg {
		int status;							// 0 is off and 1 is on
		int mode;							// 0 for ascii framing and 1 for 
											// RTU framing
		uint txbuf;
		short txcount;
		short rxcount;
	    short frame_started;                // tell if there is any frame currently being received
		struct frame_mode_stats rx_stat;	// Statistic for rx frames
		struct frame_mode_stats tx_stat;	// Statistic for tx frames
};

#endif
