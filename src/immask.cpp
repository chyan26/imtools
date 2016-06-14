/*
 * immask.cpp
 *
 *  Created on: Aug 28, 2011
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
	   "Producing source mask based on a sigma threshold and weight image. \n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
		"	-i, --image=<FITS image>  \n"
		"	-o, --out=<check image>\n"
		"	-w, --weight=<weight image>\n"
		"	-s, --sigmalevel= threshold of sigma level \n"
		, prgname);

}

int main(int argc, char *argv[]){
	int opt;
	char *imagename = NULL;
	char *outname = NULL;
	char *weightname = NULL;
	char *sigmalevel = NULL;

	char checkname[80];

	stringstream ss;
	string tempname;

	Sextractor* sex;

	StackImage *raw;
	Image *original,*check,*mask;

	Process *process;

	/** Check the total number of the arguments */
	struct option longopts[] = {
		 {"image" ,1, NULL, 'i'},
		 {"out",1, NULL, 'o'},
		 {"weight",1, NULL, 'w'},
		 {"sigma",1, NULL, 's'},
		 {"help", 0, NULL, 'h'},
		 {0,0,0,0}};

	while((opt = getopt_long(argc, argv, "i:o:w:s:h",
	   longopts, NULL))  != -1){
	      switch(opt) {
	         case 'i':
	               imagename = optarg;
	               break;
	         case 'o':
	               outname = optarg;
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

	/** Define a check name */
	ss << imagename;
	sprintf(checkname,"%s/%s",dirname((char *)ss.str().c_str()),"checktemp.fits");


	/** Starting a try block for running SExtractor*/
	try{
		/** First of all, making a crunched image for enhancing SN ratio */
		raw = new StackImage(imagename);

	} catch (FITSIOException &e){
		fprintf(stderr, "Error: (%s:%s:%d) unable to finishing processing. \n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}

	try{

		/** Save it to another name */
		ss.str("");
		ss << imagename;
		tempname=ss.str()+".tmp";
		raw->SaveImage16B((char *)tempname.c_str());

		/** Use this crunched FITS file for SExtractor */
		sex = new Sextractor((char *)tempname.c_str());

		/** Setting the name of SExtractor configuration file */
		sex->setConfigName("sex.conf");

		/** Defined the check image type to be OBJECTS */
		sex->setCheckType("OBJECTS");

		/** Passing the file name of check image into setting */
		sex->setCheckName(checkname);

		if (sigmalevel != NULL){
			sex->setSigmaLevel(sigmalevel);
		}

		if (weightname != NULL){
			sex->setWeightName(weightname);
		}

		sex->runSextractor();
		delete(raw);

	} catch (SextractorException &e) {
		fprintf(stderr, "Error: (%s:%s:%d) unable to run sextractor.\n",__FILE__, __func__, __LINE__);
		e.dumpMessage();
		return EXIT_FAILURE;
	}

	/* Adding a check for bad file names */
	#ifdef DEBUG
		printf("Image= %s\n",imagename);
		printf("Check= %s\n",checkname);
		printf("Mask= %s\n",weightname);
	#endif

	/** Starting another try block for producing image mask */
	try {

		/** Loading images */
		check = new Image(checkname);
		mask = new Image(weightname);
		original= new Image(imagename);

		/** Loading image processing class */
		process = new Process;

		/** Smooth the check image (OBJECTS) with kernel size to be 1.5 */
		process->imageSmooth(check, 1.5);

		/** Masking source based on the smoothed check image */
		process->makeSourceMask(original, check, mask);

		/** Masking guide window */
		process->maskGuideWindow(original);

		/** Write file to disk */
		original->SaveImage16B(outname);

		/** Clean temporary crunched image */
		remove(tempname.c_str());
		remove(checkname);

		delete(check);
		delete(mask);
		delete(original);
		delete(process);


	} catch (FITSIOException &e){
		fprintf(stderr, "Error: (%s:%s:%d) unable to finishing processing. \n",__FILE__, __func__, __LINE__);
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




