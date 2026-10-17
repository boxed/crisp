/*************************************************************************/
/* PCI based VGA device description                                      */
/*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct {
	DWORD	size;	// Size of structure
	DWORD	bus;	// PCI bus no
	DWORD	dev;	// PCI device no
	DWORD	fun;	// PCI function no
	DWORD	ID;		// VGA device ID
	DWORD	VB00;	// Vmem physical base addr
	DWORD	VB;		// Top left corner physical base addr
	DWORD	VL;		// Pitch
	DWORD	X;		// X resolution
	DWORD	Y;		// Y resolution
	DWORD	VMEM;	// Vmem size in bytes
	WORD	bpp;	// Bits/pixel
	WORD	bypp;	// Bytes/pixel
	BOOL	exist;	// VGA present
	LPBYTE	vsel;	// Pointer to the VMEM (32MB limit)
	LPBYTE	vusel;	// User pointer to the VMEM (4MB limit)
	HWND	hWnd;	// Handle for live window
} PCIDEV;
typedef PCIDEV far * LPPCIDEV;

typedef BOOL (CALLBACK* VGAENUMPROC)(DWORD ID, BOOL bPrimary, DWORD dwCFGAddr, DWORD dwContext);

#pragma pack()

#ifdef __cplusplus
}
#endif
