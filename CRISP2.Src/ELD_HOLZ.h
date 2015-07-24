#pragma once

void DrawELD_HOLZ(OBJ* lpObj, LPELD lpEld, HDC hDC);
//BOOL DoAutoHOLZ(OBJ* lpObj, LPELD lpEld);
//BOOL DoHOLZref(OBJ* lpObj, LPELD lpEld, int nLaueZone);
//void SetELDparamsFromHOLZRef(OBJ* lpObj, LPELD lpEld, VectorD &vParams);
BOOL IsPossibleCellDoubling(OBJ* lpObj, LPELD lpEld, const CVector2d &shift,
							double &a_new, double &b_new, double &gamma_new,
							CVector2d &dir_h_new, CVector2d &dir_k_new);
BOOL AskUserForAxesChange(OBJ* lpObj, LPELD lpEld,
						  double a_new, double b_new, double gamma_new,
						  const CVector2d &dir_h_new, const CVector2d &dir_k_new);
//void CalculateProfiles(OBJ* lpObj, LPELD lpEld, double &dShiftX, double &dShiftY);
