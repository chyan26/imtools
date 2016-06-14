/**
 * \file calculate.cpp
 * \brief This file defines the functions that will be used for imaeg processing
 *  Created on: Aug 17, 2010
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

using namespace std;


void stoupper(char *s)
{
    for(; *s; s++)
        if(('a' <= *s) && (*s <= 'z'))
            *s = 'A' + (*s - 'a');
}

void stolower(char *s)
{
    for(; *s; s++)
        if(('A' <= *s) && (*s <= 'Z'))
            *s = 'a' + (*s - 'A');
}


char *sltrim(char *str) {

   char *p;

   if (str == NULL)
      return NULL;

   for (p = str; (*p && isspace(*p)); p++)
      ;

   return p;
}

/* Trim off trailing whitespace in a string */

char *
srtrim(char *str) {

   int i;

   if (str != NULL) {
      for (i = strlen(str) - 1; (i >= 0 && isspace(str[i])); i--)
         ;
      str[++i] = '\0';
   }

   return str;
}

/* Trim off all leading and trailing whitespace from a string */

char * strim(char *str) {

        return srtrim(sltrim(str));
}

/** \fn long gd2jd(int y, int m, int d)
 *  \brief Core function for calculating Julian day
 *  \param y: year
 *  \param m: month
 *  \param d: day
 *  \return the Julian day
 *
 */
long gd2jd(int y, int m, int d){
   y+=8000;
   if(m<3) { y--; m+=12; }
   return (y*365) +(y/4) -(y/100) +(y/400) -1200820
              +(m*153+3)/5-92
              +d-1;
}

/** \fn long str2jd(const char* gd)
 * \brief Convert a date string to Julian day.
 * \param gd the point to the date string
 * \return the julian day
 *
*/

long str2jd(const char* gd){
   int y,m,d;
   string datestring;
   datestring=gd;

   y=atoi(datestring.substr(0,4).c_str());
   m=atoi(datestring.substr(5,2).c_str());
   d=atoi(datestring.substr(8,2).c_str());

   return gd2jd(y,m,d);

}
/**
 *  \fn float GetAverage(float arr[], int n)
 *  \brief This function calculates the mean value of a image array.
 *  \param arr[] is the image
 *  \param n is the number of pixels in the image
 *
 */
float
GetAverage(float arr[], int n)
{
	int i=0;
	float avg=0;

	for(i=0;i<n;i++){
		avg+=arr[i]/n;
	}
	return avg;
}

#define ELEM_SWAP(a,b) {register float t=(a);(a)=(b);(b)=t; }

/** \fn  float GetMedian(float arr[], int n)
 * \brief This function is the quick_select routine based on the algorithm found
 * in Numerical Recipes in C.
 *
 * \param arr[] is the image
 * \param n is the number of pixels in the image
 * \return median value of an array
 */
double GetMedian(double arr[], int n){
	int low, high;
	int median;
	int middle, ll, hh;

	low = 0; high = n-1; median = (low + high) / 2;
	for (;;) {

		if (high <= low) /* One element only */
			return arr[median] ;

		if (high == low + 1) { /* Two elements only */
			if (arr[low] > arr[high])
				ELEM_SWAP(arr[low], arr[high]) ;
			return arr[median] ;
		}

		/* Find median of low, middle and high items; swap into position low */
		middle = (low + high) / 2;
		if (arr[middle] > arr[high])
			ELEM_SWAP(arr[middle], arr[high])
		if (arr[low] > arr[high])
			ELEM_SWAP(arr[low], arr[high])
		if (arr[middle] > arr[low])
			ELEM_SWAP(arr[middle], arr[low])

		/* Swap low item (now in position middle) into position (low+1) */
		ELEM_SWAP(arr[middle], arr[low+1]) ;

		/* Nibble from each end towards middle, swapping items when stuck */
		ll = low + 1;
		hh = high;
		for (;;) {
			do ll++; while (arr[low] > arr[ll]) ;
			do hh--; while (arr[hh] > arr[low]) ;

				if (hh < ll) break;

				ELEM_SWAP(arr[ll], arr[hh])
		}

		/* Swap middle item (in position low) back into correct position */
		ELEM_SWAP(arr[low], arr[hh])

		/* Re-set active partition */
		if (hh <= median) low = ll;
		if (hh >= median) high = hh - 1;
	}

}
#undef ELEM_SWAP

/**
 * \fn float GetMedianDev(float arr[],  int n, float median)
 * \brief
 *
 */
double GetMedianDev(double arr[],  int n, double median)
{
   int i;

   /* Calculate the median deviation at each location in the array */
   for (i = 0; i < n; i++) {
      arr[i] = (double)fabs(arr[i] - median);
   }

   /**
    * Return the median deviation
    *
    * 0.674433 is magic number such that this deviation is the same as a
    * classic standard deviation assuming a normal distribution function
    * (gaussian)
    */
   return GetMedian(arr, n) / 0.674433;
}


