/*
 * imastro.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: chyan
 */

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <math.h>
#include <getopt.h>

#include "fitsio.h"
#include "image.h";
#include "calculate.h"
#include "imexception.h"
#include "wcs.h"

using namespace std;



/* Print out the proper program usage syntax */
static void
printUsageSyntax(char *prgname) {
   fprintf(stderr,
	   "Remove the artificial effects from an WIRCam image\n"
	   "Usage: %s <INPUT> <OUTPUT> [options...]\n"
		"	-h, --help  display help message\n"
	    "	-i, --input=<output FITS file>.\n"
		"	-o, --out=<output FITS file>\n"
		"	-c, --cat=<catalog name>\n"
		"	-v, --save32bit,	save image in 32 bit.\n"
		, prgname);

}


int main(int argc, char *argv[])
{

	int opt;
	char *inname = NULL;
	char *outname = NULL;
	char *catname  = NULL;

	/* Check the total number of the arguments */
	struct option longopts[] = {
		 {"input" ,1, NULL, 'i'},
		 {"output",1, NULL, 'o'},
		 {"cat",1, NULL, 'c'},
		 {"help", 0, NULL, 'h'},
		 {"verbose", 0, NULL, 'v'},
		 {0,0,0,0}};

	while((opt = getopt_long(argc, argv, "hi:o:c:v",
	   longopts, NULL))  != -1){
	      switch(opt) {
	         case 'i':
	               inname = optarg;
	               break;
	         case 'o':
	               outname = optarg;
	               break;
	         case 'c':
	               catname = optarg;
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


	return EXIT_SUCCESS;
}
