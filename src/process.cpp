/**
 *  \file process.cpp
 *  \brief This file contains the class used for image processing.
 *
 *  \details In this file, the class Process is defined.  This class contains
 *  several method for WIRCam image reduction.  Including reference pixel subtraction,
 *  non-linear correction and image detrending itself.
 *
 *  Created on: Aug 17, 2010
 *      Author: chyan
 */
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <float.h>
#include <math.h>
#include <libgen.h>

#include "CImg.h"
#include "fitsio.h"
#include "image.h"
#include "process.h"
#include "calculate.h"
#include "imparameter.h"
#include "imexception.h"

using namespace std;
using namespace cimg_library;


int Process::subRefPixel(Image *raw){

	int x,y,offset;
	int slices, s, ext=0;

	double* temp =NULL;
	float* final =NULL;

	float med;

	#ifdef DEBUG
		printf("Subtracting reference pixel.\n");
	#endif




	if ((temp=(double*)malloc(raw->getImageHeader().naxis1 * sizeof(double))) == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}
	if ((final=(float*)malloc(raw->getImageHeader().naxis1 * sizeof(float))) == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}

	slices = raw->getImageHeader().naxis3 * raw->getImageHeader().nextend;

	for (s=0;s<slices;s++){
	  /* Calculate the offset of images*/
	  offset = s*raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;
	  /* Loop through all pixels */
	  for (x=0;x<raw->getImageHeader().naxis1;x++){
		 /* Select first and last 4 columm */
		 if (x==0||x==1||x==2||x==3||x==2044||x==2045||x==2046||x==2047){
			/* Take all pixel values and store into buffer */
			for (y=0;y<raw->getImageHeader().naxis2;y++){
			   temp[y] = raw->getImageArrayValue((offset)+x+y*raw->getImageHeader().naxis1);
					   //ImageArray[(offset)+x+y*raw->getImageHeader().naxis1];
			   final[y] = 0;
			}
			med = GetMedian(temp, raw->getImageHeader().naxis1);
			/* Subtract median value */
			for (y=0;y<raw->getImageHeader().naxis2;y++){
			   final[y]=final[y]+(raw->getImageArrayValue((offset)+x+y*raw->getImageHeader().naxis1)-med)/8;
			}
		 }
	  }

	  for (x=4;x<raw->getImageHeader().naxis1-4;x++){
		 for (y=0;y<raw->getImageHeader().naxis2;y++){
			raw->setImageArrayValue((offset)+x+y*raw->getImageHeader().naxis1,
			   raw->getImageArrayValue((offset)+x+y*raw->getImageHeader().naxis1)+final[y]);
		 }
	  }
	}

	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
		raw->updateExtensionHeader(ext,"REFPXCOR","yes","Ref. pixel correction done?");
	}

	free(temp);
	free(final);
	return EXIT_SUCCESS;
}

/**
 *
 */
