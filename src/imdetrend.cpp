/*
 *  Created on: Aug 13, 2010
 *      Author: chyan
 */

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <math.h>
#include <getopt.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "process.h"
#include "imexception.h"
using namespace std;



/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Remove the artificial effects from an WIRCam image\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
		"	-r, --raw=<raw image>  \n"
		"	-b, --dark=<dark image>\n"
		"	-f, --flat=<flat image>\n"
		"	-m, --badpix=<bad pix mask>\n"
		"	-o, --out=<output name prefix>\n"
		"	-n, --noflat,		processing image w/o applying flat field.\n"
		"	-a, --addchipbias,	add chip bias to final reduced image.\n"
		"	-c, --correctrefpix,	correcting reference pixel.\n"
		"	-g, --maskguidewindow,	masking guide window.\n"
		"	-l, --save32bit,	save image in 32 bit.\n"
		, prgname);

}


int main(int argc, char *argv[])
{
	int opt,ext;
	char* rawname = NULL;
	char *flatname = NULL;
	char *darkname = NULL;
	char *maskname = NULL;
	char *outname = NULL;

	bool noflat = FALSE;
	bool correctrefpix = FALSE;
	bool save32bit = FALSE;
	bool maskgw = TRUE;

	Image *raw,*flat,*dark,*mask;
	Process *process;

	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];

	/* Check the total number of the arguments */
	struct option longopts[] = {
		 {"raw" ,1, NULL, 'r'},
		 {"dark",1, NULL, 'b'},
		 {"flat",1, NULL, 'f'},
		 {"badpix",1, NULL, 'm'},
		 {"output", 1, NULL, 'o'},
		 {"help", 0, NULL, 'h'},
		 {"noflat", 0, NULL, 'n'},
		 {"correctrefpix", 0, NULL, 'c'},
		 {"maskguidewindow", 0, NULL, 'g'},
		 {"save32bit",0,NULL,'l'},
		 {0,0,0,0}};

	while((opt = getopt_long(argc, argv, "cr:b:f:p:m:s:o:lghn",
	   longopts, NULL))  != -1){
	      switch(opt) {
	         case 'r':
	               rawname = optarg;
	               break;
	         case 'b':
	               darkname = optarg;
	               break;
	         case 'f':
	               flatname = optarg;
	               break;
	         case 'm':
	               maskname = optarg;
	               break;
	         case 'o':
	               outname = optarg;
	               break;
	         case 'c':
	               correctrefpix = TRUE;
	               break;
	         case 'n':
	               noflat = TRUE;
	               break;
	         case 'l':
	               save32bit = TRUE;
	               break;
	         case 'g':
	               maskgw = TRUE;
	               break;
	         case 'h':
	               printUsageSyntax(argv[0]);
	               exit(EXIT_FAILURE);
	               break;
	         case '?':
	               printUsageSyntax(argv[0]);
	               exit(EXIT_FAILURE);
	               break;
	      }
	}

	/* Print the usage syntax if there is no input */
	if (argc < 2 ) {
		printUsageSyntax(argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Go through and handle the filename and path information */
	if ((rawname == NULL) || (outname == NULL)){
		fprintf(stderr, "Error: (%s:%s:%d) please define the "
		"file name of the raw and/or output image.\n", __FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}
	if (noflat==FALSE && flatname == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) please define the "
		"file name of the FLAT FIELD image.\n", __FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}
	if (darkname == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) please define the "
		"file name of the DARK image.\n", __FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}
	if (maskname == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) please define the "
		"file name of the BAD PIXEL image.\n", __FILE__, __func__, __LINE__);
		return EXIT_FAILURE;

	}
	if ((flatname == NULL) || (darkname == NULL) || (maskname == NULL)){
		fprintf(stderr, "Error: (%s:%s:%d) please define the "
		"calibration images.\n", __FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}

	/* Adding a check for bad file names */
	#ifdef DEBUG
		printf("Raw= %s\n",rawname);
		printf("Flat= %s\n",flatname);
		printf("Dark= %s\n",darkname);
		printf("Mask= %s\n",maskname);
	#endif


	try {
		raw = new Image(rawname);
		flat = new Image(flatname);
		dark = new Image(darkname);
		mask = new Image(maskname);
	} catch (FITSIOException &e){
		fprintf(stderr, "Error: (%s:%s:%d) \n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}


	/// Write processing time into FITS header.
	time(&rawtime);
	timeinfo = localtime(&rawtime );
	strftime(buffer,sizeof(buffer),"%Y-%m-%dCST%H:%M:%S",timeinfo);

	/// Updating processing information in headers.
	try {
		for (ext=0;ext<raw->getImageHeader().nextend;ext++){
			raw->updateExtensionHeader(ext+1,"PROCDATE",buffer,"Date-Time of Processing");
			raw->updateExtensionHeader(ext+1,"GAIN",3.80,"Gain with capac. coupl. correction [e-/adu]");
			raw->updateExtensionHeader(ext+1,"GAINERR",0.20,"Gain uncertainty [e-/adu]");
			raw->updateExtensionHeader(ext+1,"GAINMEAS",4.30,"Gain measured without correction [e-/adu]");
			if (correctrefpix == TRUE){
				raw->updateExtensionHeader(ext+1,"REFPXCOR","yes","Ref. pixel correction done?");
			} else {
				raw->updateExtensionHeader(ext+1,"REFPXCOR","no","Ref. pixel correction done?");
			}
		}
	} catch (FITSIOException &e){
		fprintf(stderr, "Error: (%s:%s:%d) \n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}


	try {
		/// Establish a new processing object.
		process=new Process();

		/// Carry out non-linearity correction.
		process->NonLinearCorrect(raw);

		/// Subtracting reference pixel if required.
		if (correctrefpix == TRUE){
			process->subRefPixel(raw);
		}

		/// Processing the image. NOFLAT option in used inly when producing images for building sky flats.
		if (noflat == TRUE){
			printf("Caution: no flat fielding is applied.\n");
			process->detrendImage(raw,dark,mask);
		} else {
			process->detrendImage(raw,flat,dark,mask);
		}


		/// Masking the guide windows
		if (maskgw == TRUE){
			process->maskGuideWindow(raw);
		}

		/// Calculate the median and median deviation of this image.
		raw->setImageMedian();
		raw->updateSkylevelHeader();

		/// Save image.  Default is 16 bit for save disk space.
		if (save32bit == TRUE){
			raw->SaveImage32B(outname);
		} else {
			raw->SaveImage16B(outname);
		}

		/// Clean unused or reserved FITS header.
		raw->cleanDetrendHeader();
	} catch (FITSIOException &e){
		fprintf(stderr, "Error: (%s:%s:%d) unable to finishing processing. \n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}

	/// Destroy all image objects.
	delete(process);
	delete(dark);
	delete(flat);
	delete(mask);
	delete(raw);

	return EXIT_SUCCESS;
}
