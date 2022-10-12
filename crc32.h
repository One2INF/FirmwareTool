#pragma once


#include <stdint.h>
#include <stddef.h>


uint32_t stm32crc32_Byte(uint32_t crc32, uint8_t* ptr, size_t len);
uint32_t stm32crc32(uint32_t* ptr, size_t len);
uint32_t CalcCRC(uint8_t * pData, uint32_t DataLength);
