/*
 * wcs.h
 *
 *  Created on: Sep 12, 2012
 *      Author: chyan
 */

#ifndef WCS_H_
#define WCS_H_

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "fitsio.h"
#include "image.h"
#include "calculate.h"
#include "imparameter.h"
#include "imexception.h"

using namespace std;


class wcs {



public:
	wcs();

	~wcs();
};

#endif /* WCS_H_ */
