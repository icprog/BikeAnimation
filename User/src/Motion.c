
#include "Motion.h"

void ACCE_MasterConfiguration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /* Time Base configuration */
	TIM_Cmd(ACCE_MONITOR_TIMER, DISABLE);
	TIM_ITConfig(ACCE_MONITOR_TIMER, TIM_IT_Update, DISABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = ACCE_MONITOR_TIMER_PRESCAL;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = ACCE_MONITOR_TIMER_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(ACCE_MONITOR_TIMER, &TIM_TimeBaseStructure);
	// Enable timer to start routine acce data read after device config

	/* SPI_MASTER configuration ------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	// 
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		// Master of course
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	// 8bit format
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		// High in idle
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	// Rising edge capture (TBD)
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	// 72M for APB2, 10M max reqirement for BMA020 SPI IF, so divide 8 = 9M for SPI CLK
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	// Accroding to BMA020's datasheet, first bit is MSB
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(ACCE_MASTER_SPI, &SPI_InitStructure);

	/* SPI_DMA_Channel configuration ---------------------------------------------*/
	// Here only focus on some FIXED parameter, other parameter will be adjusted according to different event
	// like send command or fill rectangle...
	DMA_DeInit(ACCE_MASTER_DMA_TX_CHN);
	staAcceMasterTX_DMAInitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ACCE_MASTER_SPI->DR));
	staAcceMasterTX_DMAInitStructure.DMA_MemoryBaseAddr = (uint32_t)staAcceTXBUF;
	staAcceMasterTX_DMAInitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	staAcceMasterTX_DMAInitStructure.DMA_BufferSize = ACCE_TXRX_BUF_LEN_MAX;
	staAcceMasterTX_DMAInitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	staAcceMasterTX_DMAInitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	staAcceMasterTX_DMAInitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	staAcceMasterTX_DMAInitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	staAcceMasterTX_DMAInitStructure.DMA_Mode = DMA_Mode_Normal;
	staAcceMasterTX_DMAInitStructure.DMA_Priority = DMA_Priority_Medium;
	staAcceMasterTX_DMAInitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(ACCE_MASTER_DMA_TX_CHN, &staAcceMasterTX_DMAInitStructure);
	DMA_ITConfig(ACCE_MASTER_DMA_TX_CHN, DMA_IT_TC, DISABLE);

	DMA_DeInit(ACCE_MASTER_DMA_RX_CHN);
	staAcceMasterRX_DMAInitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(ACCE_MASTER_SPI->DR));
	staAcceMasterRX_DMAInitStructure.DMA_MemoryBaseAddr = (uint32_t)staAcceRXBUF;
	staAcceMasterRX_DMAInitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	staAcceMasterRX_DMAInitStructure.DMA_BufferSize = ACCE_TXRX_BUF_LEN_MAX;
	staAcceMasterRX_DMAInitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	staAcceMasterRX_DMAInitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	staAcceMasterRX_DMAInitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	staAcceMasterRX_DMAInitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	staAcceMasterRX_DMAInitStructure.DMA_Mode = DMA_Mode_Normal;
	staAcceMasterRX_DMAInitStructure.DMA_Priority = DMA_Priority_Medium;
	staAcceMasterRX_DMAInitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(ACCE_MASTER_DMA_RX_CHN, &staAcceMasterRX_DMAInitStructure);
	DMA_ITConfig(ACCE_MASTER_DMA_RX_CHN, DMA_IT_TC, DISABLE);
	

	/* Enable SPI_MASTER NSS output for master mode */
	SPI_SSOutputCmd(ACCE_MASTER_SPI, ENABLE);

	/* Enable SPI_MASTER */
	SPI_Cmd(ACCE_MASTER_SPI, ENABLE);

	/* Enable Acce SPI's TX DMA */
	SPI_I2S_DMACmd(ACCE_MASTER_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_I2S_DMACmd(ACCE_MASTER_SPI, SPI_I2S_DMAReq_Rx, ENABLE);

	
	/* Enable DMA1 Channel3 */
	//DMA_Cmd(ACCE_MASTER_DMA_TX_CHN, ENABLE);
}

ENUM_OperationResult AcceReadRX_DMAInitial(uint8_t iBUFLength, uint8_t* pRecvContainer)
{
	DMA_Cmd(ACCE_MASTER_DMA_RX_CHN, DISABLE);
	DMA_ITConfig(ACCE_MASTER_DMA_RX_CHN, DMA_IT_TC, DISABLE);
	staAcceMasterRX_DMAInitStructure.DMA_MemoryBaseAddr = (uint32_t)pRecvContainer;
	staAcceMasterRX_DMAInitStructure.DMA_BufferSize = iBUFLength;
	DMA_Init(ACCE_MASTER_DMA_RX_CHN, &staAcceMasterRX_DMAInitStructure);
	return ACCEPTED;
}

