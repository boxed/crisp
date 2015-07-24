#pragma once

#ifndef __SOFTHARD_TECHNOLOGY_MV_4_0_HPP__
#define __SOFTHARD_TECHNOLOGY_MV_4_0_HPP__

GRABBER ChooseGrabber();
void DoLiveMV40( HWND hWnd );
void TerminateMV40Capturing();
extern BOOL		bHandleChanged;
extern HANDLE	hLiveThreadMV40;
extern BOOL		bStopGrabbingMV40;

#endif // __SOFTHARD_TECHNOLOGY_MV_4_0_HPP__
