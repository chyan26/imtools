/*
 * listextname.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: chyan
 */

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include <string.h>
#include <stdio.h>
#include "fitsio.h"

using namespace std;

int main(int argc, char *argv[])
{
    fitsfile *fptr;         /* FITS file pointer, defined in fitsio.h */
    char keyname[FLEN_KEYWORD], colname[FLEN_VALUE], coltype[FLEN_VALUE];
    int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    int single = 0, hdupos, hdutype,check, bitpix, naxis, ncols, ii;
    long naxes[10], nrows;
    char card[FLEN_CARD]="";
    char keyvalue[FLEN_VALUE], keycomment[FLEN_COMMENT];
    string string;

    if (argc != 2) {
      printf("Usage:  listextname filename[ext] \n");
      printf("\n");
      printf("List the structure of a single extension, or, if ext is \n");
      printf("not given, list the structure of the entire FITS file.  \n");
      printf("\n");
      printf("Note that it may be necessary to enclose the input file\n");
      printf("name in single quote characters on the Unix command line.\n");
      return(0);
    }

    if (!fits_open_file(&fptr, argv[1], READONLY, &status))
    {
      fits_get_hdu_num(fptr, &hdupos);  /* Get the current HDU position */

      /* List only a single structure if a specific extension was given */
      if (strchr(argv[1], '[') || strchr(argv[1], '+')) single++;

      for (; !status; hdupos++)   /* Main loop for each HDU */
      {
        fits_get_hdu_type(fptr, &hdutype, &status);  /* Get the HDU type */

        //printf("\nHDU #%d  ", hdupos);
        if (hdutype == IMAGE_HDU)   /* primary array or image HDU */
        {
      	  if(hdupos != 1) {
      		  fits_read_card(fptr,"EXTNAME", card, &status);
      		  if (status == 0){

          		fits_parse_value(card, keyvalue, keycomment, &status);
          		string=keyvalue;
          		string.erase(0,1);
          		string.erase(string.find_last_not_of("' ")+1,string.length());
          		cout<<argv[1]<<'['<<string<<']'<<'\n';
      		  }

      	  }

        }
        else  /* a table HDU */
        {
          printf("here.\n");
          fits_get_num_rows(fptr, &nrows, &status);
          fits_get_num_cols(fptr, &ncols, &status);

          if (hdutype == ASCII_TBL)
            printf("ASCII Table:  ");
          else
            printf("Binary Table:  ");

          printf("%d columns x %ld rows\n", ncols, nrows);
          printf(" COL NAME             FORMAT\n");

          for (ii = 1; ii <= ncols; ii++)
          {
            fits_make_keyn("TTYPE", ii, keyname, &status); /* make keyword */
            fits_read_key(fptr, TSTRING, keyname, colname, NULL, &status);
            fits_make_keyn("TFORM", ii, keyname, &status); /* make keyword */
            fits_read_key(fptr, TSTRING, keyname, coltype, NULL, &status);

            printf(" %3d %-16s %-16s\n", ii, colname, coltype);
          }
        }

        if (single)break;  /* quit if only listing a single HDU */

        check=fits_movrel_hdu(fptr, 1, NULL, &status);  /* try move to next ext */
        if (hdupos < 4 and check){
        	printf("error: there is not one slice in this FITS file.\n");
        }
      }

      if (status == END_OF_FILE) status = 0; /* Reset normal error */
      fits_close_file(fptr, &status);
    }

    if (status) fits_report_error(stderr, status); /* print any error message */
    return(status);

}


