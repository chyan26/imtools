/*
 * \file sextractor.cpp
 *
 * \author Chi-Hung Yn
 * \brief This class defines the basic implements of C++ API of SExtractor.
 *
 *  Created on: Jul 21, 2011
 *
 * \mainpage Akepa
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *
 * etc...
 */

#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <libgen.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "skybackground.h"
#include "sextractor.h"
using namespace std;


void SextractorConfig::initializeConfig(){
	 CATALOG_NAME="test.cat";
	 CATALOG_TYPE="ASCII_HEAD";
	 PARAMETERS_NAME="default.param";
	 DETECT_TYPE="CCD";
	 DETECT_MINAREA=5.0;
	 THRESH_TYPE="RELATIVE";
	 DETECT_THRESH=1.5;
	 ANALYSIS_THRESH=1.5;
	 FILTER="Y";
	 FILTER_NAME="default.conv";
	 FILTER_THRESH=0;
	 DEBLEND_NTHRESH=32;
	 DEBLEND_MINCONT=0.05;
	 CLEAN="Y";
	 CLEAN_PARAM=1.0;
	 MASK_TYPE="CORRECT";
	 WEIGHT_TYPE="NONE";
	 WEIGHT_IMAGE="weight.fits";
	 WEIGHT_GAIN=0;
	 FLAG_IMAGE="flag.fits";
	 FLAG_TYPE="OR";
	 PHOT_APERTURES=5;

	 PHOT_AUTOPARAMS[0]=2.5;
	 PHOT_AUTOPARAMS[1]=3.5;

	 PHOT_PETROPARAMS[0]=2.0;
	 PHOT_PETROPARAMS[1]=3.5;

	 PHOT_AUTOAPERS[0]=0.0;
	 PHOT_AUTOAPERS[1]=0.0;

	 PHOT_FLUXFRAC=0.5;
	 SATUR_LEVEL= 50000.0;
	 SATUR_KEY="SATURATE";
	 MAG_ZEROPOINT=0.0;
	 MAG_GAMMA =4.0;
	 GAIN = 0.0;
	 GAIN_KEY= "GAIN";
	 PIXEL_SCALE = 1.0;
	 SEEING_FWHM = 1.2;
	 STARNNW_NAME="default.nnw";
	 BACK_TYPE="AUTO";
	 BACK_VALUE=0.0;
	 BACK_SIZE=64;
	 BACK_FILTERSIZE=3;
	 BACKPHOTO_TYPE="GLOBAL";
	 BACKPHOTO_THICK=24;
	 BACK_FILTTHRESH=0.0;
	 CHECKIMAGE_TYPE="NONE";
	 CHECKIMAGE_NAME="check.fits";
	 MEMORY_OBJSTACK=3000;
	 MEMORY_PIXSTACK=300000;
	 MEMORY_BUFSIZE=1024;
	 ASSOC_NAME="sky.list";
	 ASSOC_DATA[0]=2;
	 ASSOC_DATA[1]=3;
	 ASSOC_DATA[2]=4;

	 ASSOC_PARAMS[0]=2;
	 ASSOC_PARAMS[1]=3;
	 ASSOC_PARAMS[2]=4;
	 ASSOC_RADIUS =2.0;
	 ASSOC_TYPE="NEAREST";
	 ASSOCSELEC_TYPE="MATCHED";
	 VERBOSE_TYPE="NORMAL";
	 WRITE_XML="N";
	 XML_NAME="sex.xml";
	 XSL_URL="file:///opt/local/share/sextractor/sextractor.xsl";
	 NTHREADS=0;
	 FITS_UNSIGNED="N";
	 INTERP_MAXXLAG=16;
	 INTERP_MAXYLAG=16;
	 INTERP_TYPE="ALL";
}

void SextractorConfig::setCatalogName(char* catname){
	stringstream ss;
	ss << catname;
	ss >> CATALOG_NAME;
}

string SextractorConfig::getCatalogName(){
	return CATALOG_NAME;
}

