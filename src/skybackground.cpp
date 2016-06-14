/**
 * skybackground.cpp
 *
 *  Created on: Dec 2, 2010
 *      Author: chyan
 */

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "skybackground.h"
#include "imexception.h"
#include "imparameter.h"

using namespace std;

/** Set the file list for skybackground
 *  \param list is a pointer of char to the file list
 */
void SkyBackground::setFileList(char* list){
	FileList = list;
}

/** Get the file list for skybackground
 *
 */
char* SkyBackground::getFileList(){
	return FileList;
}

/** Set the file list for bad pixel mask
 *  \param list is a pointer of char to the file list
 */
void SkyBackground::setMaskList(char* list){
	MaskList = list;
}

/** Get the file list for bad pixel mask
 *
 */
char* SkyBackground::getMaskList(){
	return MaskList;
}
/** Set the number of sky frames based on input files
 *  \param
 */
void SkyBackground::setNSkyFITS(int nsky){
	nSkyFITS=nsky;
}

/** Get the number of sky frames
 *
 */
int SkyBackground::getNSkyFITS(){
	return nSkyFITS;
}
/** Set the number of bad pixel frames based on input files
 *  \param
 */
void SkyBackground::setNMaskFITS(int nsky){
	nMaskFITS=nsky;
}

/** Get the number of bad pixel frames
 *
 */
int SkyBackground::getNMaskFITS(){
	return nMaskFITS;
}
/**
 *  Setting an array for storing detrend FITS files
 */
void SkyBackground::setDetrendFile(char* list){
	int n=0;
	parseList(this->FileList,DetrendFile,&n);
	setNSkyFITS(n);
}
/**
 *  getting the name of detrended FITS file based on array index
 */
char* SkyBackground::getDetrendFile(int i){
	return this->DetrendFile[i];
}

/**
 *  Setting an array for storing bad pixel masks
 */
void SkyBackground::setMaskFile(char* list){
	int n=0;
	parseList(this->MaskList,MaskFile,&n);
	setNMaskFITS(n);
}
/**
 * Getting the name of bad pixel masks based on array index
 */
char* SkyBackground::getMaskFile(int i){
	return this->MaskFile[i];
}

/**
 * Setting image objects for sky frames
 */
void SkyBackground::setDetrendImage(){
	int n=0;
	DetrendImage = new Image[getNSkyFITS()];
	for (n=0;n<this->nSkyFITS;n++){
		DetrendImage[n].setImage(getDetrendFile(n));
		cout << "Loading "<< DetrendImage[n].getFilename()<< '\n';
	}
}

/**
 * Setting image objects for mask/bad pixel files
 */
void SkyBackground::setMaskImage(){
	int n;
	MaskImage = new Image[getNMaskFITS()];
	for (n=0;n<this->nMaskFITS;n++){
		MaskImage[n].setImage(getMaskFile(n));
		cout << "Loading "<< getMaskFile(n)<< '\n';
	}
}


void SkyBackground::setTotalFrameNumber(){
	int i;
	TotalFrameNumber=0;
	for (i=0;i<this->nSkyFITS;i++){
		TotalFrameNumber=TotalFrameNumber+DetrendImage[i].getImageHeader().naxis3;
	}
}

int SkyBackground::getTotalFrameNumber(){
	return TotalFrameNumber;
}

/**
 * Parse the string list to array
 */
void SkyBackground::parseList(char *liststr,char* filelist[], int* n){
	int i=0;
	char* pch;
	pch = strtok (liststr,",");
	filelist[i]=pch;
	while (pch != NULL){
		filelist[i]=pch;
		pch = strtok (NULL, ",");
		i++;
	}
	*n=i;
}

/**
 * This function is used for apply bad pixel to detrend data
 */
void SkyBackground::applyMasktoDetrend(){
	int n,i,size;

	#ifdef DEBUG
	printf("Masking flatten images.\n");
	#endif


	for (n=0;n<nSkyFITS;n++){
		MaskImage = new Image(getMaskFile(n));
		/** Loading Mask Images */
		//MaskImage->setImage(getMaskFile(n));
		size=DetrendImage[n].getImageHeader().nextend*DetrendImage[n].getImageHeader().naxis1*
				DetrendImage[n].getImageHeader().naxis2*DetrendImage[n].getImageHeader().naxis3;
		for (i=0;i<size;i++){
			if (MaskImage->getImageArrayValue(i) == 0){
				DetrendImage[n].setImageArrayValue(i,0);
			}
		}
		delete(MaskImage);
	}
}

/** This function calculate QE ration for each chip using
 *  skyrate
 */
