#ifndef _SETINP_H_
#define _SETINP_H_
/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.setinp * by ML ****************************************************/
/****************************************************************************/

BOOL    CALLBACK SetInputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL	siSetSharkInput( VOID );
VOID	siSaveInputs( VOID );
VOID	siInitInputs( VOID );
UINT	siGetBitRes( VOID );

#endif // _SETINP_H_