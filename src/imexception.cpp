/*
 * imexception.cpp
 *
 *  Created on: Jul 27, 2011
 *      Author: chyan
 */
#include <stdio.h>
#include <cstring>
using namespace std;

#include "imexception.h"

void FITSIOException::setMessage(){
	string string,substring;
	stringstream fbuffer,pbuffer,lbuffer;

	fbuffer << file;
	if (fbuffer.str().length() != 0 ){
		substring=fbuffer.str();
	}
	pbuffer << function;
	if (pbuffer.str().length() != 0 ){
		substring=substring+":"+pbuffer.str();
	}

	lbuffer << line;
	if (line != -1 ){
		substring=substring+":"+lbuffer.str();
	}

	if (substring.length() != 0 ){
		string="FITIOException: "+cfitsioerror[status]+"("+substring+")";
	} else {
		string="FITIOException: "+cfitsioerror[status];
	}


	message = (char *)malloc(sizeof(char)*(string.size()+1));
	std::copy(string.begin(), string.end(), message);
	message[string.size()] = '\0'; // don't forget the terminating 0

}


char* FITSIOException::getMessage(){
	return message;

}


void FITSIOException::intitErrorMessage(){

	cfitsioerror[101]="input and output files are the same";
	cfitsioerror[103]="tried to open too many FITS files at once";
	cfitsioerror[104]="could not open the named file";
	cfitsioerror[105]="could not create the named file";
	cfitsioerror[106]="error writing to FITS file";
	cfitsioerror[107]="tried to move past end of file";
	cfitsioerror[108]="error reading from FITS file";
	cfitsioerror[110]="could not close the file";
	cfitsioerror[111]="array dimensions exceed internal limit";
	cfitsioerror[112]="cannot write to readonly file";
	cfitsioerror[113]="could not allocate memory";
	cfitsioerror[114]="invalid fitsfile pointer";
	cfitsioerror[115]="NULL input pointer to routine";
	cfitsioerror[116]="error seeking position in file";

	cfitsioerror[121]="invalid URL prefix on file name";
	cfitsioerror[122]="tried to register too many IO drivers";
	cfitsioerror[123]="driver initialization failed";
	cfitsioerror[124]="matching driver is not registered";
	cfitsioerror[125]="failed to parse input file URL";

	cfitsioerror[151]="bad argument in shared memory driver";
	cfitsioerror[152]="null pointer passed as an argument";
	cfitsioerror[153]="no more free shared memory handles";
	cfitsioerror[154]="shared memory driver is not initialized";
	cfitsioerror[155]="IPC error returned by a system call";
	cfitsioerror[156]="no memory in shared memory driver";
	cfitsioerror[157]="resource deadlock would occur";
	cfitsioerror[158]="attempt to open/create lock file failed";
	cfitsioerror[159]="shared memory block cannot be resized at the moment";

	cfitsioerror[201]="header already contains keywords";
	cfitsioerror[202]="keyword not found in header";
	cfitsioerror[203]="keyword record number is out of bounds";
	cfitsioerror[204]="keyword value field is blank";
	cfitsioerror[205]="string is missing the closing quote";
	cfitsioerror[207]="illegal character in keyword name or card";
	cfitsioerror[208]="required keywords out of order";
	cfitsioerror[209]="keyword value is not a positive integer";
	cfitsioerror[210]="couldn't find END keyword";
	cfitsioerror[211]="illegal BITPIX keyword value";
	cfitsioerror[212]="illegal NAXIS keyword value";
	cfitsioerror[213]="illegal NAXISn keyword value";
	cfitsioerror[214]="illegal PCOUNT keyword value";
	cfitsioerror[215]="illegal GCOUNT keyword value";
	cfitsioerror[216]="illegal TFIELDS keyword value";
	cfitsioerror[217]="negative table row size";
	cfitsioerror[218]="negative number of rows in table";
	cfitsioerror[219]="column with this name not found in table";
	cfitsioerror[220]="illegal value of SIMPLE keyword";
	cfitsioerror[221]="Primary array doesn't start with SIMPLE";
	cfitsioerror[222]="Second keyword not BITPIX";
	cfitsioerror[223]="Third keyword not NAXIS";
	cfitsioerror[224]="Couldn't find all the NAXISn keywords";
	cfitsioerror[225]="HDU doesn't start with XTENSION keyword";
	cfitsioerror[226]="the CHDU is not an ASCII table extension";
	cfitsioerror[227]="the CHDU is not a binary table extension";
	cfitsioerror[228]="couldn't find PCOUNT keyword";
	cfitsioerror[229]="couldn't find GCOUNT keyword";
	cfitsioerror[230]="couldn't find TFIELDS keyword";
	cfitsioerror[231]="couldn't find TBCOLn keyword";
	cfitsioerror[232]="couldn't find TFORMn keyword";
	cfitsioerror[233]="the CHDU is not an IMAGE extension";
	cfitsioerror[234]="TBCOLn keyword value < 0 or > rowlength";
	cfitsioerror[235]="the CHDU is not a table";
	cfitsioerror[236]="column is too wide to fit in table";
	cfitsioerror[237]="more than 1 column name matches template";
	cfitsioerror[241]="sum of column widths not = NAXIS1";
	cfitsioerror[251]="unrecognizable FITS extension type";
	cfitsioerror[252]="unknown record; 1st keyword not SIMPLE or XTENSION";
	cfitsioerror[253]="END keyword is not blank";
	cfitsioerror[254]="Header fill area contains non-blank chars";
	cfitsioerror[255]="Illegal data fill bytes (not zero or blank)";
	cfitsioerror[261]="illegal TFORM format code";

	file=NULL;
	function=NULL;
	line=-1;

}


