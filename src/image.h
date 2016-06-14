/**
 * @file image.h
 * This class defines the basic operations for FITS image.
 *  Created on: Aug 13, 2010
 *      Author: chyan
 */

#ifndef IMAGE_H_
#define IMAGE_H_
#include <string>

#include "imparameter.h"
using namespace std;




/**
 *  Define a structure for sky background values
 */
struct skybackground_t{
	float exptime;
	float* skylvl;
	float* skydev;
	float* skyrate;
};

/**
 *  Define a header for
 */
struct ImageHeader{
	int  mainHeaderKeys;
	int  extHeaderKeys;
	int  firstMainHeaderSpace;
	int  firstExtHeaderSpace[MaxChipInstalled];
	string mainHeader;
	string extHeader;
	//string extHeader;
	//char mainHeader[MaxHeaderKeys*FLEN_CARD];
	//char extHeader[MaxChipInstalled*MaxHeaderKeys*FLEN_CARD];
};

/**
 *
 * \brief This the structure for storing the FITS header information.
 * This structure can be used for WIRCam images.
 *
 *
*/
///\struct ImageInfo
struct ImageInfo{
  ///BITPIX keyword of FITS image.
  int bitpix;
  /// Number of total extensions in FITS image.
  int nextend;
  /// Dimension FITS images
  int naxis;
  /// Dimension of X-axis
  int naxis1;
  /// Dimension of Y-axis
  int naxis2;
  /// Dimension of Z-axis or time sequence.
  int naxis3;
  /// Number for image wrapping.
  int bzero;
  /// Steps of micro-dithering.
  int mdcoords;
  /// Chip bias level
  int chipbias;
  ///Pairs of reads in science Fowler sampling.
  int scfowler;
  /// Exposure time
  double exptime;
  /// Target Right Ascension
  double ra;
  /// Target Declination
  double dec;
  /// Instrument RA
  double zra;
  /// instrument Dec
  double zdec;
  /// Rotation of the camera
  double wcrotate;
  /// Single readout time for Non-linear correction
  double rdtime;
  /// MD coordinate X1
  float mdx1;
  /// MD coordinate Y1
  float mdy1;
  /// MD coordinate X2
  float mdx2;
  /// MD coordinate Y2
  float mdy2;
  /// MD coordinate X3
  float mdx3;
  /// MD coordinate Y3
  float mdy3;
  /// MD coordinate X4
  float mdx4;
  /// MD coordinate Y4
  float mdy4;
  /// X position of guide window in chip 1
  int wcposx1;
  /// Y position of guide window in chip 1
  int wcposy1;
  /// X position of guide window in chip 2
  int wcposx2;
  /// Y position of guide window in chip 2
  int wcposy2;
  /// X position of guide window in chip 3
  int wcposx3;
  /// Y position of guide window in chip 3
  int wcposy3;
  /// X position of guide window in chip 4
  int wcposx4;
  /// Y position of guide window in chip 4
  int wcposy4;
  /// Filter for FITS image
  const char *filter;
  /// Date of observation in readable format
  const char *date;
  /// Date of observation in Julian Day
  long jd;
};


/**
 * \class Image
 * \brief The basic images class of image operation.
 * \details The default constructor
 *   take the FITS filename as a input.  Then it will establish a data structure
 *   to store the FITS header information.  The data is also imported in a 1-D array
 *   as well.  This class also contains the methods for image saving and loading.
 *
 */

class Image {
private:
	int flagImageBackground;
	void initImageBackground();
	void freeImageBackground();
	//FITSIOException* ce;

protected:
	/** A pointer to FITS file name
	 *  \details
	 */
	char* Filename;
	//char* Basename;
	//char* Dirname;
	/** Pointers of FITS files that will be used in this subroutine
	 *  \details
	 */
	fitsfile* fptr;
	fitsfile* outfptr;

	/** A pointer to FITS file name
	 *  \details
	 */
	ImageInfo HeaderInfo;

	int ImageType;

	float* ImageArrayFloat;
	int* ImageArrayInt;
	bool* ImageArrayByte;

	skybackground_t* ImageBackground;

	ImageHeader Header;

public:
	friend class Process;

	void openFitfile();
	int closeFitfile();


	void setFilename(char* filename);
	void setImageHeader();
	int setImageArray();
	int setImageMedian();
	void setImageArrayValue(int n, double value);
	double getImageArrayValue(int n);
	void setImage(char* filename);


	char* getFilename();
	char* getBaseName();
	char* getDirName();
	ImageInfo getImageHeader();
	skybackground_t* getImageMedian();
	fitsfile* getInputFITSptr();


	int SaveImage16B(char *filename);
	int SaveImage32B(char *filename);

	int updateMasterHeader(char* keyword,char* string,char* comment);
	int updateMasterHeader(char* keyword,double value,char* comment);

	int updateExtensionHeader(int ext, char* keyword,char* string,char* comment);
	int updateExtensionHeader(int ext, char* keyword,double value,char* comment);

	int updateSkylevelHeader();

	int cleanDetrendHeader();

	Image();

	Image(char* filename);
	/** Image destructor.
	 *  The destructor of Image class.
	 */

	~Image();

};


class StackImage : public Image{

protected:

public:
	typedef Image super;

	void setCrunchMethod(int value);
	void getCrunchMethod();

	void setStackImage();

	StackImage();

	StackImage(char* filename);

	~StackImage();
};



#endif /* IMAGE_H_ */
