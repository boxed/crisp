#pragma once

class ELD;
void RunMenuCommand(UINT cmd);

namespace test
{
	HWND OpenELD();
	HWND OpenELDForRings();

	void RunAutomaticRefinement(HWND eldWindow);
	void RunManualRefinement(
		HWND eldWindow,
		double x1, double y1,
		int h1, int k1,
		double x2, double y2,
		int h2, int k2,
		double x3, double y3,
		int h3, int k3);

	ELD* ELDFromWindow(HWND eldWindow);
	
	void RunUnitTests();
}