ENUM_OperationResult AcceReadTX_DMAInitial(uint8_t iRegAddress, uint8_t iBUFLength)
{
	DMA_Cmd(ACCE_MASTER_DMA_TX_CHN, DISABLE);
	DMA_ITConfig(ACCE_MASTER_DMA_TX_CHN, DMA_IT_TC, DISABLE);
	staAcceTXBUF[0] = iRegAddress | MSB_MASK_8BIT;
	staAcceMasterRX_DMAInitStructure.DMA_MemoryBaseAddr = (uint32_t)staAcceTXBUF;
	staAcceMasterTX_DMAInitStructure.DMA_BufferSize = iBUFLength;
	DMA_Init(ACCE_MASTER_DMA_TX_CHN, &staAcceMasterTX_DMAInitStructure);
	return ACCEPTED;
}

ENUM_OperationResult AcceWriteTX_DMAInitial(uint8_t* pTXBuffer, uint8_t iBUFLength)
{
	DMA_Cmd(ACCE_MASTER_DMA_TX_CHN, DISABLE);
	DMA_Cmd(ACCE_MASTER_DMA_RX_CHN, DISABLE);
	DMA_ITConfig(ACCE_MASTER_DMA_TX_CHN, DMA_IT_TC, DISABLE);
	DMA_ITConfig(ACCE_MASTER_DMA_RX_CHN, DMA_IT_TC, DISABLE);
	staAcceMasterRX_DMAInitStructure.DMA_MemoryBaseAddr = (uint32_t)pTXBuffer;
	staAcceMasterTX_DMAInitStructure.DMA_BufferSize = iBUFLength;
	DMA_Init(ACCE_MASTER_DMA_TX_CHN, &staAcceMasterTX_DMAInitStructure);
	return ACCEPTED;
}

ENUM_OperationResult AcceRead(uint8_t iRegAddress, uint8_t iReadLength, uint8_t* pRecvContainer)
{
	AcceReadRX_DMAInitial(iReadLength + 1, pRecvContainer);
	AcceReadTX_DMAInitial(iRegAddress, iReadLength + 1);
	DMA_ITConfig(ACCE_MASTER_DMA_RX_CHN, DMA_IT_TC, ENABLE);
	ACCE_MASTER_NSS_ENABLE;
	FLAG_MOTION_ACCE_COMMU_BUSY = SET;
	// Clear RXNE
	SPI_I2S_ReceiveData(ACCE_MASTER_SPI);
	DMA_Cmd(ACCE_MASTER_DMA_RX_CHN, ENABLE);
	DMA_Cmd(ACCE_MASTER_DMA_TX_CHN, ENABLE);
	return ACCEPTED;
}

ENUM_OperationResult AcceWrite(uint8_t* pTXBuffer, uint8_t iWriteLength)
{	
	AcceWriteTX_DMAInitial(pTXBuffer, iWriteLength);
	DMA_ITConfig(ACCE_MASTER_DMA_TX_CHN, DMA_IT_TC, ENABLE);
	ACCE_MASTER_NSS_ENABLE;
	FLAG_MOTION_ACCE_COMMU_BUSY = SET;
	DMA_Cmd(ACCE_MASTER_DMA_TX_CHN, ENABLE);
	return ACCEPTED;
}

STR_AcceCommu* AcceQueueSlotSearch(STR_AcceCommu* pAcceCommuQueueHead, 
								  uint8_t iAcceCommuQueueLen, 
								  uint8_t iAcceCommuWorkingPos)
{
	uint8_t temp_QueueIndex;
	STR_AcceCommu* pAcceCommuCurrent = pAcceCommuQueueHead + iAcceCommuWorkingPos;
	if ((NULL == (*pAcceCommuCurrent).pRXBuf) &&
		(NULL == (*pAcceCommuCurrent).pTXBuf))
	{
		return pAcceCommuCurrent;
	}
	else
	{
		temp_QueueIndex = iAcceCommuWorkingPos + 1;
		while (temp_QueueIndex < iAcceCommuQueueLen)
		{
			pAcceCommuCurrent = pAcceCommuQueueHead + temp_QueueIndex;
			if ((NULL == (*pAcceCommuCurrent).pRXBuf) &&
				(NULL == (*pAcceCommuCurrent).pTXBuf))
			{
				return pAcceCommuCurrent;
			}
			temp_QueueIndex++;
		}
		temp_QueueIndex = 0;
		while (temp_QueueIndex < iAcceCommuWorkingPos)
		{
			pAcceCommuCurrent = pAcceCommuQueueHead + temp_QueueIndex;
			if ((NULL == (*pAcceCommuCurrent).pRXBuf) &&
				(NULL == (*pAcceCommuCurrent).pTXBuf))
			{
				return pAcceCommuCurrent;
			}
			temp_QueueIndex++;
		}
		return NULL;
	}
}

