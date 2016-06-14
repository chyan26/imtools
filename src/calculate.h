/**
 * \file calculate.h
 *
 *  Created on: Aug 17, 2010
 *      Author: chyan
 */

#ifndef CALCULATE_H_
#define CALCULATE_H_

void stoupper(char *s);
void stolower(char *s);

char *sltrim(char *str);
/* Trim off trailing whitespace in a string */
char * srtrim(char *str);
/* Trim off all leading and trailing whitespace from a string */
char *strim(char *str);

long gd2jd(int y, int m, int d);

long str2jd(const char* gd);

float GetAverage(float arr[], int n);

double GetMedian(double arr[], int n);

double GetMedianDev(double arr[],  int n, double median);

#endif /* CALCULATE_H_ */
