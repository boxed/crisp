
/* =====[ MARXDEV.H ]====================================================

   Description:     Include file for CBN driver.

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  01.02.95  Peter Glen    Initial version.
      0.00  13.02.95  Peter Glen    Cleaned up for marxdev.
      0.00  14.02.95  Peter Glen    Got name as define
      0.00  16.02.95  Peter Glen    Three devices.
      0.00  16.02.95  Peter Glen    Porting to '95.

   ======================================================================= */

#ifndef marxdev_defined
#define marxdev_defined

// ***********************************************************************
//
// Define the IOCTL codes we will use.  The IOCTL code contains a command
// identifier, plus other information about the device, the type of access
// with which the file must have been opened, and the type of buffering.
//
// -----------------------------------------------------------------------
//
// Note: This file relies upon <windows.h> being included.
//
// -----------------------------------------------------------------------
//
// File system device name:
//
//      When you execute a CreateFile call to open the device, use
//      "\\.\MARXDEV", or, given C's conversion of \\ to \, "\\\\.\\MARXDEV"
//
//      Simply use MARX_DEVICE_NAME
//
// 95 note: This file is near identical with NT countepart.
//
// ***********************************************************************

/*
 * Export the three device names:
 */

#define MARX_DEVICE_NAME1 "\\\\.\\MARXDEV1"
#define MARX_DEVICE_NAME2 "\\\\.\\MARXDEV2"
#define MARX_DEVICE_NAME3 "\\\\.\\MARXDEV3"

#define MARX_95DEVICE_NAME1 "\\\\.\\CBN"
#define MARX_95DEVICE_NAME2 "\\\\.\\CBN"
#define MARX_95DEVICE_NAME3 "\\\\.\\CBN"


//define CTL_CODE(code_type, code_off) code_type + code_off

// Device type in the "USER DEFINED" range.

#define GPD_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

#define IOCTL_GPD_READ_PORT_UCHAR \
    CTL_CODE( GPD_TYPE, 0x900, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_READ_PORT_USHORT \
    CTL_CODE( GPD_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_READ_PORT_ULONG \
    CTL_CODE( GPD_TYPE, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_WRITE_PORT_UCHAR \
    CTL_CODE(GPD_TYPE,  0x910, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_GPD_WRITE_PORT_USHORT \
    CTL_CODE(GPD_TYPE,  0x911, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_GPD_WRITE_PORT_ULONG \
    CTL_CODE(GPD_TYPE,  0x912, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define MARX_IOCTL_ID \
    CTL_CODE(GPD_TYPE,  0x920, METHOD_BUFFERED, FILE_READ_ACCESS)

#define MARX_IOCTL_RAM \
    CTL_CODE(GPD_TYPE,  0x921, METHOD_BUFFERED, FILE_READ_ACCESS)

#define MARX_IOCTL_CRYPT \
    CTL_CODE(GPD_TYPE,  0x922, METHOD_BUFFERED, FILE_READ_ACCESS)

#define MARX_IOCTL_TEST \
    CTL_CODE(GPD_TYPE,  0x923, METHOD_BUFFERED, FILE_READ_ACCESS)

// Types to handle the input and output:

typedef struct
    {
    unsigned long     iOpCode;               // input code
    unsigned long     iPortNr;               // port number
    unsigned char     input[12];             // input pass and all params
    unsigned long     ret_val;               // output ret_val
    unsigned long     len;                   // output length (not used)
    unsigned char     firstbyte[2];          // output data (used as pointer)
    //  The structure can be arbitrary long, according to the function used
    } MARX_I_O;

#endif

/* EOF */