void AcceQueueSlotFree(STR_AcceCommu *pAcceQueueSlot)
{
	// Only free the space when they are in heap
	// May be const value like start up device config parameter stream
	if (NULL != pAcceQueueSlot->pRXBuf)
	{
		if (__IS_IN_HEAP(pAcceQueueSlot->pRXBuf))
		{
			free(pAcceQueueSlot->pRXBuf);
		}
		pAcceQueueSlot->pRXBuf = NULL;
	}
	if (NULL != pAcceQueueSlot->pTXBuf)
	{
		if (__IS_IN_HEAP(pAcceQueueSlot->pTXBuf))
		{
			free(pAcceQueueSlot->pTXBuf);
		}
		pAcceQueueSlot->pTXBuf = NULL;
	}
}

// Better to invoke AcceQueueSlotInitial() and HeapInitial() together
void AcceQueueSlotInitial(void)
{
	uint8_t tempIndex;
	staAcceCommuIndex = 0;	// Initial staAcceCommuIndex in acce commu queue to 0
	staAcceRecvRawDataWalker = 0;
	for (tempIndex = 0; tempIndex < ACCE_COMMU_QUEUE_LEN; tempIndex++)
	{
		(staAcceCommuQueue + tempIndex)->pRXBuf = NULL;
		(staAcceCommuQueue + tempIndex)->pTXBuf = NULL;
	}
}

void HeapInitial(void)
{
	memset((uint8_t *)HEAP_BASE_SCT, NULL, HEAP_SIZE);
}

void MotionStateInitial(void)
{
	staMotionDetectState = MOTION_DETECT_MASTER_CONF;
}

ENUM_OperationResult AcceMasterBlastTX_Const(const uint8_t* pFormattedCMDStream, uint8_t iFormattedCMDStreamLen)
{
	uint8_t iTempIndex = 0;
	STR_AcceCommu* pTempAvailableQueueSlot = NULL;
	while (iTempIndex < iFormattedCMDStreamLen)
	{
		if (2 > *(pFormattedCMDStream + iTempIndex))
		{
			return REFUSED;
		}
		pTempAvailableQueueSlot = AcceQueueSlotSearch(staAcceCommuQueue, sizeof(staAcceCommuQueue)/sizeof(staAcceCommuQueue[0]), staAcceCommuIndex);
		if (NULL == pTempAvailableQueueSlot)
		{
			return REFUSED;
		}
		pTempAvailableQueueSlot->pRXBuf = NULL;
		pTempAvailableQueueSlot->bCommuSuccess = FALSE;
		pTempAvailableQueueSlot->pTXBuf = (uint8_t *)(pFormattedCMDStream + iTempIndex + 1);
		pTempAvailableQueueSlot->iTXBufLen = *(pFormattedCMDStream + iTempIndex) - 1;
		iTempIndex += *(pFormattedCMDStream + iTempIndex);
	}
	return ACCEPTED;
}

void AcceDeviceConfig(void)
{
/* Like this, the first number of each line is the number of bytes
   (Include it self)
	const uint8_t BMA020_CONFIG_PARA[] = {
		6, 0x00, 0x01, 0x02, 0x03, 0x04,
		3, 0x05, 0x12,
		4, 0x04, 0x17, 0x28
	}
*/
	AcceMasterBlastTX_Const(BMA020_CONFIG_PARA, sizeof(BMA020_CONFIG_PARA));
	// Enable timer to start routine acce data read after device config
	TIM_Cmd(ACCE_MONITOR_TIMER, ENABLE);
	TIM_ITConfig(ACCE_MONITOR_TIMER, TIM_IT_Update, ENABLE);
	staMotionDetectState = MOTION_DETECT_TASK_DISPATCH;
}

ENUM_OperationResult MTN_NewRoutineDataReadRequest(void)
{
	uint8_t* pRecvContainer = NULL;
	STR_AcceCommu* pTempAvailableQueueSlot = NULL;

	pTempAvailableQueueSlot = AcceQueueSlotSearch(staAcceCommuQueue, sizeof(staAcceCommuQueue)/sizeof(staAcceCommuQueue[0]), staAcceCommuIndex);
	if (NULL == pTempAvailableQueueSlot)
	{
		return REFUSED;
	}
	// Get pRecvContainer second to avoid free the allocated space
	pRecvContainer = malloc(ACCE_ROUTINE_DATA_READ_LEN + 1);
	if (NULL == pRecvContainer)
	{
		return REFUSED;
	}
	pTempAvailableQueueSlot->pRXBuf = pRecvContainer;
	pTempAvailableQueueSlot->bCommuSuccess = FALSE;
	pTempAvailableQueueSlot->pTXBuf = NULL;
	pTempAvailableQueueSlot->iRXAddress = ACCE_ROUTINE_DATA_READ_ADDR;
	pTempAvailableQueueSlot->iRXBufLen = ACCE_ROUTINE_DATA_READ_LEN + 1;

	return ACCEPTED;
}