SextractorConfig::SextractorConfig(){
	initializeConfig();
}

SextractorConfig::~SextractorConfig(){
}



///------------------------------------------------------------------
void Sextractor::setFileName(char* name){
	filename=name;
}

char* Sextractor::getFileName(){
	return filename;
}

void Sextractor::setSigmaLevel(char* name){
	SexConf.DETECT_THRESH=atof(name);
	SexConf.ANALYSIS_THRESH=atof(name);
}

float Sextractor::getSigmaLevel(){
	return SexConf.DETECT_THRESH;
}


void Sextractor::setCatName(char* name){
	stringstream ss;
	ss<<name;
	catname=ss.str();
	SexConf.CATALOG_NAME=catname;
}

string Sextractor::getCatName(){
	return catname;
}

void Sextractor::setCheckName(char* name){
	stringstream ss;
	ss<<name;
	checkname=ss.str();
	SexConf.CHECKIMAGE_NAME=checkname;
}

string Sextractor::getCheckName(){
	return checkname;
}

void Sextractor::setCheckType(char* name){
	stringstream ss;
	ss<<name;
	checktype=ss.str();
	SexConf.CHECKIMAGE_TYPE=checktype;
}

string Sextractor::getCheckType(){
	return checktype;
}

void Sextractor::setWeightName(char* name){
	stringstream ss;
	ss<<name;
	weightname=ss.str();
	SexConf.WEIGHT_IMAGE=weightname;
	SexConf.WEIGHT_TYPE="MAP_WEIGHT";
}

string Sextractor::getWeightName(){
	return weightname;
}

void Sextractor::setExcutable(string name){
	seExcutable=name;
}

string Sextractor::getExcutable(){
	return seExcutable;
}

void Sextractor::locateSexExcutable(){
	char buffer[80];
	string data;

	FILE *fp=popen("which sex","r");
	while (fgets(buffer, 80, fp) != NULL){
	    data=string(strim(buffer));
	}
	pclose(fp);

	/** Check if the excutable existed, otherwise throw an exception*/
	if (data.length() == 0){
		throw SextractorException(1,__FILE__, __func__, __LINE__);
	}

	setExcutable(data);

}

void Sextractor::setWorkDir(){
	pid_t pid;
	string command;
	stringstream ss;

	/** Using PID to assign a directory for working */
	pid=getpid();
	ss << pid;

	seWorkDir=string("/tmp/")+ss.str();

	/** Making a directory */
	if(mkdir(seWorkDir.c_str(),0777)==-1){
		throw SextractorException(2,__FILE__, __func__, __LINE__);
	}

}


string Sextractor::getWorkDir(){
	return seWorkDir;
}


void Sextractor::setConfigName(char *name){
	seConfigName=string(name);
}


string Sextractor::getConfigName(){
	return seConfigName;
}


void Sextractor::cleanWorkDir(){
	string command;

	command="rm -rf "+seWorkDir;

	if(system(command.c_str())==-1){
		throw SextractorException(3,__FILE__, __func__, __LINE__);
	}

}

/**
 *   This method is used to gather all files need for run SExtractor.
 */
