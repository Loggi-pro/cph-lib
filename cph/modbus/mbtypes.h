#pragma once
#include "bit_array.h"


/* ----------------------- Defines ------------------------------------------*/
#define MB_ADDRESS_BROADCAST    ( 0 )   /*! Modbus broadcast address. */
#define MB_ADDRESS_MIN          ( 1 )   /*! Smallest possible slave address. */
#define MB_ADDRESS_MAX          ( 247 ) /*! Biggest possible slave address. */
#define MB_FUNC_NONE                          (  0 )
#define MB_FUNC_READ_COILS                    (  1 )
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
#define MB_FUNC_READ_HOLDING_REGISTER         (  3 )
#define MB_FUNC_READ_INPUT_REGISTER           (  4 )
#define MB_FUNC_WRITE_REGISTER                (  6 )
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS  ( 23 )
#define MB_FUNC_DIAG_READ_EXCEPTION           (  7 )
#define MB_FUNC_DIAG_DIAGNOSTIC               (  8 )
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT        ( 11 )
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG        ( 12 )
#define MB_FUNC_OTHER_REPORT_SLAVEID          ( 17 )
#define MB_FUNC_ERROR                         ( 128 )

/*! \ingroup modbus
 * \brief Errorcodes used by all function in the protocol stack.
 */
enum MBErrorCode {
	MB_ENOERR,                  /*!< no error. */
	MB_ENOREG,                  /*!< illegal register address. */
	MB_EINVAL,                  /*!< illegal argument. */
	MB_EPORTERR,                /*!< porting layer error. */
	MB_ENORES,                  /*!< insufficient resources. */
	MB_EIO,                     /*!< I/O error. */
	MB_EILLSTATE,               /*!< protocol stack in illegal state. */
	MB_ETIMEDOUT,                /*!< timeout error occurred. */
	MB_ILLEGAL_DATA             /*!< illegal data value. */
};


typedef MBErrorCode(*mbReadRegisterCallback_t)(uint16_t* out, uint16_t address, uint16_t count);
typedef MBErrorCode(*mbWriteRegisterCallback_t)(const uint16_t* data, uint16_t address,
        uint16_t count);

typedef MBErrorCode(*mbReadBitCallback_t)(bit_array_t& bits, uint16_t address, uint16_t count);
typedef MBErrorCode(*mbWriteBitCallback_t)(const bit_array_t& bits, uint16_t address,
        uint16_t count);


/* ----------------------- Type definitions ---------------------------------*/
enum MBException {
	MB_EX_NONE = 0x00,
	MB_EX_ILLEGAL_FUNCTION = 0x01,
	MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
	MB_EX_ILLEGAL_DATA_VALUE = 0x03,
	MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
	MB_EX_ACKNOWLEDGE = 0x05,
	MB_EX_SLAVE_BUSY = 0x06,
	MB_EX_MEMORY_PARITY_ERROR = 0x08,
	MB_EX_GATEWAY_PATH_FAILED = 0x0A,
	MB_EX_GATEWAY_TGT_FAILED = 0x0B
} ;
class Modbus;
typedef MBException(*MBFunctionHandler) (Modbus& obj, uint8_t* frame, uint16_t* length);

