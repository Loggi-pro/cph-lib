#pragma once
#include "mbtypes.h"
/*! \defgroup modbus_utils Utilities
 *
 * This module contains some utility functions which can be used by
 * the application. It includes some special functions for working with
 * bitfields backed by a character array buffer.
 *
 */
/*! \addtogroup modbus_utils
 *  @{
 */


MBException prveMBError2Exception(MBErrorCode eErrorCode);

uint16_t  calc_crc16(uint8_t* frame, uint16_t usLen);
uint16_t readAsU16_HighEndian(uint8_t* arr);
/*! @} */