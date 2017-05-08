

CC=gcc
CXX=g++
INC_PATH = src
CFLAGS += -g -MMD 
CFLAGS +=  -I$(INC_PATH) -I/usr/local/mysql/include -I/usr/include/sqlpg/ -I/usr/local/xerces-c/include
LDFLAGS +=  -lpthread  -L/usr/local/mysql/lib -lmysqlclient -lgsoap++ -L/usr/local/xerces-c/lib -lxerces-c -luuid
TARGET= ./bin/AdInspectAres
OUTPUT_PATH = ./obj


SUBDIR =database log para MarkJob  threadManage timeTask utility webservice TempletManager .

#…Ë÷√VPATH
EMPTY = 
SPACE = $(EMPTY)$(EMPTY) 
VPATH = $(subst $(SPACE), : ,$(strip $(foreach n,$(SUBDIR), $(INC_PATH)/$(n)))) : $(OUTPUT_PATH)

CXX_SOURCES = $(notdir $(foreach n, $(SUBDIR), $(wildcard $(INC_PATH)/$(n)/*.cpp)))
ifeq ($(findstring soapC.cpp,$(CXX_SOURCES)),)
  CXX_SOURCES:=soapC.cpp $(CXX_SOURCES)
endif

ifeq "$(findstring soapServer.cpp,$(CXX_SOURCES))" ""
  CXX_SOURCES:=soapServer.cpp $(CXX_SOURCES) 
endif
CXX_OBJECTS = $(patsubst  %.cpp,  %.o, $(CXX_SOURCES))
DEP_FILES = $(patsubst  %.cpp,  $(OUTPUT_PATH)/%.d, $(CXX_SOURCES))
all:mkdir $(TARGET)
.PHONY:all
$(TARGET):$(CXX_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(foreach n, $(CXX_OBJECTS), $(OUTPUT_PATH)/$(n))
	rm -f mons* soap*
	#******************************************************************************#
	#                          Bulid successful !                                  #
	#******************************************************************************#

%.o:%.cpp
	$(CXX) -c $(CFLAGS) -MT $@ -MF $(OUTPUT_PATH)/$(notdir $(patsubst  %.cpp, %.d,  $<)) -o $(OUTPUT_PATH)/$@ $< 

soapC.cpp soapServer.cpp:%.cpp:webservice.h
	soapcpp2 -S -xL $< ;cp mons* src/webservice/;cp soap* src/webservice/

-include $(DEP_FILES)




test:
	@echo $(VPATH)
	@echo $(CXX_OBJECTS)
	@echo $(CXX_SOURCES)
mkdir:
	mkdir -p $(dir $(TARGET))
	mkdir -p $(OUTPUT_PATH)
	
rmdir:
	rm -rf $(dir $(TARGET))
	rm -rf $(OUTPUT_PATH)

clean:
	rm -f $(OUTPUT_PATH)/*
	