uint8_t AcceRawDataRecv(STR_AcceCommu strRecvRawDataSlot)
{
	if (TRUE == strRecvRawDataSlot.bCommuSuccess)
	{
		*(pStaAcceRecvDawDataBufX + staAcceRecvRawDataWalker) = (int16_t)GET_ACCE_RAW_DATA_FROM_BUF(strRecvRawDataSlot.pRXBuf + ACCE_RECV_RAW_DATA_X_POS);
		//staAcceRecvDawDataBufX[staAcceRecvRawDataWalker] = (int16_t)GET_ACCE_RAW_DATA_FROM_BUF(strRecvRawDataSlot.pRXBuf + ACCE_RECV_RAW_DATA_X_POS);
		//staAcceRecvDawDataBufY[staAcceRecvRawDataWalker] = (int16_t)GET_ACCE_RAW_DATA_FROM_BUF(strRecvRawDataSlot.pRXBuf + ACCE_RECV_RAW_DATA_Y_POS);
		//staAcceRecvDawDataBufZ[staAcceRecvRawDataWalker] = (int16_t)GET_ACCE_RAW_DATA_FROM_BUF(strRecvRawDataSlot.pRXBuf + ACCE_RECV_RAW_DATA_Z_POS);		
	}
	else
	{
		*(pStaAcceRecvDawDataBufX + staAcceRecvRawDataWalker) = (int16_t)GET_ACCE_RAW_DATA_FROM_BUF(strRecvRawDataSlot.pRXBuf + ACCE_RECV_RAW_DATA_X_POS);
		//staAcceRecvDawDataBufX[staAcceRecvRawDataWalker] = ACCE_ERROR_VALUE_INT16;
		//staAcceRecvDawDataBufY[staAcceRecvRawDataWalker] = ACCE_ERROR_VALUE_INT16;
		//staAcceRecvDawDataBufZ[staAcceRecvRawDataWalker] = ACCE_ERROR_VALUE_INT16;
	}
	LOOP_WALKER_FORWARD(staAcceRecvRawDataWalker, ACCE_RECV_RAW_DATA_BUF_LEN);
	return staAcceRecvRawDataWalker;
}

int16_t getAcceMaxRawData(int16_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	uint8_t tempI;
	int16_t iResult = *pInputRawDataArray;
	for (tempI = 1; tempI < iInputRawDataLen; tempI++)
	{
		if (iResult < *(pInputRawDataArray + tempI))
		{
			iResult = *(pInputRawDataArray + tempI);
		}
	}
	return iResult;
}

int8_t getInt8MaxData(int8_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	uint8_t tempI;
	int8_t iResult = *pInputRawDataArray;
	for (tempI = 1; tempI < iInputRawDataLen; tempI++)
	{
		if (iResult < *(pInputRawDataArray + tempI))
		{
			iResult = *(pInputRawDataArray + tempI);
		}
	}
	return iResult;
}

int16_t getAcceMinRawData(int16_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	uint8_t tempI;
	int16_t iResult = *pInputRawDataArray;
	for (tempI = 1; tempI < iInputRawDataLen; tempI++)
	{
		if (iResult > *(pInputRawDataArray + tempI))
		{
			iResult = *(pInputRawDataArray + tempI);
		}
	}
	return iResult;
}

int8_t getInt8MinData(int8_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	uint8_t tempI;
	int8_t iResult = *pInputRawDataArray;
	for (tempI = 1; tempI < iInputRawDataLen; tempI++)
	{
		if (iResult > *(pInputRawDataArray + tempI))
		{
			iResult = *(pInputRawDataArray + tempI);
		}
	}
	return iResult;
}

int32_t getAcceSumRawData(int16_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	uint8_t tempI;
	int32_t iResult = 0;
	for (tempI = 0; tempI < iInputRawDataLen; tempI++)
	{
		iResult += *(pInputRawDataArray + tempI);
	}
	return iResult;
}

int32_t getAcceRoundData(int16_t* pInputRawDataArray, uint8_t iInputRawDataLen)
{
	int16_t iMaxRawData;
	int16_t iMinRawData;
	iMaxRawData = getAcceMaxRawData(pInputRawDataArray, iInputRawDataLen);
	iMinRawData = getAcceMinRawData(pInputRawDataArray, iInputRawDataLen);
	return (getAcceSumRawData(pInputRawDataArray, iInputRawDataLen) - iMaxRawData - iMinRawData);
}
int16_t* pConjunctRawDataArray(int16_t *pRawDataEntr, uint16_t iArrayIndex, uint16_t iBufferLength)
{
	if (iArrayIndex >= iBufferLength)
	{
		return NULL;
	}
	memcpy(staSequencedRawDataBuf, pRawDataEntr + iArrayIndex, iBufferLength - iArrayIndex);
	if (0 != iArrayIndex)
	{
		memcpy(staSequencedRawDataBuf + iBufferLength - iArrayIndex, pRawDataEntr, iArrayIndex);
	}
	return staSequencedRawDataBuf;
}

