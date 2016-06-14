/*
 * imtest.c
 *
 *  Created on: Jul 9, 2011
 *      Author: chyan
 */

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <math.h>
#include <getopt.h>

#include "fitsio.h"
#include "longnam.h"
#include "image.h"
#include "calculate.h"
#include "skybackground.h"
#include "imexception.h"
using namespace std;

/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Crunching a multiple slices WIRCam FITS image into single slice image.\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
		"	-i, --input,	name of input image \n"
		"	-o, --output,	name of output image\n"
		"	-m, --method,	combine methods(median, average et al.)\n"
		"	-l, --save32bit,	save image in 32 bit.\n"
		, prgname);

}

int main(int argc, char *argv[]){
	int opt;
	char* input = NULL;
	char *output = NULL;
	char *method = NULL;

	bool save32bit = FALSE;

	StackImage *raw;

	/* Check the total number of the arguments */
  	struct option longopts[] = {
		{"input" ,1, NULL, 'i'},
		{"method", 1, NULL, 'm'},
		{"output", 1, NULL, 'o'},
		{"save32bit", 1, NULL, 'l'},
		{"help", 0, NULL, 'h'},
		{0,0,0,0}};
	while((opt = getopt_long(argc, argv, "hi:o:m:l",
	longopts, NULL))  != -1){
    	switch(opt) {
			case 'i':
	    			input = optarg;
	    			break;
			case 'o':
	    			output = optarg;
					break;
			case 'm':
	    			method = optarg;
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

	if (input != NULL && output == NULL){
		fprintf(stderr, "Error: (%s:%s:%d) please specify the output file.\n",
				__FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}

	try {
		raw = new StackImage(input);

		/// Save image.  Default is 16 bit for save disk space.
		if (save32bit == TRUE){
			raw->SaveImage32B(output);
		} else {
			raw->SaveImage16B(output);
		}
	} catch (FITSIOException &e){
		fprintf(stderr, "%s (%s:%s:%d)\n",e.getMessage(),__FILE__, __func__, __LINE__);
		return EXIT_FAILURE;
	}


	delete(raw);

	return EXIT_SUCCESS;

}
