PROD=  -O2 -D_GTKMAINBOROLL
TARGETDIR= /usr/bin
TARGETCONFDIR= ~/.Boroll
CONFIGMAIN= .bconf 
CONFIGEXT=   config
RESOURCES= res
LEX= flex
YACC= bison
USER=

CXXFLAGS =  -std=c++1y   -g -Wall -fmessage-length=0  `pkg-config --cflags gtkmm-3.0` `curl-config --cflags` -Ipugixml-1.9

BISONFLAGS= -d 
BISONSRC= syntaxTools/keyv.y
BISONTGT= keyv.cpp

FLEXFLAGS=
FLEXSRC= syntaxTools/keyvscanner.l
FLEXTGT= keyvf.cpp

SRC =BoAppLoop.cpp \
	 Bofiguration.cpp \
	 Boroll.cpp \
	 CurlWrap.cpp \
	 ShiftingFrame.cpp \
	 XMLPostFetch.cpp \
	 $(BISONTGT) \
	 $(FLEXTGT) \
	 pugixml-1.9/pugixml.cpp
	 

LIBS =`pkg-config --libs gtkmm-3.0` `curl-config --libs`

TARGET =boroll 
TARGETBIN=boroll_bin
TARGETLAUNCH=boroll_launcher

$(TARGET): $(FLEXTGT) $(SRC)
	$(CXX) -o $(TARGET) $(SRC)  $(LIBS) $(CXXFLAGS) $(PROD)

install:
	if [ -n "$(USER)" ];then \
		install -d -o $(USER) -g $(USER) $(TARGETCONFDIR); \
		else \
		install -d  $(TARGETCONFDIR); \
	fi
	cp -p $(CONFIGMAIN) $(CONFIGEXT) $(TARGETCONFDIR)
	cp -p $(TARGET)  $(TARGETDIR)/$(TARGETBIN)
	cp -rp $(TARGET)  $(TARGETDIR)/$(TARGETBIN)
	cp -rp $(RESOURCES)  $(TARGETCONFDIR)
	echo '#!/bin/sh' > $(TARGETLAUNCH)
	echo 'cd $(TARGETCONFDIR)'  >> $(TARGETLAUNCH)
	echo $(TARGETBIN)  >> $(TARGETLAUNCH)
	cp $(TARGETLAUNCH)  $(TARGETDIR)/$(TARGET)
	chmod +x $(TARGETDIR)/$(TARGET)
	
$(FLEXTGT) : $(BISONTGT)
	$(LEX)   $(FLEXFLAGS) -o $(FLEXTGT)  $(FLEXSRC)
	
$(BISONTGT):
	$(YACC)  $(BISONFLAGS) -o $(BISONTGT) $(BISONSRC)


all:	$(TARGET)

clean:
	rm -f $(TARGET) keyv* $(TARGETLAUNCH)