void MotionDetectManager(void)
{
	static uint16_t staAcceCommuBusyWaitTime;

	switch (staMotionDetectState)
	{
	case MOTION_DETECT_IDLE:

		break;

	case MOTION_DETECT_MASTER_CONF:
			/* MCU periphery config ------------------------------------------------------*/
			ACCE_MasterConfiguration();
			staMotionDetectState = MOTION_DETECT_MASTER_INIT;
		break;

	case MOTION_DETECT_MASTER_INIT:
			/* MCU queue slot and heap initial ------------------------------------------------------*/
			AcceQueueSlotInitial();
			HeapInitial();
			staMotionDetectState = MOTION_DETECT_SLAVE_CONF;
			pStaAcceRecvDawDataBufX = staAcceRecvDawDataBufX;
			staPicRefreshInterval = PIC_REFRESH_ALL_LED_OFF;
			iPeakValleyNotFoundCNT = 0;
		break;

	case MOTION_DETECT_SLAVE_CONF:
			AcceDeviceConfig();
			// staMotionDetectState was changed in the routine of AcceDeviceConfig() because some device may need other state
		break;

	case MOTION_DETECT_TASK_DISPATCH:
			if (NULL != staAcceCommuQueue[staAcceCommuIndex].pRXBuf)
			{
				staAcceCommuBusyWaitTime = GET_GLOBAL_FREERUN_US;
				staMotionDetectState = MOTION_DETECT_TRIGGER_RX;
			}
			else if (NULL != staAcceCommuQueue[staAcceCommuIndex].pTXBuf)
			{
				staAcceCommuBusyWaitTime = GET_GLOBAL_FREERUN_US;
				staMotionDetectState = MOTION_DETECT_TRIGGER_TX;
			}
			else
			{
				//staMotionDetectState = MOTION_DETECT_IDLE;
			}

		break;

	case MOTION_DETECT_TRIGGER_TX:
		while (FLAG_MOTION_ACCE_COMMU_BUSY && GLOBAL_FREERUN_US_WAITING(staAcceCommuBusyWaitTime, ACCE_COMMU_BUSY_WAIT_MAX_US))	
		{	// Normally FLAG_MOTION_ACCE_COMMU_BUSY will only be busy in MOTION_DETECT_TXING or MOTION_DETECT_RXING state
		}
		if (FLAG_MOTION_ACCE_COMMU_BUSY)
		{
			FLAG_MOTION_ACCE_COMMU_BUSY = RESET; // Force reset of FLAG_MOTION_ACCE_COMMU_BUSY
			staAcceCommuErrorCNT++;
		}
		AcceWrite(staAcceCommuQueue[staAcceCommuIndex].pTXBuf, 
				 staAcceCommuQueue[staAcceCommuIndex].iTXBufLen);
		staAcceCommuBusyWaitTime = GET_GLOBAL_FREERUN_US;
		staMotionDetectState = MOTION_DETECT_TXING;
		break;

	case MOTION_DETECT_TXING:
		if (RESET == FLAG_MOTION_ACCE_COMMU_BUSY)
		{
			AcceQueueSlotFree(staAcceCommuQueue + staAcceCommuIndex);
			LOOP_WALKER_FORWARD(staAcceCommuIndex, ACCE_COMMU_QUEUE_LEN);
			staMotionDetectState = MOTION_DETECT_TASK_DISPATCH;
		}
		else if (GLOBAL_FREERUN_US_TIMEOUT(staAcceCommuBusyWaitTime, ACCE_COMMU_TIMEOUT_US))
		{
			AcceQueueSlotFree(staAcceCommuQueue + staAcceCommuIndex);
			LOOP_WALKER_FORWARD(staAcceCommuIndex, ACCE_COMMU_QUEUE_LEN);
			staAcceCommuErrorCNT++;
			staMotionDetectState = MOTION_DETECT_TASK_DISPATCH;
		}
		break;

	case MOTION_DETECT_TRIGGER_RX:
		while (FLAG_MOTION_ACCE_COMMU_BUSY && GLOBAL_FREERUN_US_WAITING(staAcceCommuBusyWaitTime, ACCE_COMMU_BUSY_WAIT_MAX_US))	
		{	// Normally FLAG_MOTION_ACCE_COMMU_BUSY will only be busy in MOTION_DETECT_TXING or MOTION_DETECT_RXING state
		}
		if (FLAG_MOTION_ACCE_COMMU_BUSY)
		{
			FLAG_MOTION_ACCE_COMMU_BUSY = RESET; // Force reset of FLAG_MOTION_ACCE_COMMU_BUSY
			staAcceCommuErrorCNT++;
		}
		AcceRead(staAcceCommuQueue[staAcceCommuIndex].iRXAddress, 
				 staAcceCommuQueue[staAcceCommuIndex].iRXBufLen - 1,
				 staAcceCommuQueue[staAcceCommuIndex].pRXBuf);
		staAcceCommuBusyWaitTime = GET_GLOBAL_FREERUN_US;
		staMotionDetectState = MOTION_DETECT_RXING;
		break;

	case MOTION_DETECT_RXING:
		if (RESET == FLAG_MOTION_ACCE_COMMU_BUSY)
		{
			staAcceCommuQueue[staAcceCommuIndex].bCommuSuccess = TRUE;
			staMotionDetectState = MOTION_DETECT_RX_DATA_PROCCESS;
		}
		else if (GLOBAL_FREERUN_US_TIMEOUT(staAcceCommuBusyWaitTime, ACCE_COMMU_TIMEOUT_US))
		{
			staAcceCommuQueue[staAcceCommuIndex].bCommuSuccess = FALSE;
			staAcceCommuErrorCNT++;
			staMotionDetectState = MOTION_DETECT_RX_DATA_PROCCESS;
		}

		break;

	case MOTION_DETECT_RX_DATA_PROCCESS:
		if (IS_TIME_TO_PROCESS_DATA(staAcceCommuIndex))
		{
			pConjunctRawDataArray(staAcceRecvDawDataBufX, staAcceCommuIndex, ACCE_RECV_RAW_DATA_BUF_LEN);
			FLAG_MOTION_ACCE_DATA_PROCESS = SET;
		}

		// Free and move forward of task queue
		AcceQueueSlotFree(staAcceCommuQueue + staAcceCommuIndex);
		LOOP_WALKER_FORWARD(staAcceCommuIndex, ACCE_COMMU_QUEUE_LEN);
		staMotionDetectState = MOTION_DETECT_TASK_DISPATCH;
		break;

		default :
		break;
	}
}

