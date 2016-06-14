/*
 * imsex.cpp
 *
 *  Created on: Jul 22, 2011
 *      Author: chyan
 */

#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <ctype.h>
#include <map>
#include <math.h>
#include <getopt.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "process.h"
#include "sextractor.h"
using namespace std;



/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Run SExtractor on WIRCam image with default settings\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
		"	-i, --image=<FITS image>  \n"
		"	-c, --check=<check image>\n"
		"	-t, --checktype= type of check image\n"
		"	-s, --sigmalevel= threshold of source detection\n"
		"	-d, --catalog=<catalog name>\n"
		"	-w, --weight=<weight image>\n"
		, prgname);

}

int main(int argc, char *argv[]){
	int opt;
	char *catname = NULL;
	char *imagename = NULL;
	char *checkname = NULL;
	char *weightname = NULL;
	char *checktype = NULL;
	char *sigmalevel = NULL;

//	bool keepcat=FALSE;
//	bool keepchkimg=FALSE;
	stringstream ss;

	Sextractor* sex;

	/** Check the total number of the arguments */
	struct option longopts[] = {
		 {"image" ,1, NULL, 'i'},
		 {"check",1, NULL, 'c'},
		 {"checktype",1, NULL, 't'},
		 {"catalog",1, NULL, 'd'},
		 {"weight",1, NULL, 'w'},
		 {"sigma",1, NULL, 's'},
		 {"help", 0, NULL, 'h'},
		 {0,0,0,0}};

	while((opt = getopt_long(argc, argv, "i:c:t:d:w:s:h",
	   longopts, NULL))  != -1){
	      switch(opt) {
	         case 'i':
	               imagename = optarg;
	               break;
	         case 'c':
	               checkname = optarg;
	               break;
	         case 't':
	               checktype = optarg;
	               break;
	         case 'd':
	               catname = optarg;
	               break;
	         case 'w':
	               weightname = optarg;
	               break;
	         case 's':
	               sigmalevel = optarg;
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


	/** Print the usage syntax if there is no input */
	if (argc < 2 ) {
		printUsageSyntax(argv[0]);
		return EXIT_FAILURE;
	}

	if (imagename == NULL){
		printUsageSyntax(argv[0]);
		return EXIT_FAILURE;

	}

	if (checkname != NULL) {

		/** If check image type is not given, use APERTURES as default. */
		if (checktype == NULL){
			fprintf(stderr, "Logonly: (%s:%s:%d) use APERTURES for default check image type.\n",
					__FILE__, __func__, __LINE__);
			checktype="APERTURE";
		}

		/** Check if the input is correct */
		stoupper(checktype);
		ss << checktype;

		if ((ss.str().compare("APERTURES") != 0) && ((ss.str().compare("BACKGROUND") != 0)) &&
				((ss.str().compare("BACKGROUND_RMS") != 0)) && ((ss.str().compare("MINIBACKGROUND") != 0)) &&
				((ss.str().compare("MINIBACK_RMS") != 0)) && ((ss.str().compare("-BACKGROUND") != 0)) &&
				((ss.str().compare("FILTERED") != 0)) && ((ss.str().compare("OBJECTS") != 0)) &&
				((ss.str().compare("-OBJECTS") != 0)) && ((ss.str().compare("SEGMENTATION") != 0))){

			fprintf(stderr, "Error: (%s:%s:%d) incorrect setting for check image type: %s.\n",
					__FILE__, __func__, __LINE__,checktype);
			fprintf(stderr, "Available Types: BACKGROUND, BACKGROUND_RMS, MINIBACKGROUND, "
					"MINIBACK_RMS,-BACKGROUND, FILTERED, OBJECTS, -OBJECTS, SEGMENTATION, APERTURES\n");
			return EXIT_FAILURE;
		}

	}

	try{
		sex = new Sextractor(imagename);
		sex->setConfigName("sex.conf");

		if (sigmalevel != NULL){
			sex->setSigmaLevel(sigmalevel);
		}

		if (weightname != NULL){
			sex->setWeightName(weightname);
		}

		if (catname != NULL){
			sex->setCatName(catname);
		}

		if (checkname != NULL){
			sex->setCheckName(checkname);
		}

		if (checktype != NULL){
			sex->setCheckType(checktype);
		}

		sex->runSextractor();

	} catch (SextractorException &e) {
		fprintf(stderr, "Error: (%s:%s:%d) unable to run sextractor.\n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}


	try{
		delete(sex);
	} catch (SextractorException &e) {
		fprintf(stderr, "Error: (%s:%s:%d) unable to finish sextractor task.\n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;
}