void Sextractor::gatherFiles(){
	stringstream ss;
	string command;
	ofstream sefile;

	/** Check if the executable existed, otherwise throw an exception*/
	if (seConfigName.length() == 0){
		throw SextractorException(4,__FILE__, __func__, __LINE__);
	}

	/** Generate a configuration file in this directory */
	command=seExcutable+" -dd >"+seWorkDir+"/"+getConfigName();
	if (system(command.c_str()) == -1){
		throw SextractorException(5,__FILE__, __func__, __LINE__);
	}

	/** copy FITS file to this working directory */
	ss<<filename;
	command="cp -rf "+ss.str()+" "+seWorkDir+"/";
	if (system(command.c_str()) == -1){
		throw SextractorException(6,__FILE__, __func__, __LINE__);
	}


	/** If weight map is specified, copy it to current directory */
	if (weightname.length() != 0){
		command="cp -rf "+weightname+" "+seWorkDir+"/";
		if (system(command.c_str()) == -1){
			throw SextractorException(7,__FILE__, __func__, __LINE__);
		}

	}

	/** Preparing filter, here we use Gaussian filter gauss_2.0_3x3.conv as default */
	sefile.open((seWorkDir+"/"+"default.conv").c_str());
	sefile << "CONV NORM\n";
	sefile << "# 3x3 convolution mask of a gaussian PSF with FWHM = 2.0 pixels.\n";
	sefile << "0.260856 0.483068 0.260856\n";
	sefile << "0.483068 0.894573 0.483068\n";
	sefile << "0.260856 0.483068 0.260856\n";
	sefile << "\n";
	sefile.close();

	/** Preparing default neural network file */
	sefile.open((seWorkDir+"/"+"default.nnw").c_str());
	sefile << "NNW\n";
	sefile << "# Neural Network Weights for the SExtractor star/galaxy classifier (V1.3)\n";
	sefile << "# inputs:     9 for profile parameters + 1 for seeing.\n";
	sefile << "# outputs:    ``Stellarity index'' (0.0 to 1.0)\n";
	sefile << "# Seeing FWHM range: from 0.025 to 5.5'' (images must have 1.5 < FWHM < 5 pixels)\n";
	sefile << "# Optimized for Moffat profiles with 2<= beta <= 4.\n";
	sefile << "\n";
	sefile << " 3 10 10  1\n";
	sefile << "\n";
	sefile << "-1.56604e+00 -2.48265e+00 -1.44564e+00 -1.24675e+00 -9.44913e-01 -5.22453e-01  4.61342e-02  8.31957e-01  2.15505e+00  2.64769e-01\n";
	sefile << " 3.03477e+00  2.69561e+00  3.16188e+00  3.34497e+00  3.51885e+00  3.65570e+00  3.74856e+00  3.84541e+00  4.22811e+00  3.27734e+00\n";
	sefile << "\n";
	sefile << "-3.22480e-01 -2.12804e+00  6.50750e-01 -1.11242e+00 -1.40683e+00 -1.55944e+00 -1.84558e+00 -1.18946e-01  5.52395e-01 -4.36564e-01 -5.30052e+00\n";
	sefile << " 4.62594e-01 -3.29127e+00  1.10950e+00 -6.01857e-01  1.29492e-01  1.42290e+00  2.90741e+00  2.44058e+00 -9.19118e-01  8.42851e-01 -4.69824e+00\n";
	sefile << "-2.57424e+00  8.96469e-01  8.34775e-01  2.18845e+00  2.46526e+00  8.60878e-02 -6.88080e-01 -1.33623e-02  9.30403e-02  1.64942e+00 -1.01231e+00\n";
	sefile << " 4.81041e+00  1.53747e+00 -1.12216e+00 -3.16008e+00 -1.67404e+00 -1.75767e+00 -1.29310e+00  5.59549e-01  8.08468e-01 -1.01592e-02 -7.54052e+00\n";
	sefile << " 1.01933e+01 -2.09484e+01 -1.07426e+00  9.87912e-01  6.05210e-01 -6.04535e-02 -5.87826e-01 -7.94117e-01 -4.89190e-01 -8.12710e-02 -2.07067e+01\n";
	sefile << "-5.31793e+00  7.94240e+00 -4.64165e+00 -4.37436e+00 -1.55417e+00  7.54368e-01  1.09608e+00  1.45967e+00  1.62946e+00 -1.01301e+00  1.13514e-01\n";
	sefile << " 2.20336e-01  1.70056e+00 -5.20105e-01 -4.28330e-01  1.57258e-03 -3.36502e-01 -8.18568e-02 -7.16163e+00  8.23195e+00 -1.71561e-02 -1.13749e+01\n";
	sefile << " 3.75075e+00  7.25399e+00 -1.75325e+00 -2.68814e+00 -3.71128e+00 -4.62933e+00 -2.13747e+00 -1.89186e-01  1.29122e+00 -7.49380e-01  6.71712e-01\n";
	sefile << "-8.41923e-01  4.64997e+00  5.65808e-01 -3.08277e-01 -1.01687e+00  1.73127e-01 -8.92130e-01  1.89044e+00 -2.75543e-01 -7.72828e-01  5.36745e-01\n";
	sefile << "-3.65598e+00  7.56997e+00 -3.76373e+00 -1.74542e+00 -1.37540e-01 -5.55400e-01 -1.59195e-01  1.27910e-01  1.91906e+00  1.42119e+00 -4.35502e+00\n";
	sefile << "\n";
	sefile << "-1.70059e+00 -3.65695e+00  1.22367e+00 -5.74367e-01 -3.29571e+00  2.46316e+00  5.22353e+00  2.42038e+00  1.22919e+00 -9.22250e-01 -2.32028e+00\n";
	sefile << "\n";
	sefile << "\n";
	sefile << " 0.00000e+00\n";
	sefile << " 1.00000e+00\n";
	sefile.close();

	/** Preparing default parameter file */
	sefile.open((seWorkDir+"/"+"default.param").c_str());
	sefile << "NUMBER\n";
	sefile << "EXT_NUMBER\n";
	sefile << "FLUX_AUTO\n";
	sefile << "FLUXERR_AUTO\n";
	sefile << "MAG_AUTO\n";
	sefile << "MAGERR_AUTO\n";
	sefile << "XWIN_IMAGE\n";
	sefile << "YWIN_IMAGE\n";
	sefile << "A_IMAGE\n";
	sefile << "B_IMAGE\n";
	sefile << "FLAGS\n";
	sefile << "CLASS_STAR\n";
	sefile << "\n";
	sefile.close();
}