int8_t* getDigitalizedData(int8_t* pDestinDataBuf, int16_t* pSourceDataBuf, 
						   uint8_t iDigitalizedDataBufLen, int8_t iStartUpSeed)
{
	uint8_t iTempIndex;
	*pDestinDataBuf = iStartUpSeed;
	for (iTempIndex = 1; iTempIndex < iDigitalizedDataBufLen; iTempIndex++)
	{
		if (*(pSourceDataBuf + iTempIndex) > *(pSourceDataBuf + iTempIndex - 1))
		{
			*(pDestinDataBuf + iTempIndex) = *(pDestinDataBuf + iTempIndex - 1) + 1;
		}
		else if ((*(pSourceDataBuf + iTempIndex) < *(pSourceDataBuf + iTempIndex - 1)))
		{
			*(pDestinDataBuf + iTempIndex) = *(pDestinDataBuf + iTempIndex - 1) - 1;
		}
		else
		{
			*(pDestinDataBuf + iTempIndex) = *(pDestinDataBuf + iTempIndex - 1);
		}
	}
	return pDestinDataBuf;
}

uint32_t* getEqualBitArray(uint32_t* pDestinBitArray, int8_t* pSourceDataBuf, 
						   int8_t iEqualData, uint8_t iBitArrayLen, uint8_t iDigitalizedDataBufLen) 
{
	uint8_t iTempIndex;
	uint8_t iTempArrayIndex;
	uint8_t iTempBitIndex;
	// Clear the destiny buffer
	memset(pDestinBitArray, NULL, iBitArrayLen);

	for (iTempIndex = 0; iTempIndex < iDigitalizedDataBufLen; iTempIndex++)
	{
		if (*(pSourceDataBuf + iTempIndex) == iEqualData)
		{
			iTempArrayIndex = iTempIndex / INT32_BIT_NUM;
			iTempBitIndex = iTempIndex % INT32_BIT_NUM;
			SET_BIT_INDEX(*(pDestinBitArray + iTempArrayIndex), iTempBitIndex);
		}
	}
	return pDestinBitArray;
}

