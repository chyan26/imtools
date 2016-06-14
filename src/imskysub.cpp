/*
 * imskysub.cpp
 *
 *  Created on: Aug 29, 2010
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
#include "imexception.h"
#include "process.h"
using namespace std;

/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Remove the sky background from a detrended WIRCam image.\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help,	display help message\n"
		"	-d, --detrend=<detrend image>  \n"
		"	-o, --out=<output name>\n"
		"	-s, --sky=<sky image>\n"
		"	-m, --mask=<mask image>\n"
		"	-l, --save32bit,   save image in 16 bit.\n"
		, prgname);

}

int main(int argc, char *argv[]){
	int opt;
	char* detrendname = NULL;
	char* skyname = NULL;
	char* outname = NULL;
	char* maskname = NULL;

	bool save32bit = FALSE;
	bool domask =FALSE;

	Image *detrend,*sky,*mask=NULL;
	Process *process;

	/* Check the total number of the arguments */
	struct option longopts[] = {
		 {"detrend" ,1, NULL, 'd'},
		 {"sky",1, NULL, 's'},
		 {"output", 1, NULL, 'o'},
		 {"mask", 1, NULL, 'm'},
		 {"help", 0, NULL, 'h'},
		 {"save32bit",0,NULL,'l'},
		 {0,0,0,0}};

	while((opt = getopt_long(argc, argv, "d:s:o:m:hl",
	   longopts, NULL))  != -1){
	      switch(opt) {
	         case 'd':
	               detrendname = optarg;
	               break;
	         case 's':
	               skyname = optarg;
	               break;
	         case 'm':
	               maskname = optarg;
				   domask = TRUE;
	               break;
	         case 'o':
	               outname = optarg;
	               break;
	         case 'l':
	               save32bit = TRUE;
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
		return EXIT_FAILURE;
	}

	/* Go through and handle the filename and path information */
	if (! argv[optind] ){
		if ((detrendname == NULL) || (outname == NULL) || (skyname == NULL)){
			fprintf(stderr, "Error: (%s:%s:%d) please define the "
			"file name of the detrend, sky and/or output image.\n", __FILE__, __func__, __LINE__);
			return EXIT_FAILURE;
		}
	}

	try{
		detrend = new Image(detrendname);
		sky = new Image(skyname);
		if (domask == TRUE){
			mask = new Image(maskname);
		}
		/// Establish a new processing object.
		process=new Process();

		if (domask == TRUE){
			process->skySubtraction(detrend,sky,mask);
		} else {
			process->skySubtraction(detrend,sky);
		}

		/// Save image.  Default is 16 bit for save disk space.
		if (save32bit == TRUE){
			detrend->SaveImage32B(outname);
		} else {
			detrend->SaveImage16B(outname);
		}

	} catch(FITSIOException &e) {
		fprintf(stderr, "%s (%s:%s:%d)\n",e.getMessage(),__FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}



	delete(process);
	delete(detrend);
	delete(sky);
	if (domask == TRUE){
		delete(mask);
	}


	return EXIT_SUCCESS;
}
