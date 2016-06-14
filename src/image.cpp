/**
 * \file image.cpp
 * \author Chi-Hung Yan
 * \brief This class defines the basic implements
 *
 *  Created on: Aug 13, 2010
 *      Author: chyan
 */
/*! \mainpage Akepa
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
#include <string>
#include <cstring>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "imparameter.h"
#include "imexception.h"

using namespace std;



/** Set the filename
 *  \param filename is a pointer of char to the file
 */
void Image::setFilename(char* filename){
	//string string,basestring,dirstring;
	Filename = filename;
	//Basename=basename(filename);
	//Dirname=dirname(filename);

}
/** Get the filename in this class.
 *
 *  \return a char pointer to the filename
 */
char* Image::getFilename(){
	return Filename;
}

char* Image::getBaseName(){
	return basename(Filename);
}

char* Image::getDirName(){
	return dirname(Filename);
}

/** Get the FITS file pointer in this class.
 *
 *  \return a FITS fie pointer to the input file
 */
fitsfile* Image::getInputFITSptr(){
	return fptr;
}

/** This function opens a fits file and put the point of to the FITS file to
 *   a pointer.
 *
 * \return int if the FITS file is closed
 */
void Image::openFitfile(){
	int status=0;
	fits_open_file(&fptr,  Filename, READONLY, &status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not open FITS file "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}
}
/** This function close a fits file.
 *
 * \return int if the FITS file is closed
 */

int Image::closeFitfile(){
	int status=0;
	if (fptr != NULL){
		fits_close_file(fptr, &status);
		/** If status != 0, then throw an exception and return */
		if (status != 0){
			fprintf(stderr, "Error: (%s:%s:%d) can not close FITS file "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(status);
		}
	}

	if (outfptr != NULL){
		fits_close_file(outfptr, &status);
		/** If status != 0, then throw an exception and return */
		if (status != 0){
			fprintf(stderr, "Error: (%s:%s:%d) can not close FITS file "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(status);
		}
	}

	return EXIT_SUCCESS;
}
/** This function read the FITS header into a structure
 *   a pointer.
 *
 * \return int if the FITS file is closed
 */
void Image::setImageHeader(){
	int status=0,hdupos, control=0;
	int naxis,bitpix,datatype,size;
	long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	long intnum;
	double dblnum;
	char keyvalue[FLEN_VALUE], keycomment[FLEN_COMMENT];
	string string;
	char card[FLEN_CARD]="";

	int  keyexist,morekeys,ii;

	/// Get the current HDU position
	fits_get_hdu_num(fptr, &hdupos);

	/// Get information from PHU
	fits_movabs_hdu(fptr, 1, NULL, &status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	/// Count the number of keys in this header unit
	fits_get_hdrspace(fptr, &keyexist, &morekeys,&status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not access FITS header "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	this->Header.mainHeaderKeys=keyexist;
	for (ii = 1; ii <= keyexist; ii++)  {
		fits_read_record(fptr, ii, card, &status); /* read keyword */
		if (status != 0){
			fprintf(stderr, "Logonly: (%s:%s:%d) could not access FITS header "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			status = 0;
		}
		if (strstr (card,"Reserved space") != NULL && control == 0){
			this->Header.firstMainHeaderSpace=ii-1;
			control=1;
		}
		string=card;
		Header.mainHeader.append(string);
		Header.mainHeader.append("&");
	}

	/// If the variable control is still zero that means there is no Reserved Space.
	///  So, use the last available position.
	if (control == 0){
		this->Header.firstMainHeaderSpace=keyexist-1;
	}

	fits_read_key_lng(fptr, "NEXTEND", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword NEXTEND "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status = 0;
	}
	HeaderInfo.nextend = intnum;


	fits_read_card(fptr,"FILTER", card, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword FILTER "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		HeaderInfo.filter="None";
		status = 0;
	} else {
		fits_parse_value(card, keyvalue, keycomment, &status);
		string=keyvalue;
		string.erase(0,1);
		string.erase(string.find_last_not_of("' ")+1,string.length());
		HeaderInfo.filter=string.c_str();
	}

	fits_read_card(fptr,"DATE", card, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword DATE "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		HeaderInfo.date="None";
		HeaderInfo.jd=0;
		status = 0;
	} else {
		fits_parse_value(card, keyvalue, keycomment, &status);
		string=keyvalue;
		string.erase(0,1);
		string.erase(string.find_last_not_of("' ")+1,string.length());
		HeaderInfo.date=string.c_str();
		HeaderInfo.jd=str2jd(HeaderInfo.date);
	}

	/**
	 *  This is important part.  Determining the image type and then initiate the ImageArray
	 *
	 *  */
	fits_get_img_param(fptr, 9,  &bitpix, &naxis, naxes, &status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) unable to access FITS file parameter "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}
	HeaderInfo.bitpix = bitpix;

	fits_movabs_hdu(fptr, 2, NULL, &status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	fits_get_img_param(fptr, 9,  &bitpix, &naxis, naxes, &status);
	/** If status != 0, then throw an exception and return */
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) unable to access FITS file parameter "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}
	HeaderInfo.naxis = naxis;
	HeaderInfo.naxis1 = naxes[0];
	HeaderInfo.naxis2 = naxes[1];
	HeaderInfo.naxis3 = naxes[2];

	fits_read_key_lng(fptr, "BZERO", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword BZERO "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.bzero = intnum;

	fits_read_key_lng(fptr, "CHIPBIAS", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword CHIPBIAS "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.chipbias = intnum;


	fits_read_key(fptr,TSHORT, "SCFOWLER", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword SCFOWLER "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
		HeaderInfo.scfowler = 1;
	}
	HeaderInfo.scfowler = intnum;

	fits_read_key_dbl(fptr, "EXPTIME", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword EXPTIME "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.exptime = dblnum;

	/// Get RA in the unit of degree
	fits_read_key_dbl(fptr, "RA_DEG", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword RA_DEG "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;

	}
	HeaderInfo.ra = dblnum;

	fits_read_key_dbl(fptr, "DEC_DEG", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword RA_DEG "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.dec = dblnum;

	fits_read_key_dbl(fptr, "INSTZRA", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword INSTZRA "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.zra = dblnum;

	fits_read_key_dbl(fptr, "INSTZDEC", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword INSTZDEC "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;

	}
	HeaderInfo.zdec = dblnum;

	fits_read_key_dbl(fptr, "WCROTATE", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCROTATE "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcrotate = dblnum;

	fits_read_key_dbl(fptr, "RDTIME", &dblnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword RDTIME "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.rdtime = dblnum;

	fits_read_key_lng(fptr, "WCPOSX1", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSX1 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposx1 = intnum;

	fits_read_key_lng(fptr, "WCPOSY1", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSY1 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposy1 = intnum;

	fits_read_key_lng(fptr, "WCPOSX2", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSX2 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposx2 = intnum;

	fits_read_key_lng(fptr, "WCPOSY2", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSY2 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposy2 = intnum;

	fits_read_key_lng(fptr, "WCPOSX3", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSX3 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposx3 = intnum;

	fits_read_key_lng(fptr, "WCPOSY3", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSY3 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposy3 = intnum;

	fits_read_key_lng(fptr, "WCPOSX4", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSX4 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposx4 = intnum;

	fits_read_key_lng(fptr, "WCPOSY4", &intnum, NULL, &status);
	if (status != 0){
		fprintf(stderr, "Logonly: (%s:%s:%d) could not find FITS header keyword WCPOSY4 "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		status=0;
	}
	HeaderInfo.wcposy4 = intnum;


	/// Starting extracting extension headers
	fits_get_hdrspace(fptr, &keyexist, &morekeys,&status);
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not access FITS header "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	this->Header.extHeaderKeys=keyexist;


	for (hdupos=2; hdupos<=5; hdupos++){
		control=0;
		fits_movabs_hdu(fptr, hdupos, NULL, &status);

		for (ii = 1; ii <= keyexist; ii++)  {
			fits_read_record(fptr, ii, card, &status); /* read keyword */
			if (strstr (card,"Reserved space") != NULL && control == 0){
				this->Header.firstExtHeaderSpace[hdupos-2]=ii-1;
				control = 1;
			}
			string=card;
			Header.extHeader+=string;
			Header.extHeader+="&";
		}

		/// If the variable control is still zero that means there is no Reserved Space.
		///  So, use the last available position.
		if (control == 0){
			this->Header.firstExtHeaderSpace[hdupos-2]=keyexist-1;
		}

	}

	size=HeaderInfo.naxis1*HeaderInfo.naxis2*HeaderInfo.naxis3*HeaderInfo.nextend;

	switch(bitpix) {
		case BYTE_IMG:
			datatype = BYTE_IMG;
			ImageType = datatype;
			ImageArrayByte = (bool*)calloc(size,sizeof(bool));
			if (ImageArrayByte == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case SHORT_IMG:
			datatype = SHORT_IMG;
			ImageType = datatype;
			ImageArrayInt = (int*)calloc(size,sizeof(int));
			if (ImageArrayInt == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case LONG_IMG:
			datatype = LONG_IMG;
			ImageType = datatype;
			ImageArrayInt = (int*)calloc(size,sizeof(int));
			if (ImageArrayInt == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case FLOAT_IMG:
			datatype = FLOAT_IMG;
			ImageType = datatype;
			ImageArrayFloat = (float*)calloc(size,sizeof(float));
			if (ImageArrayFloat == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case DOUBLE_IMG:
			datatype = DOUBLE_IMG;
			ImageType = datatype;
			ImageArrayFloat = (float*)calloc(size,sizeof(float));
			if (ImageArrayFloat == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
	}
}


/** Return a image array of FITS file
 *
 * \return int if the FITS file is closed
 */
ImageInfo Image::getImageHeader(){
	return HeaderInfo;
}
/** This method read the image in FITS into a image array in 1-D.
 *
 *   \return int if the method is done without error
 */
int Image::setImageArray(){
	double nulval=0.0;
	int bitpix,anynul,npix=0,bytepix;
	int naxis;
	long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	float *array=NULL;
	int hdupos,hdutype,status=0;
	int size;
	int i;

	size = HeaderInfo.nextend*HeaderInfo.naxis1*HeaderInfo.naxis2*HeaderInfo.naxis3;
	array = (float *)malloc(size*sizeof(float));
	if (array == NULL)  {
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}


	/* Main loop through each extension for image size*/
	for (hdupos=1; !status; hdupos++){

		fits_movabs_hdu(fptr, hdupos, &hdutype, &status);
		if (status != 0){
			fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(status);
		}

		/* Make sure the file is image */
		fits_get_hdu_type(fptr, &hdutype, &status);
		if (hdutype != IMAGE_HDU){
			fprintf(stderr, "Error: (%s:%s:%d) FITS extension is not IMAGE "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(NOT_IMAGE);
		}

		/* Determine the image size */
		fits_get_img_param(fptr, 9,  &bitpix,&naxis, naxes, &status);
		if (status != 0){
			fprintf(stderr, "Error: (%s:%s:%d) can not get image parameter "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(status);
		}
		npix = naxes[0] * naxes[1] * naxes[2] * naxes[3] * naxes[4]
			 * naxes[5] * naxes[6] * naxes[7] * naxes[8];


		/* Start to operate on image when hdupos and npix are greater than 1 */
		if (npix > 1){
			bytepix = abs(bitpix) / 8;


			/* read all or part of image */
			fits_read_img(fptr, TFLOAT, 1, npix, &nulval, array, &anynul, &status);
			if (status != 0){
				fprintf(stderr, "Error: (%s:%s:%d) can not read image array into memory "
						"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(status);
			}

			/* Also remove the chipbias */
			for(i=0;i<npix;i++){
				//if (i==2048500){printf("2 value=%f, %f\n",array[i],(double)HeaderInfo.chipbias);}

				setImageArrayValue((hdupos-2)*npix+i,array[i]-(double)HeaderInfo.chipbias);
			}

		}
		fits_movrel_hdu(fptr, 1, &hdutype, &status);
	}


	free(array);
	return EXIT_SUCCESS;

}


/**
 *
 */
void Image::setImageArrayValue(int n, double value){
	switch(ImageType) {
		case BYTE_IMG:
			if (value > 1){
				ImageArrayByte[n]=1;
			} else {
				ImageArrayByte[n]=0;
			}
			break;
		case SHORT_IMG:
			ImageArrayInt[n]=(int)(floor(value));
			break;
		case LONG_IMG:
			ImageArrayInt[n]=(int)(floor(value));
			break;
		case FLOAT_IMG:
			ImageArrayFloat[n]=(float)value;
			break;
		case DOUBLE_IMG:
			ImageArrayFloat[n]=value;
			break;
	}
}

double Image::getImageArrayValue(int n){
	double val=-999999.0;
	switch(ImageType) {
		case BYTE_IMG:
			val=(double)ImageArrayByte[n];
			break;
		case SHORT_IMG:
			//if (n==2048500){printf("1 value=%i\n",this->ImageArrayInt[n]);}
			val=(double)ImageArrayInt[n];
			break;
		case LONG_IMG:
			val=(double)ImageArrayInt[n];
			break;
		case FLOAT_IMG:
			//if (n==2048500){printf("1 value=%i\n",this->ImageArrayFloat[n]);}
			val=(double)ImageArrayFloat[n];
			break;
		case DOUBLE_IMG:
			val=(double)ImageArrayFloat[n];
			break;
	}
	return val;
}

/** This function opens a fits file and put the point of to the FITS file to
 *   a pointer.
 * \param filename is a pointer of char to the file
 * \return int if the FITS file is closed
 */
int Image::SaveImage16B(char *filename){
	string token;
	int keyexist,morekeys,ii;
	char card[FLEN_CARD];

	int status=0,hdupos,npix;

	int i;
	long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	unsigned short *array;
	double val;


	/** Remove the file if it exists*/
	remove(filename);

	/** Create an image file on disk */
	fits_create_file(&outfptr, filename, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not get create image in disk "
				"in %s.\n", __FILE__, __func__, __LINE__,filename);
		throw FITSIOException(status);
	}

	/** Copy image header to new image */
	fits_movabs_hdu(fptr, 1, NULL, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}

	fits_copy_header(fptr, outfptr, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not copy FITS header to disk "
				"from %s to %s.\n", __FILE__, __func__, __LINE__,getBaseName(),filename);
		throw FITSIOException(status);
	}

	/// Count the number of keys in this header unit
	fits_get_hdrspace(outfptr, &keyexist, &morekeys,&status);
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not access FITS header "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	/// Starting updating output FITS header based on Header information
	ii=1;

	std::istringstream miss(this->Header.mainHeader);
	while(getline(miss,token,'&')){
		fits_read_record(outfptr, ii, card, &status); /* read keyword */
		if (token.compare(card) != 0){
			fits_delete_record(outfptr, ii,&status);
			fits_insert_record(outfptr, ii, (char *)token.c_str(), &status);
		}
	    ii++;
	}
	npix=HeaderInfo.naxis1*HeaderInfo.naxis2*HeaderInfo.naxis3;
	array = (unsigned short *)malloc( npix* sizeof( unsigned short ) );

	if (array == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}


	/* Main loop through each extension for image size*/
	for (hdupos=2; hdupos<=5; hdupos++){
		fits_movabs_hdu(fptr, hdupos, NULL, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}
		fits_copy_header(fptr, outfptr, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not copy FITS header to disk "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}

		fits_update_key_lng(outfptr,"BZERO",32768,NULL,&status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not change value for FITS keyword BZERO "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}

		/** Check if keyword NAXIS3 presented */
		fits_read_card(outfptr, "NAXIS3", card, &status);
		if (status != 0){
			/** if status != 0 that means NAXIS3 keyword is not there, so do nothing but set status = 0 */
			status=0;
		} else {
			status=0;
			fits_update_key_lng(outfptr,"NAXIS3",HeaderInfo.naxis3,NULL,&status);
		}


		naxes[0]=HeaderInfo.naxis1;
		naxes[1]=HeaderInfo.naxis2;
		naxes[2]=HeaderInfo.naxis3;
		for (i=0;i<npix;i++){
			val=(getImageArrayValue((hdupos-2)*npix+i)+HeaderInfo.chipbias);
			array[i]=(unsigned short)lround(val);
			//if (i==204800){printf("value=%f\n",getImageArrayValue((hdupos-2)*npix+i));}
		}
		fits_write_img(outfptr, TUSHORT, 1, npix, array, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not write image array to disk "
					"in %s.\n", __FILE__, __func__, __LINE__,filename);
			throw FITSIOException(status);
		}
	}
	/// Updating FITS headers in extensions
	hdupos=1;

	std::istringstream eiss(this->Header.extHeader);
	while(getline(eiss,token,'&')){

		if (token.find("XTENSION") != token.npos){
			hdupos++;
			fits_movabs_hdu(outfptr, hdupos, NULL, &status);
			if (status != 0) {
				fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
						"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
				throw FITSIOException(status);
			}
			fits_get_hdrspace(outfptr, &keyexist, &morekeys,&status);
			ii=1;
		}
		fits_read_record(outfptr, ii, card, &status); /* read keyword */
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not read FITS header  "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}

		if (token.compare(card) != 0){
			fits_delete_record(outfptr, ii,&status);
			if (status != 0) {
				fprintf(stderr, "Error: (%s:%s:%d) can not delete keyword in FITS header "
						"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
				throw FITSIOException(status);
			}

			fits_insert_record(outfptr, ii, (char *)token.c_str(), &status);

			if (status != 0) {
				fprintf(stderr, "Error: (%s:%s:%d) can not insert a FITS keyword in "
						"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
				throw FITSIOException(status);
			}
		}
		ii++;
	}

	free(array);
	return EXIT_SUCCESS;
};

/** This function opens a fits file and put the point of to the FITS file to
 *   a pointer.
 * \param filename is a pointer of char to the file
 * \return int if the FITS file is closed
 */

int Image::SaveImage32B(char *filename){
	int keyexist,morekeys,ii;
	char card[FLEN_CARD];
	string token;
	int status=0,hdupos,npix;

	int i;
	long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	float *array, val;
	/* Remove the file if it exists*/
	remove(filename);

	/** Create an image file on disk */
	fits_create_file(&outfptr, filename, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not create FITS image in disk "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}

	/** Copy image header to new image */
	fits_movabs_hdu(fptr, 1, NULL, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}
	fits_copy_header(fptr, outfptr, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not copy FITS header to disk "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}

	/// Count the number of keys in this header unit
	fits_get_hdrspace(outfptr, &keyexist, &morekeys,&status);
	if (status != 0){
		fprintf(stderr, "Error: (%s:%s:%d) can not access FITS header "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(status);
	}

	/// Starting updating output FITS header based on Header information
	ii=1;
	std::istringstream miss(this->Header.mainHeader);
	while(getline(miss,token,'&')){
		fits_read_record(outfptr, ii, card, &status); /* read keyword */
		if (token.compare(card)){
			fits_delete_record(outfptr, ii,&status);
			fits_insert_record(outfptr, ii, (char *)token.c_str(), &status);
		}
	    ii++;
	}

	/* Update important FITS keywords */
	fits_modify_key_lng(outfptr,"BITPIX",FLOAT_IMG,NULL,&status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not modify value of FITS header BITPIX "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}
	fits_modify_key_lng(outfptr,"CHIPBIAS",0,NULL,&status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not modify value of FITS header CHIPBIAS "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}

	npix=HeaderInfo.naxis1*HeaderInfo.naxis2*HeaderInfo.naxis3;
	array = (float *)malloc( npix* sizeof(float) );
	if (array == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}

	/* Main loop through each extension for image size*/
	for (hdupos=2; hdupos<=5; hdupos++){
		fits_movabs_hdu(fptr, hdupos, NULL, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}
		fits_copy_header(fptr, outfptr, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not copy FITS header to disk "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}

		/* Update important FITS keywords */
		fits_modify_key_lng(outfptr,"BITPIX",FLOAT_IMG,NULL,&status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not modify value of FITS header BITPIX "
					"in %s.\n", __FILE__, __func__, __LINE__,filename);
			throw FITSIOException(status);
		}
		this->updateExtensionHeader(hdupos-1,"CHIPBIAS",0.0,"Science frame chip bias added to CDS value");

		/** Check if keyword NAXIS3 presented */
		fits_read_card(outfptr, "NAXIS3", card, &status);
		if (status != 0){
			/** if status != 0 that means NAXIS3 keyword is not there, so do nothing but set status = 0 */
			status=0;
		} else {
			status=0;
			fits_update_key_lng(outfptr,"NAXIS3",HeaderInfo.naxis3,NULL,&status);
		}

		naxes[0]=HeaderInfo.naxis1;
		naxes[1]=HeaderInfo.naxis2;
		naxes[2]=HeaderInfo.naxis3;

		for (i=0;i<npix;i++){
			val=this->getImageArrayValue((hdupos-2)*npix+i);
			if (isnan(val) || val == -HeaderInfo.chipbias){
				val = 0;
			}
			array[i]=val;
		}
		fits_write_img(outfptr, TFLOAT, 1, npix, array, &status);
	}


	/// Updating FITS headers in extensions
	hdupos=1;
	std::istringstream eiss(this->Header.extHeader);
	while(getline(eiss,token,'&')){
		if (token.find("XTENSION") != token.npos){
			hdupos++;
			fits_movabs_hdu(outfptr, hdupos, NULL, &status);
			fits_get_hdrspace(outfptr, &keyexist, &morekeys,&status);
			ii=1;
		}
		fits_read_record(outfptr, ii, card, &status); /* read keyword */
		if (!strstr(card,"BITPIX")){
			if (token.compare(card) != 0){
				fits_delete_record(outfptr, ii,&status);
				fits_insert_record(outfptr, ii, (char *)token.c_str(), &status);
			}
		}
		ii++;
	}

	free(array);
	return EXIT_SUCCESS;
};

void Image::initImageBackground(){
	int i;
	ImageBackground=(skybackground_t *)malloc(this->HeaderInfo.naxis3*sizeof(skybackground_t));
	if (ImageBackground == NULL ){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}
	for (i=1;i<=HeaderInfo.naxis3;i++){
		ImageBackground[i-1].skylvl=(float *)malloc(MaxChipInstalled*sizeof(float));
		ImageBackground[i-1].skydev=(float *)malloc(MaxChipInstalled*sizeof(float));
		ImageBackground[i-1].skyrate=(float *)malloc(MaxChipInstalled*sizeof(float));
	}
	flagImageBackground=1;
}


void Image::freeImageBackground(){
	int i;
	for (i=1;i<=HeaderInfo.naxis3;i++){
		free(ImageBackground[i-1].skylvl);
		free(ImageBackground[i-1].skydev);
		free(ImageBackground[i-1].skyrate);
	}
	free(ImageBackground);
}
/**
 * This method is used to calculate the median and median deviation values as indicators of
 *   image background level.
 */
int Image::setImageMedian(){

	//ImageBackground=NULL;
	double *arr=NULL;
	double median,meddev,dblnum;
	int s=0, status=0,i;
	int ext,size;
	char skylvl[80]="",skydev[80]="";

	initImageBackground();

	/* First of all, test if the header contain the SKY* related headers */
	/*  the SKY* keywords will be always in header unit 2 */
	fits_movabs_hdu(fptr, 2, NULL, &status);
	if (status != 0) {
		fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
				"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
		throw FITSIOException(status);
	}
	fits_read_key_dbl(fptr, "SKYLVL01", &dblnum, NULL, &status);


	if (status != 0){
		#ifdef DEBUG
			printf("No header SKY* keyword, estimate image median.\n");
		#endif

		size=this->HeaderInfo.naxis1*this->HeaderInfo.naxis2;
		arr=(double*)calloc(size,sizeof(double));
		if (arr == NULL ){
			fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
					"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(MemoryAllocateError);
		}


		/* Go through each extension */
		for (ext=1;ext<=this->HeaderInfo.nextend;ext++){
			/* Go through each slice */
			for (s=1;s<=this->HeaderInfo.naxis3;s++){

				for (i=0;i<size;i++){
					arr[i]=this->getImageArrayValue(((ext-1)*this->HeaderInfo.naxis3+(s-1))*size+i);
				}

				median=GetMedian(arr,size);
				meddev=GetMedianDev(arr,size,median);

				#ifdef DEBUG
				printf("Image median=%f deddev=%f\n",median,meddev);
				#endif

			 if (ext == 1){
				(this->ImageBackground)[s-1].exptime=this->HeaderInfo.exptime;
			 }
			 if (HeaderInfo.naxis3 == 1){
				(ImageBackground)->skylvl[ext-1]=median;
				(ImageBackground)->skydev[ext-1]=meddev;
				(ImageBackground)->skyrate[ext-1]=median/HeaderInfo.exptime;
			 } else {
				(ImageBackground)[s-1].skylvl[ext-1]=median;
				(ImageBackground)[s-1].skydev[ext-1]=meddev;
				(ImageBackground)[s-1].skyrate[ext-1]=median/HeaderInfo.exptime;
			 }
		  }
		}
		free(arr);
	} else {
		fits_movabs_hdu(fptr, 1, NULL, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}
		fits_read_key_dbl(fptr, "EXPTIME", &dblnum, NULL, &status);
		ImageBackground->exptime=dblnum;

		/// Go through each extension
		for (ext=1;ext<=HeaderInfo.nextend;ext++){
			#ifdef DEBUG
				printf("SKY* keyword exists, read from header.\n");
			#endif

			/// Get information from PHU
			fits_movabs_hdu(fptr, ext+1, NULL, &status);
			if (status != 0) {
				fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
						"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
				throw FITSIOException(status);
			}

			if (HeaderInfo.naxis3 == 1){
				sprintf(skylvl,"%s%02d","SKYLVL",1);
				sprintf(skydev,"%s%02d","SKYDEV",1);
				fits_read_key_dbl(fptr, skylvl, &dblnum, NULL, &status);
				ImageBackground->skylvl[ext-1]=dblnum;
				ImageBackground->skyrate[ext-1]=dblnum/HeaderInfo.exptime;

				fits_read_key_dbl(fptr, skydev, &dblnum, NULL, &status);
				ImageBackground->skydev[ext-1]=dblnum;

			} else {
				/// Go through each slices
				for (s=1;s<=this->HeaderInfo.naxis3;s++){
					sprintf(skylvl,"%s%02d","SKYLVL",s);
					sprintf(skydev,"%s%02d","SKYDEV",s);
					fits_read_key_dbl(fptr, skylvl, &dblnum, NULL, &status);
					ImageBackground[s-1].skylvl[ext-1]=dblnum;
					ImageBackground[s-1].skyrate[ext-1]=dblnum/HeaderInfo.exptime;

					fits_read_key_dbl(fptr, skydev, &dblnum, NULL, &status);
					ImageBackground[s-1].skydev[ext-1]=dblnum;
				}
			}
		}

	}

	return EXIT_SUCCESS;
}

/**
 *  This function is used to get the sky level of each frame.  It checks the header first if the sky matrix is not
 *    there, setImageMedian is called.
 */
skybackground_t* Image::getImageMedian(){
	return ImageBackground;
}

/**
 *  This function is used to update the main FITS header of a raw image.
 *
 */
int Image::updateMasterHeader(char* keyword,char* string,char* comment){
	char newcard[FLEN_CARD];
	char* pch;
	int i=0;

	char HeaderTemp[MaxHeaderKeys*FLEN_CARD];

	/// Clean this variable to ensure this is empty all the time!!
	HeaderTemp[0] = '\0';

	pch = strtok((char *)(this->Header.mainHeader.c_str()),"&");
	while (pch != NULL){
		if (i == this->Header.firstMainHeaderSpace){
			sprintf(newcard,"%-7s = '%19s'/ %s",keyword,string,comment);
			strcat(HeaderTemp,newcard);
			strcat(HeaderTemp,"&");
		} else {
			strcat(HeaderTemp,pch);
			strcat(HeaderTemp,"&");
		}
		pch = strtok (NULL, "&");
	    i++;
	}

	this->Header.mainHeader=HeaderTemp;

	/// Adjust the number for next available key position
	this->Header.firstMainHeaderSpace++;

	return EXIT_SUCCESS;
}

/**
 *  This function is used to update the main FITS header of a raw image.
 *
 */
int Image::updateMasterHeader(char* keyword,double value,char* comment){
	char newcard[FLEN_CARD];
	char* pch;
	int i=0;
	char HeaderTemp[MaxHeaderKeys*FLEN_CARD];

	/// Clean this variable to ensure this is empty all the time!!
	HeaderTemp[0] = '\0';

	pch = strtok((char *)(this->Header.mainHeader.c_str()),"&");
	while (pch != NULL){
		if (i == this->Header.firstMainHeaderSpace){
			if (value < 0.00001){
				sprintf(newcard,"%-8s= %20.4E / %s",keyword,value,comment);
			} else {
				sprintf(newcard,"%-8s= %20.6f / %s",keyword,value,comment);
			}
			strcat(HeaderTemp,newcard);
			strcat(HeaderTemp,"&");
		} else {
			strcat(HeaderTemp,pch);
			strcat(HeaderTemp,"&");
		}
		pch = strtok (NULL, "&");
	    i++;
	}

	this->Header.mainHeader=HeaderTemp;

	/// Adjust the number for next available key position
	this->Header.firstMainHeaderSpace++;

	return EXIT_SUCCESS;
}


/**
 *  This function is used to update the extension FITS header of a raw image.
 *
 */
int Image::updateExtensionHeader(int ext, char* keyword,char* string,char* comment){
	char newcard[FLEN_CARD];
	char* pch;
	int i=0, extpos=0,flag=0;
	char HeaderTemp[MaxChipInstalled*MaxHeaderKeys*FLEN_CARD];

	/// Clean this variable to ensure this is empty all the time!!
	HeaderTemp[0] = '\0';

	pch = strtok((char *)(this->Header.extHeader.c_str()),"&");
	while (pch != NULL){
		if (strstr(pch,"XTENSION") != NULL){
			extpos++;
			i=0;
		}

		if (extpos == ext && (i == this->Header.firstExtHeaderSpace[extpos-1] || strstr(pch,keyword)) && flag == 0){
			sprintf(newcard,"%-8s= '%-19s'/ %s",keyword,string,comment);
			strcat(HeaderTemp,newcard);
			strcat(HeaderTemp,"&");
			flag=1;
			if (i == this->Header.firstExtHeaderSpace[extpos-1]){
				this->Header.firstExtHeaderSpace[ext-1]++;
			}
		} else {
			strcat(HeaderTemp,pch);
			strcat(HeaderTemp,"&");

		}
		pch = strtok (NULL, "&");
	    i++;
	}


	Header.extHeader=HeaderTemp;

	/// Adjust the number for next available key position
	this->Header.firstMainHeaderSpace++;

	return EXIT_SUCCESS;
}

/**
 *  This function is used to update the extension FITS header of a raw image.
 *
 */
int Image::updateExtensionHeader(int ext, char* keyword,double value,char* comment){
	char newcard[FLEN_CARD];
	char* pch;
	int i=0, extpos=0,flag=0;
	char HeaderTemp[MaxChipInstalled*MaxHeaderKeys*FLEN_CARD];

	/// Clean this variable to ensure this is empty all the time!!
	HeaderTemp[0] = '\0';



	pch = strtok((char *)(this->Header.extHeader.c_str()),"&");
	while (pch != NULL){
		if (strstr(pch,"XTENSION") != NULL){
			extpos++;
			i=0;
		}

		if (extpos == ext && (i == this->Header.firstExtHeaderSpace[extpos-1] || strstr(pch,keyword)) && flag == 0){
			if (value != 0 && value < 0.0001){
				sprintf(newcard,"%-8s= %20.4E / %s",keyword,value,comment);
			} else if (value == 0.0 || value- floor(value) == 0) {
				sprintf(newcard,"%-8s= %20i / %s",keyword,(int)value,comment);
			} else {
				sprintf(newcard,"%-8s= %20.4f / %s",keyword,value,comment);
			}
			strcat(HeaderTemp,newcard);
			strcat(HeaderTemp,"&");
			flag=1;

			/// Adjust the number for next available key position
			if (i == this->Header.firstExtHeaderSpace[extpos-1]){
				this->Header.firstExtHeaderSpace[ext-1]++;
			}
		} else {
			strcat(HeaderTemp,pch);
			strcat(HeaderTemp,"&");

		}
		pch = strtok (NULL, "&");
	    i++;
	}

	Header.extHeader=HeaderTemp;

	/// Adjust the number for next available key position
	this->Header.firstMainHeaderSpace++;

	return EXIT_SUCCESS;
}

/**
 *  This function is used to update the FITS header of a detrended image.
 *
 */
int Image::updateSkylevelHeader(){
	int ext,s;
	char skylvl[80];
	char skydev[80];


	for (ext=1;ext<=this->getImageHeader().nextend;ext++){

		for (s=0;s<HeaderInfo.naxis3;s++){
			sprintf(skylvl,"%s%02d","SKYLVL",s+1);
			sprintf(skydev,"%s%02d","SKYDEV",s+1);
			this->updateExtensionHeader(ext,skylvl, ImageBackground[s].skylvl[ext-1],"Measured sky level (ADU)");
			this->updateExtensionHeader(ext,skydev, ImageBackground[s].skydev[ext-1],"Measured sky level deviations (ADU)");
		}
	}
	return EXIT_SUCCESS;
}
/**
 * This function removes all the reserved space in FITS headers.
 */
int Image::cleanDetrendHeader(){
	int status=0,ext,keynum,i;
	int keyexist,morekeys;
	char card[FLEN_CARD];

	for (ext=1;ext<=this->HeaderInfo.nextend+1;ext++){
		fits_movabs_hdu(outfptr, ext, NULL, &status);
		if (status != 0) {
			fprintf(stderr, "Error: (%s:%s:%d) can not move FITS header to next unit "
					"in %s.\n", __FILE__, __func__, __LINE__,getBaseName());
			throw FITSIOException(status);
		}

		/// Clean the Reserved Space in FITS header for future usage
		fits_get_hdrspace(outfptr, &keyexist, &morekeys,&status);
		if (status != 0){
			fprintf(stderr, "Error: (%s:%s:%d) can not access FITS header "
					"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
			throw FITSIOException(status);
		}
		keynum=1;
		while (keynum<keyexist){
			fits_read_record(outfptr, keynum, card, &status); /* read keyword */
			if (strstr (card,"Reserved space") != NULL){
				break;
			}
			keynum++;
		}

		for(i=keynum-1;i<keyexist;i++){
			fits_delete_record(outfptr, keynum,&status);
		}

	}
	return EXIT_SUCCESS;
}

/**
 *  This is a object initializer other than default constructor.  There is one more step taken, setImageMadian.
 */
void Image::setImage(char* filename){

	/* Set outfptr to NULL to avoid memory problem. */
	this->outfptr=NULL;

	setFilename(filename);
	try{
		openFitfile();
		setImageHeader();
		setImageArray();
		setImageMedian();
	}
	catch(FITSIOException &e){
		throw(e);
	}

}

Image::Image(){
	flagImageBackground=0;
}

/** This the default constructor of Image class.
 *
 * \param filename : the filename of FITS file
 */
Image::Image(char* filename){
	flagImageBackground=0;
	/* Set outfptr to NULL to avoid memory problem. */
	this->outfptr=NULL;


	setFilename(filename);
	try{
		openFitfile();
		setImageHeader();
		setImageArray();
	}
	catch(FITSIOException &e){
		throw(e);
	}
}


/**
 *  This destructor release the the image array and close the opened fitsfile.
 */
Image::~Image(){
	closeFitfile();
	switch(ImageType) {
		case BYTE_IMG:
			free(ImageArrayByte);
			break;
		case SHORT_IMG:
			free(ImageArrayInt);
			break;
		case LONG_IMG:
			free(ImageArrayInt);
			break;
		case FLOAT_IMG:
			free(ImageArrayFloat);
			break;
		case DOUBLE_IMG:
			free(ImageArrayFloat);
			break;
	}
	if (flagImageBackground == 1){
		freeImageBackground();
	}
	//delete(ce);
}



void StackImage::setStackImage(){
	int ext,offset;
	int j,nslice,slice;
	double* array=NULL;
	double temp[HeaderInfo.naxis3];

	array=(double *)malloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize*sizeof(double));
	if (array == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}


	offset=MaxImageXSize*MaxImageYSize;
	nslice=this->getImageHeader().naxis3;


	if (HeaderInfo.naxis3 > 1){
		for (ext=0;ext<MaxChipInstalled;ext++){
			for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
				for (slice=0;slice<HeaderInfo.naxis3;slice++){
					temp[slice]=this->getImageArrayValue((ext*nslice+slice)*offset+j);
				}
				array[ext*offset+j]=GetMedian(temp,nslice);
			}
		}
	} else {
		for (ext=0;ext<MaxChipInstalled;ext++){
			for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
				array[ext*offset+j]=this->getImageArrayValue(ext*offset+j);
			}
		}

	}

	switch(ImageType) {
		case BYTE_IMG:
			free(ImageArrayByte);
			ImageArrayByte = (bool *)calloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize,sizeof(bool));
			if (ImageArrayByte == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case SHORT_IMG:
			free(ImageArrayInt);
			ImageArrayInt = (int *)calloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize,sizeof(int));
			if (ImageArrayInt == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case LONG_IMG:
			free(ImageArrayInt);
			ImageArrayInt = (int *)calloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize,sizeof(int));
			if (ImageArrayInt == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case FLOAT_IMG:
			free(ImageArrayFloat);
			ImageArrayFloat = (float *)calloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize,sizeof(float));
			if (ImageArrayFloat == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
		case DOUBLE_IMG:
			free(ImageArrayFloat);
			ImageArrayFloat = (float *)calloc(MaxChipInstalled*MaxImageXSize*MaxImageYSize,sizeof(float));
			if (ImageArrayFloat == NULL){
				fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
						"for %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
				throw FITSIOException(MemoryAllocateError);
			}
			break;
	}

	for (ext=0;ext<MaxChipInstalled;ext++){
		for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
			//this->setImageArrayValue(j,array[j]);
			this->setImageArrayValue(ext*offset+j,(double)array[ext*offset+j]);
		}
		this->updateExtensionHeader(ext+1,"NAXIS3",1," length of data axis 3");
	}

	HeaderInfo.naxis3=1;


	free(array);

}


void setCrunchMethod(){
}

StackImage::StackImage(){}

StackImage::StackImage(char* filename): super(filename){

	/** Check how many slice in this file. If NAXIS3 = 1, then stopping stacking */
	if (HeaderInfo.naxis3 == 1){
		fprintf(stderr, "Logonly: (%s:%s:%d) this is a single sliced images "
				"in %s.\n", __FILE__, __func__, __LINE__,this->getBaseName());
		//throw FITSIOException(BAD_NAXIS);
	}

	try {
		setStackImage();
	} catch(FITSIOException &e){
		throw(e);
	}


}


/**
 *  This destructor release the the image array and close the opened fitsfile.
 */
StackImage::~StackImage(){

}