int Process::NonLinearCorrect(Image *raw){

	int i,ext,size,index;
	//float median,meddev;
	double *cds=NULL;
	float *refc=NULL;
	float *rawc=NULL;

	//struct NonLinearInfo nlcinfo;

	#ifdef DEBUG
	printf("Apply non-linear corrections.\n");
	#endif

	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2*raw->getImageHeader().naxis3;

	/* Go through the NLC date to determine index */
	for (i=0;i<4;i++){
	  if (raw->getImageHeader().jd > nlcinfo.jd[i]){
		 index=i;
	  } else {
		 index=0;
	  }
	}

	#ifdef DEBUG
	printf("Use NLC matrix for %s, the %ith set.\n",(nlcinfo.date[index]),index+1);
	#endif

	refc = (float*)malloc(size*sizeof(float));
	rawc = (float*)malloc(size*sizeof(float));
	cds = (double*)malloc(size*sizeof(double));
	if (refc == NULL || rawc==NULL ||cds==NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}


	/* Go through each extension */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){

		for  (i=0;i<size;i++){
			cds[i]=raw->getImageArrayValue((ext-1)*size+i);
		}

		/* Go through each slice and pixel*/
		for (i=0;i<size;i++){

			refc[i]= raw->getImageArrayValue((ext-1)*size+i)*(float)raw->getImageHeader().rdtime*
			   ((float)raw->getImageHeader().scfowler+1.0)/(raw->getImageHeader().exptime*2.0);  /* Estimate the the REF level*/
			rawc[i]=cds[i]+refc[i];
			refc[i]= refc[i]*(nlcinfo.param[index][ext-1][0] + refc[i]*(nlcinfo.param[index][ext-1][1]+
				 (refc[i]*nlcinfo.param[index][ext-1][2])));
			rawc[i]= rawc[i]*(nlcinfo.param[index][ext-1][0] + rawc[i]*(nlcinfo.param[index][ext-1][1]+
				 (rawc[i]*nlcinfo.param[index][ext-1][2])));

			raw->setImageArrayValue((ext-1)*size+i, (double)(rawc[i] - refc[i]));
		}
	}

	free(cds);
	free(refc);
	free(rawc);

	/// Put information in FITS header
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
		raw->updateExtensionHeader(ext,"NLCORR","yes","Non-linearity correction applied?");
		raw->updateExtensionHeader(ext,"NLC_NAME",nlcinfo.date[index],"Non-linearity solution name");
		raw->updateExtensionHeader(ext,"NLC_FUNC","xc/xm=a0+a1*xm+a2*xm^2","Non-linearity function");
		raw->updateExtensionHeader(ext,"NLC_A0",nlcinfo.param[index][ext-1][0],"Non-linearity function parameters");
		raw->updateExtensionHeader(ext,"NLC_A1",nlcinfo.param[index][ext-1][1],"Non-linearity function parameters");
		raw->updateExtensionHeader(ext,"NLC_A2",nlcinfo.param[index][ext-1][2],"Non-linearity function parameters");
	}
	return EXIT_SUCCESS;
}

int Process::setNLCInfo(){
	int i,j,k;

	/* The first index is the date index, the second is EXTENDION and the third is parameter */
	double param[MaxNLCMatrix][MaxChipInstalled][3] = {
						{{1.0,0.0,0.0},
						{1.0,0.0,0.0},
						{1.0,0.0,0.0},
						{1.0,0.0,0.0}},

						{{0.994684,3.15674e-07,4.56215e-11},
						{0.993023,1.57013e-06,3.68920e-11},
						{0.992029,1.72587e-06,3.43993e-11},
						{0.995496,8.62247e-07,5.30238e-11}},

						{{0.998810,7.92059e-07,5.39334e-11},
						{0.996922,1.28674e-06,4.18800e-11},
						{0.998038,1.15189e-06,4.63836e-11},
						{0.996537,9.29331e-07,5.25666e-11}},

						{{0.991276,1.72141e-06,7.57266e-12},
						{0.993746,1.82717e-06,1.93950e-11},
						{0.994254,1.92776e-06,1.69437e-11},
						{0.994799,1.49037e-06,2.85603e-11}}
						};

	/* In put transfromation metrixes */
	for (i=0;i<MaxNLCMatrix;i++){
//	  nlcinfo.date[i]=(char *)malloc(MaxStringLength*sizeof(char));
	  for (j=0;j<MaxChipInstalled;j++){
		 for (k=0;k<3;k++){
			nlcinfo.param[i][j][k]=param[i][j][k];
		 }
	  }
	}


	strcpy((nlcinfo.date[0]),"20060818\0");
	strcpy((nlcinfo.date[1]),"20060819\0");
	strcpy((nlcinfo.date[2]),"20070716\0");
	strcpy((nlcinfo.date[3]),"20080401\0");


	nlcinfo.jd[0]=str2jd("2006-08-18");
	nlcinfo.jd[1]=str2jd("2006-08-19");
	nlcinfo.jd[2]=str2jd("2007-07-16");
	nlcinfo.jd[3]=str2jd("2008-04-01");

	return EXIT_SUCCESS;
}


NonLinearInfo Process::getNLCInfo(){
	return nlcinfo;
}