void Sextractor::runSextractor(){
	string command;
	char buffer[80];
	stringstream ss;

	/** Preparing a directory for storing SExtractor related files. */
	setWorkDir();

	/** Putting all files together */
	gatherFiles();

	/** Run SExtractor */
	getcwd(buffer, 80);
	chdir(seWorkDir.c_str());

	command=seExcutable+" -c "+seConfigName+" "+basename(filename);

	if (catname.length() != 0){
		command.append(" -CATALOG_NAME "+SexConf.CATALOG_NAME);
	}

	if (checkname.length() != 0){
		if (checktype.length() != 0){
			command.append(" -CHECKIMAGE_TYPE "+SexConf.CHECKIMAGE_TYPE);
		}
		command.append(" -CHECKIMAGE_NAME "+SexConf.CHECKIMAGE_NAME);
	}

	ss<<SexConf.ANALYSIS_THRESH;
	command.append(" -ANALYSIS_THRESH "+ss.str());

	ss.str("");
	ss<<SexConf.DETECT_THRESH;
	command.append(" -DETECT_THRESH "+ss.str());

	command.append(" -WEIGHT_TYPE " +SexConf.WEIGHT_TYPE);
	command.append(" -WEIGHT_IMAGE "+SexConf.WEIGHT_IMAGE);
	command.append(" -WEIGHT_THRESH 0 ");

	#ifdef DEBUG
	cout << command << endl;
	#endif

	if (system((command+" 1>/dev/null 2>/dev/null").c_str()) == -1){
		chdir(strim(buffer));
		throw SextractorException(8,__FILE__, __func__, __LINE__);
	}
	chdir(strim(buffer));
}

Sextractor::Sextractor(){
	locateSexExcutable();
}

Sextractor::Sextractor(char* filename){
	locateSexExcutable();
	this->filename=filename;

}

Sextractor::Sextractor(char* filename, char* catname){
	locateSexExcutable();
	this->filename=filename;
	SexConf.setCatalogName(catname);

}


Sextractor::~Sextractor(){
	cleanWorkDir();
}