int16_t getFirstHighBitIndex(uint32_t* pBitArray, uint8_t iBitNum)
{
	uint8_t iTempUintIndex;
	uint8_t iTempBitIndex;
	uint8_t iUint32Num;
	uint8_t iLastUint32BitNum;
	iUint32Num = iBitNum / INT32_BIT_NUM;
	iLastUint32BitNum = iBitNum % INT32_BIT_NUM;
	// Only one Uint
	if (0 == iUint32Num)
	{
		for (iTempBitIndex = 0; iTempBitIndex < iBitNum; iTempBitIndex++)
		{
			if (0 != GET_BIT_INDEX(*pBitArray, iTempBitIndex))
			{
				return iTempBitIndex;
			}
		}
		return -1;
	}

	for (iTempUintIndex = 0; iTempUintIndex < iUint32Num; iTempUintIndex++)
	{
		for (iTempBitIndex = 0; iTempBitIndex < INT32_BIT_NUM; iTempBitIndex++)
		{
			if (0 != GET_BIT_INDEX(*(pBitArray + iTempUintIndex), iTempBitIndex))
			{
				return (iTempUintIndex * INT32_BIT_NUM + iTempBitIndex);
			}
		}
	}

	// Last Uint
	if (0 != iLastUint32BitNum)
	{
		for (iTempBitIndex = 0; iTempBitIndex < iLastUint32BitNum; iTempBitIndex++)
		{
			if (0 != GET_BIT_INDEX(*(pBitArray + iUint32Num), iTempBitIndex))
			{
				return (iUint32Num * INT32_BIT_NUM + iTempBitIndex);
			}
		}
	}
	return -1;
}

int16_t getLastHighBitIndex(uint32_t* pBitArray, uint8_t iBitNum)
{
	int16_t iTempUintIndex;
	int16_t iTempBitIndex;
	uint8_t iUint32Num;
	uint8_t iLastUint32BitNum;
	iUint32Num = iBitNum / INT32_BIT_NUM;
	iLastUint32BitNum = iBitNum % INT32_BIT_NUM;
	// Only one Uint
	if (0 == iUint32Num)
	{
		for (iTempBitIndex = iBitNum; iTempBitIndex >= 0; iTempBitIndex--)
		{
			if (0 != GET_BIT_INDEX(*pBitArray, iTempBitIndex))
			{
				return iTempBitIndex;
			}
		}
		return -1;
	}
	// Last Uint
	if (0 != iLastUint32BitNum)
	{
		for (iTempBitIndex = iLastUint32BitNum; iTempBitIndex >= 0; iTempBitIndex--)
		{
			if (0 != GET_BIT_INDEX(*(pBitArray + iUint32Num), iTempBitIndex))
			{
				return (iUint32Num * INT32_BIT_NUM + iTempBitIndex);
			}
		}
	}
	for (iTempUintIndex = iUint32Num - 1; iTempUintIndex >= 0; iTempUintIndex--)
	{
		for (iTempBitIndex = INT32_BIT_NUM - 1; iTempBitIndex >= 0; iTempBitIndex--)
		{
			if (0 != GET_BIT_INDEX(*(pBitArray + iTempUintIndex), iTempBitIndex))
			{
				return (iTempUintIndex * INT32_BIT_NUM + iTempBitIndex);
			}
		}
	}
	return -1;
}

int16_t getAverageHighBitPos(uint32_t* pBitArray, uint8_t iBitNum)
{
	int16_t iFirstHighBit, iLastHighBit;
	iFirstHighBit = getFirstHighBitIndex(pBitArray, iBitNum);
	iLastHighBit = getLastHighBitIndex(pBitArray, iBitNum);
	if ((-1 == iLastHighBit) || (-1 == iFirstHighBit))
	{
		return -1;
	}

	if (iLastHighBit - iFirstHighBit > PV_BIT_ARRAY_AVERAGE_LEN_MAX)
	{
		return -1;
	}

	return ((iLastHighBit + iFirstHighBit) >> 1);
}

STR_AcceWVF_PV* getPeakOrValley(uint8_t iPeakIndex, uint8_t iValleyIndex, uint8_t iBitArrayLen, 
								uint8_t iIndexEndTime, uint8_t iIndexIntervalTime)
{
	static STR_AcceWVF_PV staResult;

	if ((iPeakIndex < PV_BIT_FORBIDDEN_START_NUM) || (iPeakIndex > iBitArrayLen - PV_BIT_FORBIDDEN_END_NUM - 1))
	{
		if ((iValleyIndex < PV_BIT_FORBIDDEN_START_NUM) || (iValleyIndex > iBitArrayLen - PV_BIT_FORBIDDEN_END_NUM - 1))
		{
			return NULL;
		}
		else
		{
			staResult.enumPeakValley = ACCE_WVF_V;
			staResult.iSysTickTime = iIndexEndTime - ((iBitArrayLen - iValleyIndex - 1) << 1);
			return &staResult;
		}
		
	}

	if ((iValleyIndex < PV_BIT_FORBIDDEN_START_NUM) || (iValleyIndex > iBitArrayLen - PV_BIT_FORBIDDEN_END_NUM - 1))
	{
		if ((iPeakIndex < PV_BIT_FORBIDDEN_START_NUM) || (iPeakIndex > iBitArrayLen - PV_BIT_FORBIDDEN_END_NUM - 1))
		{
			return NULL;
		}
		else
		{
			staResult.enumPeakValley = ACCE_WVF_P;
			staResult.iSysTickTime = iIndexEndTime - ((iBitArrayLen - iPeakIndex - 1) << 1);
			return &staResult;
		}
	}
	return NULL;
}

