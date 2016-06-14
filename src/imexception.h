/*
 * exception.h
 *
 *  Created on: Jul 27, 2011
 *      Author: chyan
 */

#ifndef IMEXCEPTION_H_
#define IMEXCEPTION_H_

#include <iostream>
#include <exception>
#include <map>
#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <string>
#include <exception>
#include <vector>


using namespace std;



class FITSIOException: public exception {
	protected:
		map<int,string> cfitsioerror;
		int status;
		const char*  file;
		int line;
		const char* function;
		char* message;

	public:
		virtual void  setMessage();
		char* getMessage();

		virtual void intitErrorMessage();
		void freeErrorMessage();

		void setLine(int l);
		int getLine();
		void setFunction(const char* func);
		const char* getFunction();
		void setFile(const char* func);
		const char* getFile();

		void dumpMessage();

		FITSIOException();
		FITSIOException(int s);
		FITSIOException(int s, int line);
		FITSIOException(int s, const char* func, int line);
		FITSIOException(int s, const char* file, const char* func, int line);
		~FITSIOException() throw ();
};



class SextractorException: public FITSIOException{
	protected:
		map<int,string> sextractorError;
	public:
		typedef FITSIOException super;
		virtual void setMessage();
		virtual void intitErrorMessage();
		SextractorException();
		SextractorException(int s);
		SextractorException(int s, int line);
		SextractorException(int s, const char* func, int line);
		SextractorException(int s, const char* file, const char* func, int line);

};




#endif /* IMEXCEPTION_H_ */