int Process::detrendImage(Image *raw, Image *flat, Image *dark, Image *mask){

	int i,ext,s,size;
	float *arr=NULL;
	float *img=NULL;
	double val;

	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

	arr=(float *)malloc(size*sizeof(float));
	if (arr == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}
	img=(float *)malloc(size*sizeof(float));
	if (img == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}

	#ifdef DEBUG
	printf("Begin to processing image, take a while.\n");
	#endif

	/* Go through each extension */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
	  /* Go through each slice */
	  for (s=1;s<=raw->getImageHeader().naxis3;s++){

		 /* Go through each pixel */
		 for (i=0;i<size;i++){
			val=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i);
			//if (i == 20400){printf("pixel value=%f\n",val);}
			val=val-dark->getImageArrayValue((ext-1)*size+i);
			//if (i == 20400){printf("pixel value=%f\n",val);}

			val=val/flat->getImageArrayValue((ext-1)*size+i);
			//if (i == 20400){printf("pixel value=%f\n",val);}

			raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,val);

			if (mask->getImageArrayValue((ext-1)*size+i) == 0){
			   raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
			}
			if (val > SATURATION){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
				//raw->ImageArray[((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i]=0.0;
			}
			if (val < -SATURATION){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
				//raw->ImageArray[((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i]=0;
			}

		 }
	  }
	}
	free(img);
	free(arr);

	for (ext=0;ext<raw->getImageHeader().nextend;ext++){
		raw->updateExtensionHeader(ext+1,"BPIXNAME", basename(mask->getFilename()),"Badpix Name");
		raw->updateExtensionHeader(ext+1,"BDPIXVAL", 0.0,"Bad pixels value");
		raw->updateExtensionHeader(ext+1,"DARKSUB", "yes","Dark Subtraction done?");
		raw->updateExtensionHeader(ext+1,"DARKNAME", basename(dark->getFilename()),"Dark Name");
		raw->updateExtensionHeader(ext+1,"FLATDIV", "yes","Flat field division done? ");
		raw->updateExtensionHeader(ext+1,"FLATNAME", basename(flat->getFilename()),"Flat Name");
		raw->updateExtensionHeader(ext+1,"FLATCORR", "no","Flat field corrected (for horiz. lines) ");

	}


	return EXIT_SUCCESS;

}

int Process::detrendImage(Image *raw, Image *dark, Image *mask){

	int i,ext,s,size;
	float *arr=NULL;
	float *img=NULL;
	double val;

	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

	arr=(float *)malloc(size*sizeof(float));
	if (arr == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}
	img=(float *)malloc(size*sizeof(float));
	if (img == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
				"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
		throw FITSIOException(MemoryAllocateError);
	}


	#ifdef DEBUG
	printf("Begin to processing image, take a while.\n");
	#endif

	/* Go through each extension */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
	  /* Go through each slice */
	  for (s=1;s<=raw->getImageHeader().naxis3;s++){

		 /* Go through each pixel */
		 for (i=0;i<size;i++){
			val=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i);
			val=val-dark->getImageArrayValue((ext-1)*size+i);

			raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,val);

			/** flag bad pixels to -CHIPBIAS since this is a impossible value for 16 bit image. Do not put
			 * 		ZERO because sometimes this value is good */
			if (mask->getImageArrayValue((ext-1)*size+i) == 0){
			   raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
			}
			if (val > SATURATION){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
				//raw->ImageArray[((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i]=0.0;
			}
			if (val < -SATURATION){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
				//raw->ImageArray[((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i]=0;
			}

		 }
	  }
	}
	free(img);
	free(arr);

	for (ext=0;ext<raw->getImageHeader().nextend;ext++){
		raw->updateExtensionHeader(ext+1,"BPIXNAME", basename(mask->getFilename()),"Badpix Name");
		raw->updateExtensionHeader(ext+1,"BDPIXVAL", 0.0,"Bad pixels value");
		raw->updateExtensionHeader(ext+1,"DARKSUB", "yes","Dark Subtraction done?");
		raw->updateExtensionHeader(ext+1,"DARKNAME", basename(dark->getFilename()),"Dark Name");
		raw->updateExtensionHeader(ext+1,"FLATDIV", "yes","Flat field division done? ");
		raw->updateExtensionHeader(ext+1,"FLATCORR", "no","Flat field corrected (for horiz. lines) ");

	}


	return EXIT_SUCCESS;

}


