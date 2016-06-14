/*
 * skybackground.h
 *
 *  Created on: Dec 2, 2010
 *      Author: chyan
 */

#ifndef SKYBACKGROUND_H_
#define SKYBACKGROUND_H_

#include "imparameter.h"


class SkyBackground {

private:
	char* FileList;
	char* MaskList;
	char* DetrendFile[MaxSkyFiles];
	char* MaskFile[MaxSkyFiles];
	int nSkyFITS;
	int nMaskFITS;
	Image* DetrendImage;
	Image* MaskImage;
	int TotalFrameNumber;
	double qeratio[MaxChipInstalled];
	double* skylvl_corrmean;
	double* skylvl_var;

	double skyBackgroundImage[MaxChipInstalled*MaxImageXSize*MaxImageYSize];

public:
	void  setFileList(char* list);
	void  setMaskList(char* list);
	void  setNSkyFITS(int nsky);
	void  setNMaskFITS(int n);
	void  setDetrendFile(char* list);
	void  setMaskFile(char* list);
	void  setDetrendImage();
	void  setMaskImage();
	void  setTotalFrameNumber();

	char* getFileList();
	char* getMaskList();
	int   getNSkyFITS();
	int   getNMaskFITS();
	char* getDetrendFile(int i);
	char* getMaskFile(int i);
	int   getTotalFrameNumber();

	void parseList(char *liststr,char* filelist[], int* n);
	void applyMasktoDetrend();
	void calcQeRatio();
	void correctSkyLevel();
	void buildSkyImage();

	int writeSkyBackgroundImage(char* filename);

	SkyBackground();
	SkyBackground(char* list);
	SkyBackground(char* list, char* masklist);
	~SkyBackground();
};

#endif /* SKYBACKGROUND_H_ */