void SkyBackground::calcQeRatio(){

	int i=0,fits=0,slice=0;
	int ext=0;
	double temp=0;
	double *arr=NULL;

	arr=(double *)malloc(TotalFrameNumber*sizeof(double));

	for (ext=0;ext<MaxChipInstalled;ext++){
		i=0;
		for (fits=0;fits<nSkyFITS;fits++){
			for (slice=0;slice<DetrendImage[fits].getImageHeader().naxis3;slice++){
				arr[i]=DetrendImage[fits].getImageMedian()[slice].skyrate[ext];
				i++;
			}
		}
		qeratio[ext]=GetMedian(arr,TotalFrameNumber);
		temp=temp+qeratio[ext];
	}

	temp=temp/MaxChipInstalled;

	for (ext=0;ext<MaxChipInstalled;ext++){
		(qeratio)[ext]=(qeratio)[ext]/temp;
	}
	free(arr);

	#ifdef DEBUG
	printf("Finishing QE ration calculation. %f, %f, %f, %f\n",qeratio[0],qeratio[1],qeratio[2],qeratio[3]);
	#endif

}

/* This function will calculate corrected sky level (ADU/pixel/sec)
 *  based on QE ratio
 */
void SkyBackground::correctSkyLevel(){

	int i,ext,slice,fits;
	double temp=0,sum=0;
	double mean;

	skylvl_corrmean=(double *)malloc(TotalFrameNumber*sizeof(double));
	skylvl_var=(double *)malloc(TotalFrameNumber*sizeof(double));

	/* Correct sky level using QE ratio */
	i=0;
	for (fits=0;fits<nSkyFITS;fits++){
		for (slice=0;slice<DetrendImage[fits].getImageHeader().naxis3;slice++){
			temp=0;
			for (ext=0;ext<MaxChipInstalled;ext++){
				temp+=DetrendImage[fits].getImageMedian()[slice].skyrate[ext]/qeratio[ext];
			}
			skylvl_corrmean[i]=temp/MaxChipInstalled;
			sum=sum+skylvl_corrmean[i];
			i++;
		}
	}

	mean=sum/TotalFrameNumber;
	printf("mean=%f\n",mean);
	/** Calculate time variation of sky background */
	for (i=0;i<TotalFrameNumber;i++){
		(skylvl_var)[i]=skylvl_corrmean[i]/mean;
	}

}

/**
 *  Building sky image
 */
void SkyBackground::buildSkyImage(){
	int fits,ext,slice,offset,j,nslice,i=0;
	double arr[TotalFrameNumber];
	double *temp=NULL;
	//double temp[MaxImageXSize*MaxImageYSize];
	double value,med,factor;
	offset=MaxImageXSize*MaxImageYSize;


	temp=(double*)malloc(MaxImageXSize*MaxImageYSize*sizeof(double));
	if (temp == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space"
				".\n", __FILE__, __func__, __LINE__);
		throw FITSIOException(MemoryAllocateError);
	}

	#ifdef DEBUG
	printf("Starting to building sky frames.\n");
	#endif


	/* Correcting sky variance*/
	for (fits=0;fits<nSkyFITS;fits++){
		nslice=DetrendImage[fits].getImageHeader().naxis3;
		for (slice=0;slice<nslice;slice++){
			factor=DetrendImage[fits].getImageHeader().exptime*skylvl_var[i];
			for (ext=0;ext<MaxChipInstalled;ext++){
				/**
				 *   The value of each pixel is normalized with exposure time and sky variance.
				 *   This step is used for correction the difference between sky levels.
				 */
				for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
					value=DetrendImage[fits].getImageArrayValue((ext*nslice+slice)*offset+j)/factor;///
					DetrendImage[fits].setImageArrayValue((ext*nslice+slice)*offset+j,value);
				}
			}
			i++;
		}
	}
	/** take median value of all images as sky background */
	for (ext=0;ext<MaxChipInstalled;ext++){
		for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
			i=0;
			for (fits=0;fits<nSkyFITS;fits++){
				nslice=DetrendImage[fits].getImageHeader().naxis3;
				/** Go through all slices */
				for (slice=0;slice<nslice;slice++){
					/**
					 *  Picking-up all pixels that are not marked by NaN, -CHIPBIAS or any negative value.
					 */
					if (DetrendImage[fits].getImageArrayValue((ext*nslice+slice)*offset+j) <= 0){continue;}
					arr[i]=DetrendImage[fits].getImageArrayValue((ext*nslice+slice)*offset+j);
					i++;
				}
			}
			if (i == 0 ){
				skyBackgroundImage[ext*offset+j]=0;
			} else {
				skyBackgroundImage[ext*offset+j]=GetMedian(arr,i);
			}
		}
	}
	/** Normalize sky background to 1 */
	for (ext=0;ext<MaxChipInstalled;ext++){
		offset=ext*MaxImageXSize*MaxImageYSize;
		for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
			temp[j]=skyBackgroundImage[offset+j];
		}

		med=GetMedian(temp,MaxImageXSize*MaxImageYSize);

		#ifdef DEBUG
		printf("med=%f\n",med);
		#endif

		/* Normalize sky background to unity */
		for (j=0;j<MaxImageXSize*MaxImageYSize;j++){
			if (isnan(skyBackgroundImage[offset+j]) ||skyBackgroundImage[offset+j] <= 0){
				skyBackgroundImage[offset+j]=nanf("NaN");
			}
			else{
				skyBackgroundImage[offset+j]=skyBackgroundImage[offset+j]/med;
			}
		}
	}

	free(temp);
}