int Process::skySubtraction(Image *raw, Image *sky){

	   int i,ext,s,size;
	   double median;
	   double *arr=NULL;
	   double *img=NULL;
	   double value;


	   size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;
	   //mefsize=rawinfo.naxis1*rawinfo.naxis2*rawinfo.naxis3;
	   arr=(double *)malloc(size*sizeof(double));
	   if (arr == NULL){
			fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
					"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
			throw FITSIOException(MemoryAllocateError);
	   }

	   img=(double *)malloc(size*sizeof(double));
	   if (img == NULL){
			fprintf(stderr, "Error: (%s:%s:%d) unable to allocate memory space "
					"for %s.\n", __FILE__, __func__, __LINE__,raw->getBaseName());
			throw FITSIOException(MemoryAllocateError);
	   }

	   #ifdef DEBUG
	   printf("Begin to subtracting sky image, take a while.\n");
	   #endif

		/* Go through each extension */
		for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
		  /* Go through each slice */
		  for (s=1;s<=raw->getImageHeader().naxis3;s++){
			 for (i=0;i<size;i++){
				 arr[i]=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i);
			 }

			 median=GetMedian(arr,size);
	         //meddev=GetMedianDev(arr,size,median);
			 #ifdef DEBUG
				 printf("Image median=%f \n",median);
			 #endif

	         for (i=0;i<size;i++){
	        	//if (i==2048500){printf("value1=%f\n",raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i));}
	        	if (sky->getImageArrayValue((ext-1)*size+i) == 0 || isnan(sky->getImageArrayValue((ext-1)*size+i)) ||
	        			raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i) == 0 ||
	        			raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i) == -raw->getImageHeader().chipbias){
	            	raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
	            } else {
	            	value=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i);
	            	value=value-(median*sky->getImageArrayValue((ext-1)*size+i) );
	            	raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,value);
	            }
	        	//if (i==2048500){printf("valueS=%f\n",sky->getImageArrayValue((ext-1)*size+i));}
	        	//if (i==2048500){printf("value2=%f\n",raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i));}

	         }
		  }
		}
		free(img);
		free(arr);

		return EXIT_SUCCESS;
}

int Process::skySubtraction(Image *raw, Image *sky, Image *mask){
	int i,size=0,ext,s;

	skySubtraction(raw,sky);

	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

	/** Masking bad pixel to -CHIPBIAS, so that is will become 0 in 16BIT image */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
		for (s=1;s<=raw->getImageHeader().naxis3;s++){
			for (i=0;i<size;i++){
				if (mask->getImageArrayValue(((ext-1)*size)+i) == 0){
					raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,-raw->getImageHeader().chipbias);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}




int Process::maskGuideWindow(Image *raw){

   int ext,slice;
   int size,x,y;

   size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

   #ifdef DEBUG
   printf("Masking Guide windows.\n");
   #endif

/** flag bad pixels to -CHIPBIAS since this is a impossible value for 16 bit image. Do not put
 * 		ZERO because sometimes this value is good */
   for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
      for (slice=1;slice<=raw->getImageHeader().naxis3;slice++){
         for (y=0;y<raw->getImageHeader().naxis1;y++){
            for (x=0;x<raw->getImageHeader().naxis1;x++){
               if (ext == 1){
                  if ((x > raw->getImageHeader().wcposx1-1 && x < raw->getImageHeader().wcposx1+15)
                		|| (y > raw->getImageHeader().wcposy1-1 && y < raw->getImageHeader().wcposy1+16)) {

							  raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(slice-1))*size+
                	  	  	      (y*raw->getImageHeader().naxis2+x),-raw->getImageHeader().chipbias);

                  }
               }
                if (ext == 2){
                  if ((x > raw->getImageHeader().wcposx2-1 && x < raw->getImageHeader().wcposx2+15)
                		 || (y > raw->getImageHeader().wcposy2-1 && y < raw->getImageHeader().wcposy2+16)) {

                	  	  	  raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(slice-1))*size+
                	  	  	      	  (y*raw->getImageHeader().naxis2+x),-raw->getImageHeader().chipbias);
                  }
               }
                if (ext == 3){
                  if ((x > raw->getImageHeader().wcposx3-1 && x < raw->getImageHeader().wcposx3+15)
                		  || (y > raw->getImageHeader().wcposy3-1 && y < raw->getImageHeader().wcposy3+16)) {

						  raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(slice-1))*size+
                	  	  	          (y*raw->getImageHeader().naxis2+x),-raw->getImageHeader().chipbias);
                  }
               }
                if (ext == 4){
                  if ((x > raw->getImageHeader().wcposx4-1 && x < raw->getImageHeader().wcposx4+15)
                		  || (y > raw->getImageHeader().wcposy4-1 && y < raw->getImageHeader().wcposy4+16)) {

						  raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(slice-1))*size+
                	  	  	           (y*raw->getImageHeader().naxis2+x),-raw->getImageHeader().chipbias);
                  }
               }

            }
         }
      }

      /// Updating the FITS header for guide window masking
      raw->updateExtensionHeader(ext,"GWINX1", raw->getImageHeader().wcposx1,"Guide window lower X boundary");
      raw->updateExtensionHeader(ext,"GWINX2", raw->getImageHeader().wcposx2,"Guide window upper Y boundary");
      raw->updateExtensionHeader(ext,"GWINY1", raw->getImageHeader().wcposy1,"Guide window lower X boundary");
      raw->updateExtensionHeader(ext,"GWINY2", raw->getImageHeader().wcposy2,"Guide window upper Y boundary");
      raw->updateExtensionHeader(ext,"XTALKGWP", "yes","Guide window crosstalk present?");
      raw->updateExtensionHeader(ext,"XTALKGW ", "yes","Guide window crosstalk removed?");
      raw->updateExtensionHeader(ext,"XTALKGWM", "MASK","Guide window xtalk removal method");
   }

   return EXIT_SUCCESS;

}

