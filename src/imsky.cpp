/*
 * imsky.cpp
 *
 *  Created on: Nov 19, 2010
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
#include <getopt.h>

#include "fitsio.h"
#include "imexception.h"
#include "image.h"
#include "calculate.h"
#include "skybackground.h"
using namespace std;

/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Building WIRCam sky background image based on image list\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
		"	-f, --filelist=[range of input images]  \n"
		"	-o, --output, name of output image\n"
		"	-m, --mask=list of image mask \n"
		, prgname);

}


int main(int argc, char *argv[]){

	int opt;
	char *input = NULL;
	char *output = NULL;
	char *maskname = NULL;

	/* Define a flag for image masks */
	bool domask = FALSE;

	SkyBackground *sb;

	/* Check the total number of the arguments */
  	struct option longopts[] = {
		{"filelist" ,1, NULL, 'f'},
		{"mask", 1, NULL, 'm'},
		{"output", 1, NULL, 'o'},
		{"help", 0, NULL, 'h'},
		{0,0,0,0}};
	while((opt = getopt_long(argc, argv, "hf:o:b:",
	longopts, NULL))  != -1){
    	switch(opt) {
			case 'f':
	    			input = optarg;
	    			break;
			case 'o':
	    			output = optarg;
					break;
			case 'm':
	    			maskname = optarg;
					domask = TRUE;
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
	if (argc < 2 || argc > 4) {
		printUsageSyntax(argv[0]);
		exit(EXIT_FAILURE);
	}
	/* Go through and handle the filename and path information */
    if (! argv[optind] ){
    	if (input == NULL || output == NULL) {
			printUsageSyntax(argv[0]);
			exit(EXIT_FAILURE);
		}
    }


	#ifdef DEBUG
	printf("input = %s\n",input);
	printf("output = %s\n",output);
	printf("badpix= %s\n", maskname);
	#endif

	try{
		if (domask == TRUE){
			sb=new SkyBackground(input,maskname);

			/** Parse detrend file list into char array */
			sb->setDetrendFile(sb->getFileList());

			/** Parse mask file list into char array */
			sb->setMaskFile(sb->getMaskList());

			/** Loading all the detrend images. */
			sb->setDetrendImage();

			/** Find out how many slices are used */
			sb->setTotalFrameNumber();

			/** Calculating the QE ratios of all chips */
			sb->calcQeRatio();

			/** Correcting sky levels between different exposures */
			sb->correctSkyLevel();

			/** Applying image masks on images */
			sb->applyMasktoDetrend();

			/** Building sky background */
			sb->buildSkyImage();

		} else {
			sb=new SkyBackground(input);

			/** Parse detrend file list into char array */
			sb->setDetrendFile(sb->getFileList());

			/** Loading all the detrend images. */
			sb->setDetrendImage();

			/** Find out how many slices are used */
			sb->setTotalFrameNumber();

			/** Calculating the QE ratios of all chips */
			sb->calcQeRatio();

			/** Correcting sky levels between different exposures */
			sb->correctSkyLevel();

			/** Building sky background */
			sb->buildSkyImage();

		}

		sb->writeSkyBackgroundImage(output);

	} catch (FITSIOException &e) {
		fprintf(stderr, "Error: (%s:%s:%d) unable to finishing processing. \n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;

	}
	delete(sb);
	return EXIT_SUCCESS;
}
