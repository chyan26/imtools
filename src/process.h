/**
 * \file process.h
 * \brief  The prototype for class process and related structures
 *  Created on: Aug 17, 2010
 *      Author: chyan
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "imparameter.h"

/** \struct NonLinearInfo
 * 	\brief This structure is used for storing correction matrix when carrying out
 *    non-linear correction.
 */
struct NonLinearInfo{
   /// The non-linear correction correction matrix for each chip.
   double param[MaxNLCMatrix][MaxChipInstalled][3];
   /// Date of derived non-linear correction matrix
   char date[4][MaxStringLength];
   /// Julian day of derived non-lineae correction matrix
   long jd[MaxNLCMatrix];
};

/**
 * \class Process
 * \brief This class defines the method used for basic image processing
 * \details  Here we defined the method used to remove the artificial signal on WIRCam
 * chip.  The first step is subtraction the reference pixel.
 */
class Process {
private:
	/**
	 * A private structure for string non-linear correction matrix
	 */
	NonLinearInfo nlcinfo;
public:
	/**
	 * Method for subtracting reference pixels
	 */
	int subRefPixel(Image *raw);

	/**
	 * Non-linear correction method
	 *
	 */
	int NonLinearCorrect(Image *raw);

	/**
	 * Method to initialize non-linear correction matrix
	 *
	 */
	int setNLCInfo();

	/**
	 * A public method to return the non-linear correction matrix
	 */
	NonLinearInfo getNLCInfo();

	/**
	 * De-trending method for raw image.
	 */
	int detrendImage(Image *raw, Image *flat, Image *dark, Image *mask);
	int detrendImage(Image *raw, Image *dark, Image *mask);

	/**
	 * De-trending method for raw image.
	 */
	int skySubtraction(Image *raw, Image *sky);
	int skySubtraction(Image *raw, Image *sky, Image *mask);

	/**
	 *  Masking the guide windows.
	 */
	int maskGuideWindow(Image *raw);

	/**
	 *  Producing a source mask based on check image.
	 */
	void makeSourceMask(Image *raw, Image *check, Image *mask);

	/**
	 *  Smoothing an image using blur algorithm
	 */
	void imageSmooth(Image *check, float size);

	/**
	 *  Default constructor
	 */
	Process();
	~Process();
};


#endif /* PROCESS_H_ */
