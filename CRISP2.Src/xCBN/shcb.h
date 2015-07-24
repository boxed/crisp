/****************************************************************************/
/* Copyright© 1996 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* MARX Crypto-Box Interface by ML ******************************************/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define	APP_CRISP		0
#define	APP_TRIX		1
#define	APP_PENTACLE	2
#define	APP_SIZE		32

#pragma pack(1)

#define MAGIC_NUMBER	0x5EAA

/****************************************************************************/
typedef struct {
	DWORD		dwExp;
	DWORD		flags;
#define	CRISP_ACQUISITION	0x00000001
#define	CRISP_CRISP			0x00000002
#define	CRISP_ELD			0x00000004
#define	CRISP_CALIBRATION	0x00000008
#define	CRISP_EDRD			0x00000010
#define	CRISP_PHIDO			0x00000020
#define	CRISP_ELD_SPINSTAR	0x00000040
	DWORD		dummy[5];
	DWORD		crc;
} CRISP;

/****************************************************************************/
typedef struct {
	DWORD		dwExp;
	DWORD		flags;
#define	TRIX_MAIN			0x00000001
#define	TRIX_XDBASE			0x00000002
#define	TRIX_XDDIRM			0x00000004
	DWORD		dummy[5];
	DWORD		crc;
} TRIX;

/****************************************************************************/
typedef struct {
	DWORD		dwExp;
	double		one;
	DWORD		flags;
#define	PENTACLE_MAIN		0x00000001
	DWORD		dummy[3];
	DWORD		crc;
} PENTACLE;

/****************************************************************************/
typedef	struct	{
	DWORD		dwPW2;
	WORD		wSRN;
	char		UserName[36];
	CRISP		c;
	TRIX		t;
	PENTACLE	p;
	char		comment[256];
	DWORD		dwCreated;
	DWORD		dwModified;
} USER;

typedef USER far * LPUSER;
#define	RAM2S	(sizeof(CRISP)+sizeof(TRIX)+sizeof(PENTACLE))

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
WORD scbLookup( void );
BOOL scbGetUserName( LPSTR );
LPVOID scbGetAppData( short app );

#pragma pack()

#ifdef __cplusplus
}
#endif
