ifndef OSTYPE
  OSTYPE = $(shell uname -s|awk '{print tolower($$0)}')
endif

ifndef DOMAIN
  DOMAIN = $(shell hostname|awk -F. '{print $$2}')
  
endif

ifeq ($(OSTYPE),darwin)
	DATAPATH =	/Users/chyan
	CXXFLAGS =	-O0 -g -Wall -DDEBUG -fmessage-length=0 -Wno-write-strings -m64 -I/usr/local/include
	VGOPTION =	--dsymutil=yes --tool=memcheck --leak-check=full
	INSTALLPATH = ../bin
endif

ifeq ($(OSTYPE),linux)
	DATAPATH =	/data/chyan
ifeq ($(DOMAIN),asiaa)
	DATAPATH =	/data/local/chyan
endif	
	CXXFLAGS =	-O0 -g -Wall -DDEBUG -fmessage-length=0 -Wno-write-strings -m32 
	VGOPTION =	--tool=memcheck --leak-check=full
	INSTALLPATH = /cfht/bin
endif



ifndef CXXFLAG
	CXXFLAGS =	-O0 -g -Wall -DDEBUG -fmessage-length=0 -Wno-write-strings -I/usr/local/include
endif


OBJS =		imdetrend.o imskysub.o calculate.o image.o process.o imsky.o skybackground.o \
			imcrunch.o sextractor.o imsex.o imexception.o immask.o wcs.o imastro.o listextname.o \
			listhead.o 

LIBS = -lc -I/usr/local/include -lcfitsio -lm -L/usr/X11R6/lib -lpthread -lX11 

TARGET =	imdetrend imskysub imsky imcrunch imsex immask imastro listextname listhead


%.o : %.c
	$(CCXX) $(CXXFLAGS) -c -o $@ $< -I./ -I/usr/local/include
	
imdetrend : $(OBJS)
	$(CXX) $(CXXFLAGS) -o imdetrend imdetrend.o calculate.o image.o process.o imexception.o $(LIBS)
	mv $@ ../bin/
	
imskysub : $(OBJS)
	$(CXX) $(CXXFLAGS) -o imskysub imskysub.o calculate.o image.o process.o imexception.o $(LIBS)
	mv $@ ../bin/

imsky: $(OBJS)
	$(CXX) $(CXXFLAGS) -o imsky imsky.o skybackground.o image.o calculate.o process.o imexception.o $(LIBS)
	mv $@ ../bin/

imcrunch: $(OBJS)
	$(CXX) $(CXXFLAGS) -o imcrunch imcrunch.o image.o calculate.o process.o imexception.o $(LIBS)
	mv $@ ../bin/

imsex: $(OBJS)
	$(CXX) $(CXXFLAGS) -o imsex imsex.o sextractor.o imexception.o calculate.o $(LIBS)
	mv $@ ../bin/

immask: $(OBJS)
	$(CXX) $(CXXFLAGS) -o immask immask.o image.o process.o sextractor.o imexception.o calculate.o $(LIBS)
	mv $@ ../bin/

imastro: $(OBJS)
	$(CXX) $(CXXFLAGS) -o imastro imastro.o image.o sextractor.o imexception.o calculate.o wcs.o $(LIBS)
	mv $@ ../bin/

listextname: $(OBJS)
	$(CXX) $(CXXFLAGS) -o listextname listextname.o  $(LIBS)
	mv $@ ../bin/

listhead: $(OBJS)
	$(CXX) $(CXXFLAGS) -o listhead listhead.o  $(LIBS)
	mv $@ ../bin/

	
all: $(TARGET)
	

install: $(TARGET)
	@for file in $(TARGET); do \
      echo "--> Installing scripts $$file";\
      cp  -fp ../bin/$$file $(INSTALLPATH)/$$file-$(shell date +%y%m%d) ;\
      cp  -fp ../bin/$$file $(INSTALLPATH) ;\
	done
	 
pushmenka:
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/Makefile* -e ssh chyan@menka:/data/local/chyan/imtools/src	
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.h -e ssh chyan@menka:/data/local/chyan/imtools/src
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.cpp -e ssh chyan@menka:/data/local/chyan/imtools/src
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/Doxygen/* -e ssh chyan@menka:/data/local/chyan/imtools/Doxygen/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/php/* -e ssh chyan@menka:/data/local/chyan/imtools/php/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/perl/* -e ssh chyan@menka:/data/local/chyan/imtools/perl/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/conf/* -e ssh chyan@menka:/data/local/chyan/imtools/conf/
	
pushprocyon:
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/Makefile* -e ssh chyan@procyon:~chyan/imtools/src	
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.h -e ssh chyan@procyon:~chyan/imtools/src
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.cpp -e ssh chyan@procyon:~chyan/imtools/src
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/Doxygen/* -e ssh chyan@procyon:~chyan/imtools/Doxygen/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/php/* -e ssh chyan@procyon:~chyan/imtools/php/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/perl/* -e ssh chyan@procyon:~chyan/imtools/perl/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/conf/* -e ssh chyan@procyon:~chyan/imtools/conf/

pushcfht:
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/Makefile* -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/src/	
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.h -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/src/	
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/src/*.cpp -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/src/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/Doxygen/* -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/Doxygen/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/php/* -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/php/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/perl/* -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/perl/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/conf/* -e "ssh -1 -p 2266" chyan@polena:/data/chyan/imtools/conf/

pushcfhthome:
	rsync -arvz /Users/chyan/Documents/workspace/imtools/src/Makefile* -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/php/
	rsync -arvz /Users/chyan/Documents/workspace/imtools/src/*.h -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/src/	
	rsync -arvz /Users/chyan/Documents/workspace/imtools/src/*.cpp -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/src/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/Doxygen/* -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/Doxygen/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/php/* -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/php/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/perl/* -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/perl/
	rsync -arvzu /Users/chyan/Documents/workspace/imtools/conf/* -e "ssh -1 -p 2266" chyan@polena:/h/chyan/imtools/conf/
	
release: pushcfht pushcfhthome pushprocyon	

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf ../bin/*



include ./Makefile.test