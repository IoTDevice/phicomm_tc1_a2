/*
 * spi_eflash.c
 *
 *  Author: 
 */ 


#include "hsf.h"
#include "spi_eflash.h"


#define Min(a, b)           (((a) < (b)) ?  (a) : (b))


static void spi_cs_low(void)
{
	hfspi_cs_low();
}

static void spi_cs_high(void)
{
	hfspi_cs_high();
}

/**
 * \brief Initialize SPI as master.
 */
void at25_init_spi(void)
{
	/* Configure an SPI peripheral. */
	spi_cs_high();
	hfspi_master_init(0, 12000000);
}

/**
 * \brief Send command to dataflash.
 * \return True for OK.
 */
static bool spi_send_cmd(spi_cmd_t *p_cmd)
{
	int i;
	
	extern void core_util_critical_section_enter(void);
	extern void core_util_critical_section_exit(void);
	
	/* Wait until no activity on bus */
	core_util_critical_section_enter();
	spi_cs_low();
	
	/* Send command */
	hfspi_master_send_data(p_cmd->cmd, p_cmd->cmd_size);
	
	/* Send dummy clocks */
	unsigned char dummy[1]={0xFF};
	for (i = 0; i < p_cmd->dummy_size; i ++) {
		hfspi_master_send_data(dummy, 1);
	}
	
	if (p_cmd->data_size == 0)
	{
		spi_cs_high();
		core_util_critical_section_exit();
		return true;
	}

	/* Read/Write data */
	if (p_cmd->cmd_rx)
		hfspi_master_recv_data(p_cmd->data, p_cmd->data_size);
	else
		hfspi_master_send_data(p_cmd->data, p_cmd->data_size);
	
	spi_cs_high();
	core_util_critical_section_exit();
	return true;
}


/**
 * \brief Read the ID of serial flash.
 * \param p_id Pointer to fill ID result (word of 4 bytes).
 * \return True if OK.
 */
bool at25_read_id(uint32_t *p_id)
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_READ_JEDEC_ID,
	};
	
	spi_cmd_t  spi_cmd = {
		.data = (uint8_t*)p_id,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 1,
		.dummy_size = 0,
		.data_size = 4
	};
	
	return spi_send_cmd(&spi_cmd);
}


/**
 * \brief Read status register of serial flash.
 * \param p_status Pointer to fill status result.
 * \return True if OK.
 */
static bool at25_read_status(uint8_t *p_status)
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_READ_STATUS,
	};

	spi_cmd_t  spi_cmd = {
		.data = p_status,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 1,
		.dummy_size = 0,
		.data_size = 1
	};

	return spi_send_cmd(&spi_cmd);
}

static bool at25_read_status_highbit(uint8_t *p_status)
{
	at25_cmd_t at25_cmd = {
		.op_code = 0x35,
	};

	spi_cmd_t  spi_cmd = {
		.data = p_status,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 1,
		.dummy_size = 0,
		.data_size = 1
	};

	return spi_send_cmd(&spi_cmd);
}


/**
 * \brief disable serial flash write.
 * \return True if OK.
 */
static bool at25_disable_write(void)
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_WRITE_DISABLE,
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 0
	};
	
	return spi_send_cmd(&spi_cmd);
}

/**
 * \brief Enable serial flash write.
 * \return True if OK.
 */
static bool at25_enable_write(void)
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_WRITE_ENABLE,
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 0
	};
	
	return spi_send_cmd(&spi_cmd);
}


static bool at25_wait(uint8_t *p_status)
{
	int i = 0;
	uint8_t status = AT25_STATUS_RDYBSY;
	
	while (status & AT25_STATUS_RDYBSY) {
		if (at25_read_status(&status) == false)
			return false;
		msleep(1);

		if(i++ > 1000)//10 s
			return false;
	}

	if (p_status)
		*p_status = status;
	return true;
}

/**
 * \brief Enable qual SPI.
 * \return True if OK.
 */
bool at25_quad_enable(void)
{
	bool rc;
	uint8_t status[2];
	
	if (at25_read_status(&status[0]) == false)
		return false;

	if (at25_read_status_highbit(&status[1]) == false)
		return false;
		
	at25_cmd_t at25_cmd = {
		.op_code = AT25_WRITE_STATUS
	};

	status[1] = ((status[1]&0xFD)|0x02); //enable
		
	spi_cmd_t  spi_cmd = {
		.data = status,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 2
	};
	
	rc = at25_enable_write();
	if (!rc)
		return false;

	rc = spi_send_cmd(&spi_cmd);
	if (!rc)
		return false;

	rc = at25_wait(&status[0]);
	if (!rc)
		return false;
	if (status[0] & AT25_STATUS_EPE)
		return false;
	
	rc = at25_disable_write();
	if (!rc)
		return false;

	msleep(20);
	return true;
}

/**
 * \brief Erase 64K block.
 * \param address Serial flash internal address.
 * \return True if OK.
 */