void FITSIOException::setLine(int l){
	line=l;
}

int FITSIOException::getLine(){
	return line;
}

void FITSIOException::setFunction(const char* func){
	function=func;
}

const char* FITSIOException::getFunction(){
	return function;
}

void FITSIOException::setFile(const char* f){
	file=f;
}

const char* FITSIOException::getFile(){
	return file;
}


void FITSIOException::freeErrorMessage(){
}

void FITSIOException::dumpMessage(){
	if (message != NULL){
		fprintf(stderr, "%s\n ",message);
		free(message);
	}
}


FITSIOException::FITSIOException(){
	message=NULL;
	intitErrorMessage();
}


FITSIOException::FITSIOException(int s){
	message=NULL;
	intitErrorMessage();
	status=s;
	setMessage();
}

FITSIOException::FITSIOException(int s, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setMessage();
}

FITSIOException::FITSIOException(int s, const char* func, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setFunction(func);
	setMessage();
}

FITSIOException::FITSIOException(int s, const char* file, const char* func, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setFunction(func);
	setFile(file);
	setMessage();
}

FITSIOException::~FITSIOException() throw (){
}



void SextractorException::intitErrorMessage(){
	sextractorError[1]="The executable is not existed";
	sextractorError[2]="Could not establish working directory";
	sextractorError[3]="Could not delete working directory";
	sextractorError[4]="SEXtractor configuration file is not given";
	sextractorError[5]="Can not establish configuration file";
	sextractorError[6]="Can not copy target FITS file to working directory";
	sextractorError[7]="Can not copy weight map to working directory";
	sextractorError[7]="Running SExtractor is not successful";

}


void SextractorException::setMessage(){
	string string,substring;
	stringstream fbuffer,pbuffer,lbuffer;

	fbuffer << file;
	if (fbuffer.str().length() != 0 ){
		substring=fbuffer.str();
	}
	pbuffer << function;
	if (pbuffer.str().length() != 0 ){
		substring=substring+":"+pbuffer.str();
	}

	lbuffer << line;
	if (line != -1 ){
		substring=substring+":"+lbuffer.str();
	}

	if (substring.length() != 0 ){
		string="SextractorException: "+sextractorError[status]+" ("+substring+")";
	} else {
		string="SextractorException: "+sextractorError[status];
	}


	message = (char *)malloc(sizeof(char)*(string.size()+1));
	std::copy(string.begin(), string.end(), message);
	message[string.size()] = '\0'; // don't forget the terminating 0

}

SextractorException::SextractorException(){
	message=NULL;
	this->intitErrorMessage();
}


SextractorException::SextractorException(int s){
	message=NULL;
	intitErrorMessage();
	status=s;
	setMessage();
}

SextractorException::SextractorException(int s, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setMessage();
}

SextractorException::SextractorException(int s, const char* func, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setFunction(func);
	setMessage();
}

SextractorException::SextractorException(int s, const char* file, const char* func, int line){
	message=NULL;
	intitErrorMessage();
	status=s;
	setLine(line);
	setFunction(func);
	setFile(file);
	setMessage();
}