void Process::makeSourceMask(Image *raw, Image *check, Image *mask){
	int ext,s,size,i;
	double val;

	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

	/* Go through each extension */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
	  /* Go through each slice */
	  for (s=1;s<=raw->getImageHeader().naxis3;s++){

		 /* Go through each pixel */
		 for (i=0;i<size;i++){
			val=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i);


			if (val <= -raw->getImageHeader().chipbias || val == 0){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,0);
			} else {
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,1);
			}
			//if (i==24000){printf("valueS=%f\n",check->getImageArrayValue((ext-1)*size+i));}
			if (check->getImageArrayValue((ext-1)*size+i)+check->getImageHeader().chipbias > 50){
				raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,0);
			}

			/** flag bad pixels to -CHIPBIAS since this is a impossible value for 16 bit image. Do not put
			 * 		ZERO because sometimes this value is good */
			if (mask->getImageArrayValue((ext-1)*size+i) < 1){
			    raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+(s-1))*size+i,0);
			}

		 }
	  }
	  raw->updateExtensionHeader(ext,"CHIPBIAS", 0.0,"Science frame chip bias added to CDS value");
	}

	raw->updateMasterHeader("CHIPBIAS", 0.0,"Science frame chip bias added to CDS value");
	raw->HeaderInfo.chipbias=0;
}




void Process::imageSmooth(Image *raw, float ksize){
	int ext,s,size,x,y;
	float val;
	size=raw->getImageHeader().naxis1*raw->getImageHeader().naxis2;

	/** Define a image space for storing a WIRCam image . */
	CImg<float> img(raw->HeaderInfo.naxis1,raw->HeaderInfo.naxis2);

	/* Go through each extension */
	for (ext=1;ext<=raw->getImageHeader().nextend;ext++){
	  /* Go through each slice */
	  for (s=1;s<=raw->getImageHeader().naxis3;s++){

		 /* Go through each pixel */
		 for (x=0;x<raw->HeaderInfo.naxis1;x++){
			 for (y=0;y<raw->HeaderInfo.naxis2;y++){
				 val=raw->getImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+
						 (s-1))*size+(y*raw->HeaderInfo.naxis1+x))+raw->HeaderInfo.chipbias;
				 img(x,y)=val;
			 }
		 }
		 img.blur(ksize);
		 //img.display();
		 for (x=0;x<raw->HeaderInfo.naxis1;x++){
			 for (y=0;y<raw->HeaderInfo.naxis2;y++){
				 raw->setImageArrayValue(((ext-1)*raw->getImageHeader().naxis3+
				 						 (s-1))*size+(y*raw->HeaderInfo.naxis1+x),img(x,y)-raw->HeaderInfo.chipbias);
			 }
		 }

	  }

	}

}

Process::Process(){
	setNLCInfo();
}

Process::~Process(){
}



