int SkyBackground::writeSkyBackgroundImage(char* filename){

    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status=0,hdupos,i,fits;
    long  nelements;
    //float array[MaxImageXSize*MaxImageYSize];
    float *array=NULL;
    string string;
    /** initialize FITS image parameters */
    long naxes[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    char skykey[80];

	array=(float*)malloc(MaxImageXSize*MaxImageYSize*sizeof(float));
	if (array == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space"
				".\n", __FILE__, __func__, __LINE__);
		throw FITSIOException(MemoryAllocateError);
	}

    /** Delete old file if it already exists */
    remove(filename);

    /** create new FITS file */
    if (fits_create_file(&fptr, filename, &status))
         printf("Error on creating file. status=%i\n",status );

    /** Copy image header to new image */
	if (fits_movabs_hdu(DetrendImage[0].getInputFITSptr(), 1, NULL, &status)){
		printf("status=%i\n",status );
	}
	fits_copy_header(DetrendImage[0].getInputFITSptr(), fptr, &status);

    for (hdupos=2; hdupos<=5; hdupos++){
		fits_movabs_hdu(DetrendImage[0].getInputFITSptr(), hdupos, NULL, &status);
		fits_copy_header(DetrendImage[0].getInputFITSptr(),fptr, &status);

		/* Update important FITS keywords */
		fits_modify_key_lng(fptr,"NAXIS3",1,NULL,&status);
		fits_modify_key_lng(fptr,"BITPIX",FLOAT_IMG,NULL,&status);
		fits_modify_key_lng(fptr,"BZERO",0,NULL,&status);
		fits_modify_key_lng(fptr,"CHIPBIAS",0,NULL,&status);

		fits_update_key_lng(fptr,"N_ODOSKY",nSkyFITS,"Nbr of odometers used in constructing sky",&status);

		for (fits=0;fits<nSkyFITS;fits++){
			string=this->getDetrendFile(fits);
			sprintf(skykey,"%s%02d","ODOSKY",fits+1);
			fits_update_key_str(fptr,skykey,(char *)string.substr(string.find_last_of("/")+1).c_str(),
					"Odometers used for sky construction",&status);
		}

		naxes[0]=MaxImageXSize;
		naxes[1]=MaxImageYSize;
		naxes[2]=1;

    	nelements = naxes[0] * naxes[1] * naxes[2];
		/* initialize the values  */
		for (i=0;i<nelements;i++){
			array[i]=skyBackgroundImage[(hdupos-2)*nelements+i];
		}

		/* write the array of unsigned integers to the FITS file */
		if ( fits_write_img(fptr, TFLOAT, 1, nelements, array, &status) )
			printf("status=%i\n",status );
    }

    /** close the file */
    if (fits_close_file(fptr, &status)){
    	printf("status=%i\n",status );
    }
    free(array);
    return EXIT_SUCCESS;
}
/**
 *  Default constructor
 */
SkyBackground::SkyBackground(){}

/**
 *  Default constructor
 */
SkyBackground::SkyBackground(char* list){
	this->setFileList(list);

}
/**
 *  Default constructor
 */
SkyBackground::SkyBackground(char* list, char* masklist){
	/** set a string to hold detrend information from command line */
	this->setFileList(list);

	/** set a string to input of mask files from command line */
	this->setMaskList(masklist);

	/** Initiate variable for storing sky vaules */
	skylvl_var=NULL;
	skylvl_corrmean=NULL;

}

SkyBackground::~SkyBackground(){
	if (DetrendImage != NULL){
		delete [] DetrendImage;
	}
	if (skylvl_corrmean != NULL){
		free(skylvl_corrmean);
	}
	if (skylvl_var != NULL){
		free(skylvl_var);
	}
}