bool at25_erase_block_64k(uint32_t address)
{
	bool rc;
	uint8_t status;
	
	/*protect the last 64K for attestation*/
	if (address > DATAFLASH_TOTAL_SIZE-0x10000) {
		return false;
	}
	
	at25_cmd_t at25_cmd = {
		.op_code = AT25_BLOCK_ERASE_64K
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 4,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 0
	};
	
	at25_cmd.address_h = (address & 0xFF0000) >> 16;
	at25_cmd.address_m = (address & 0x00FF00) >>  8;
	at25_cmd.address_l = (address & 0x0000FF) >>  0;
	rc = at25_enable_write();
	if (!rc)
		return false;

	rc = spi_send_cmd(&spi_cmd);
	if (!rc)
		return false;

	rc = at25_wait(&status);
	if (!rc)
		return false;
	if (status & AT25_STATUS_EPE)
		return false;
	
	rc = at25_disable_write();
	if (!rc)
		return false;

	//vTaskDelay(20/portTICK_RATE_MS);
	msleep(20);
	return true;
}


/**
 * \brief Erase 4K block.
 * \param address Serial flash internal address.
 * \return True if OK.
 */
bool at25_erase_block_4k(uint32_t address)
{
	bool rc;
	uint8_t status;

	/*protect the last 4K for attestation*/
	if (address > DATAFLASH_TOTAL_SIZE-0x1000) {
		return false;
	}
	at25_cmd_t at25_cmd = {
		.op_code = AT25_BLOCK_ERASE_4K
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 4,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 0
	};
	
	at25_cmd.address_h = (address & 0xFF0000) >> 16;
	at25_cmd.address_m = (address & 0x00FF00) >>  8;
	at25_cmd.address_l = (address & 0x0000FF) >>  0;
	rc = at25_enable_write();
	if (!rc)
		return false;

	rc = spi_send_cmd(&spi_cmd);
	if (!rc)
		return false;

	rc = at25_wait(&status);
	if (!rc)
		return false;
	if (status & AT25_STATUS_EPE)
		return false;
	
	rc = at25_disable_write();
	if (!rc)
		return false;

	msleep(20);
	return true;
}

/**
 * \brief Read from serial flash.
 * \param address Serial flash internal address.
 * \param p_buf Pointer to data buffer to fill.
 * \param size Data buffer size.
 * \return True if OK.
 */
bool at25_read(uint32_t address, uint8_t *p_buf, uint32_t size)
{
	bool rc;
	//vTaskSuspendAll();
	at25_cmd_t at25_cmd = {
		.op_code = AT25_READ_ARRAY
	};

	if ((size + address) > DATAFLASH_TOTAL_SIZE) {
		return false;
	}
	spi_cmd_t  spi_cmd = { 
		.data = p_buf,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 4,
		.cmd_rx = 1,
		.dummy_size = 1,
		.data_size = size
	};
	
	at25_cmd.address_h = (address & 0xFF0000) >> 16;
	at25_cmd.address_m = (address & 0x00FF00) >>  8;
	at25_cmd.address_l = (address & 0x0000FF) >>  0;

	rc = spi_send_cmd(&spi_cmd);
	if (!rc)
		return false;
	return true;
}



/**
 * \brief Write to serial flash.
 * \param address Serial flash internal address.
 * \param p_buf Pointer to data buffer to send.
 * \param size Data buffer size.
 * \return True if OK.
 */
bool at25_write(uint32_t address, uint8_t *p_buf, uint32_t size)
{
	bool rc;
	uint8_t status;
	uint32_t write_size;
	
	/*protect the last 4K for attestation*/
	if ( ((size + address) > DATAFLASH_TOTAL_SIZE)||((size + address) <0)) {
		return false;
	}
	
	while (size > 0) {
		write_size = Min(size,(DATAFLASH_PAGE_SIZE - (address % DATAFLASH_PAGE_SIZE)));
	
		at25_cmd_t at25_cmd = {
			.op_code = AT25_BYTE_PAGE_PROGRAM
		};
	
		spi_cmd_t  spi_cmd = {
			.data = p_buf,
			.cmd  = (uint8_t*)&at25_cmd,
			.cmd_size = 4,
			.cmd_rx = 0,
			.dummy_size = 0,
			.data_size = write_size
		};
	
		at25_cmd.address_h = (address & 0xFF0000) >> 16;
		at25_cmd.address_m = (address & 0x00FF00) >>  8;
		at25_cmd.address_l = (address & 0x0000FF) >>  0;
		rc = at25_enable_write();
	
		if (!rc)
			return false;

		rc = spi_send_cmd(&spi_cmd);
		if (!rc)
			return false;

		rc = at25_wait(&status);
		if (!rc)
			return false;
		if (status & AT25_STATUS_EPE)
			return false;
		
		p_buf += write_size;
		size -= write_size;
		address += write_size;
	}

	msleep(20);
	return true;
}

/**
 * \brief Goto sleep mode.
 * \return True if OK.
 */
bool at25_sleep(void)
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_DEEP_PDOWN,
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 0,
		.dummy_size = 0,
		.data_size = 0
	};
	
	return spi_send_cmd(&spi_cmd);
}

/**
 * \brief wakeup SPi flash.
 * \return True if OK.
 */
 bool at25_wakeup()
{
	at25_cmd_t at25_cmd = {
		.op_code = AT25_RES_DEEP_PDOWN,
	};
	
	spi_cmd_t  spi_cmd = {
		.data = NULL,
		.cmd  = (uint8_t*)&at25_cmd,
		.cmd_size = 1,
		.cmd_rx = 1,
		.dummy_size = 3,
		.data_size = 0
	};
	
	return spi_send_cmd(&spi_cmd);
}

