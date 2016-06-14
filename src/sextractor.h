/*
 * sextractor.h
 *
 *  Created on: Jul 21, 2011
 *      Author: chyan
 */

#ifndef SEXTRACTOR_H_
#define SEXTRACTOR_H_

#include <iostream>
#include <string>
#include <cstring>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <libgen.h>
#include <sstream>
#include <map>
#include <unistd.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "imparameter.h"
#include "imexception.h"

using namespace std;



class SextractorConfig {
	friend class Sextractor;
	protected:
		string CATALOG_NAME;
		string CATALOG_TYPE;
		string PARAMETERS_NAME;
		string DETECT_TYPE;
		float DETECT_MINAREA;
		string THRESH_TYPE;
		float DETECT_THRESH;
		float ANALYSIS_THRESH;
		string FILTER;
		string FILTER_NAME;
		float FILTER_THRESH;
		int DEBLEND_NTHRESH;
		float DEBLEND_MINCONT;
		string CLEAN;
		float CLEAN_PARAM;
		string MASK_TYPE;
		string WEIGHT_TYPE;
		string WEIGHT_IMAGE;
		float WEIGHT_GAIN;;
		string FLAG_IMAGE;
		string FLAG_TYPE;
		float PHOT_APERTURES;
		float PHOT_AUTOPARAMS[2];
		float PHOT_PETROPARAMS[2];
		float PHOT_AUTOAPERS[2];
		float PHOT_FLUXFRAC;

		float SATUR_LEVEL;
		string SATUR_KEY;

		float MAG_ZEROPOINT;
		float MAG_GAMMA;
		float GAIN;
		string GAIN_KEY;
		float PIXEL_SCALE;
		float SEEING_FWHM;
		string STARNNW_NAME;

		string BACK_TYPE;
		float BACK_VALUE;
		float BACK_SIZE;
		float BACK_FILTERSIZE;

		string BACKPHOTO_TYPE;
		float BACKPHOTO_THICK;
		float BACK_FILTTHRESH;

		string CHECKIMAGE_TYPE;
		string CHECKIMAGE_NAME;

		int MEMORY_OBJSTACK;
		int MEMORY_PIXSTACK;
		int MEMORY_BUFSIZE;

		string ASSOC_NAME;
		int ASSOC_DATA[3];
		int ASSOC_PARAMS[3];
		float ASSOC_RADIUS;
		string ASSOC_TYPE;

		string ASSOCSELEC_TYPE;
		string VERBOSE_TYPE;
		string WRITE_XML;
		string XML_NAME;
		string XSL_URL;

		int NTHREADS;

		string FITS_UNSIGNED;
		int INTERP_MAXXLAG;
		int INTERP_MAXYLAG;
		string INTERP_TYPE;
	public:
		void setCatalogName(char* name);
		void setCatalogType(char* name);
		void setParametersName(char* name);

		string getCatalogName();
		string getCatalogType();
		string getParametersName();

		void initializeConfig();
		SextractorConfig();
		~SextractorConfig();
};


///-----------------------------------------------------------


class Sextractor{
	private:
		char* filename;
		string catname;
		string weightname;
		string checkname;
		string checktype;

		string seExcutable;
		SextractorConfig SexConf;
		string seWorkDir;
		string seConfigName;

	public:
		void  setFileName(char* name);
		char* getFileName();

		void  setSigmaLevel(char* name);
		float getSigmaLevel();

		void  setCatName(char* name);
		string getCatName();

		void  setCheckName(char* name);
		string getCheckName();

		void  setCheckType(char* name);
		string getCheckType();

		void  setWeightName(char* name);
		string getWeightName();

		void  setExcutable(string name);
		string getExcutable();

		void setWorkDir();
		string getWorkDir();

		void setConfigName(char* name);
		string getConfigName();

		void locateSexExcutable();
		void gatherFiles();
		void cleanWorkDir();
		void runSextractor();

		Sextractor();
		Sextractor(char* filename);
		Sextractor(char* filename, char* catname);
		~Sextractor();
};









#endif /* SEXTRACTOR_H_ */