void MotionProcessManager(void)
{
	static int8_t staDigitalizedDataBuf[ACCE_RECV_RAW_DATA_BUF_LEN];	// Limit of less than 128 length
	static int8_t staDigitalizedDataMax;
	static int8_t staDigitalizedDataMin;
	static uint32_t staPeakBitArray[ACCE_PEAK_VALLEY_BIT_ARRAY_LEN];
	static uint32_t staValleyBitArray[ACCE_PEAK_VALLEY_BIT_ARRAY_LEN];
	static int16_t staPeakAverage, staValleyAverage;
	STR_AcceWVF_PV* newAccePV;

	switch (staMotionProcessState)
	{
	case MOTION_PROCESS_IDLE:
		if (SET == FLAG_MOTION_ACCE_DATA_PROCESS)
		{
			FLAG_MOTION_ACCE_DATA_PROCESS = RESET;
			staMotionProcessState = MOTION_PROCESS_DIGITALIZE;
		}
		break;

	case MOTION_PROCESS_DIGITALIZE:
		getDigitalizedData(staDigitalizedDataBuf, staSequencedRawDataBuf, ACCE_RECV_RAW_DATA_BUF_LEN, 0);
		staMotionProcessState = MOTION_PROCESS_GET_DIGI_DATA_MAX;
		break;

	case MOTION_PROCESS_GET_DIGI_DATA_MAX:
		staDigitalizedDataMax = getInt8MaxData(staDigitalizedDataBuf, ACCE_RECV_RAW_DATA_BUF_LEN);
		staMotionProcessState = MOTION_PROCESS_GET_DIGI_DATA_MIN;
		break;

	case MOTION_PROCESS_GET_DIGI_DATA_MIN:
		staDigitalizedDataMin = getInt8MinData(staDigitalizedDataBuf, ACCE_RECV_RAW_DATA_BUF_LEN);
		staMotionProcessState = MOTION_PROCESS_GET_PEAK_BIT_ARRAY;
		break;

	case MOTION_PROCESS_GET_PEAK_BIT_ARRAY:
		getEqualBitArray(staPeakBitArray, staDigitalizedDataBuf, staDigitalizedDataMax, ACCE_PEAK_VALLEY_BIT_ARRAY_LEN, ACCE_RECV_RAW_DATA_BUF_LEN);
		staMotionProcessState = MOTION_PROCESS_GET_VALLEY_BIT_ARRAY;
		break;

	case MOTION_PROCESS_GET_VALLEY_BIT_ARRAY:
		getEqualBitArray(staValleyBitArray, staDigitalizedDataBuf, staDigitalizedDataMin, ACCE_PEAK_VALLEY_BIT_ARRAY_LEN, ACCE_RECV_RAW_DATA_BUF_LEN);
		staMotionProcessState = MOTION_PROCESS_GET_PEAK;
		break;

	case MOTION_PROCESS_GET_PEAK:
		staPeakAverage = getAverageHighBitPos(staPeakBitArray, ACCE_PEAK_VALLEY_BIT_ARRAY_LEN);
		if (-1 == staPeakAverage)
		{
			staPeakAverageErrorCNT++;
			staMotionProcessState = MOTION_PROCESS_IDLE;
		}
		else
		{
			staMotionProcessState = MOTION_PROCESS_GET_VALLEY;
		}
		break;

	case MOTION_PROCESS_GET_VALLEY:
		staValleyAverage = getAverageHighBitPos(staValleyBitArray, ACCE_PEAK_VALLEY_BIT_ARRAY_LEN);
		if (-1 == staValleyAverage)
		{
			staValleyAverageErrorCNT++;
			staMotionProcessState = MOTION_PROCESS_IDLE;
		}
		else
		{
			staMotionProcessState = MOTION_PROCESS_GET_PV;
		}
		break;

	case MOTION_PROCESS_GET_PV:
		newAccePV = getPeakOrValley(staPeakAverage, staValleyAverage, ACCE_PEAK_VALLEY_BIT_ARRAY_LEN, 
									iRawDataRecvEndTime, ACCE_MONITOR_SAMPLE_INTERVAL);
		if (NULL == newAccePV)
		{
			iPeakValleyNotFoundCNT++;
			staMotionProcessState = MOTION_PROCESS_IDLE;
		}
		else
		{
			memcpy(&strNewPeakOrValley, newAccePV, sizeof(STR_AcceWVF_PV));
			FLAG_NEW_PV_FOUND = SET;
		}
		break;
	}

}

void MotionManager(void)
{
	MotionDetectManager();
	MotionProcessManager();
}
