ObjSuf        = o
SrcSuf        = cpp
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o
HeadSuf       = h

include $(CMSSW_RELEASE_BASE)/.SCRAM/$(SCRAM_ARCH)/MakeData/Tools/boost_header.mk

ROOFIT_INCLUDE := $(shell cd $(CMSSW_BASE); scram tool info roofitcore | grep INCLUDE= | sed 's|INCLUDE=||')

ROOFIT_LIBDIR := $(shell cd $(CMSSW_BASE); scram tool info roofitcore | grep LIBDIR= | sed 's|LIBDIR=||')

INCLUDES = -I$(ROOFIT_INCLUDE)/ -I$(CMSSW_BASE)/src/JetMETCorrections/GammaJetFilter/bin/ -I$(boost_header_EX_INCLUDE)

ROOTSYS  ?= ERROR_RootSysIsNotDefined

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs) -lMinuit -lRooFitCore -lRooFit

CXX           = g++ -g
CXXFLAGS	    = -std=c++0x -Wall -fPIC
LD			      = g++
LDFLAGS		    = $(shell root-config --ldflags) -Wl,--as-needed
SOFLAGS		    = 
AR            = ar
ARFLAGS       = -cq

LDDIR         = $(shell echo "-L" $$LD_LIBRARY_PATH | sed -e 's/:/ -L/g') -L$(ROOFIT_LIBDIR)/ -L../lib/

CXXFLAGS	   += $(ROOTCFLAGS) $(INCLUDES)
LIBS  		    = $(LDDIR) $(ROOTLIBS)# -lAnalysis -lTreeAnalyzer_PhotonJet
GLIBS	    	  = $(ROOTGLIBS)
#------------------------------------------------------------------------------
SOURCES		= $(wildcard *.$(SrcSuf))
OBJECTS		= $(SOURCES:.$(SrcSuf)=.$(ObjSuf))
DEPENDS		= $(SOURCES:.$(SrcSuf)=.d)
SOBJECTS	= $(SOURCES:.$(SrcSuf)=.$(DllSuf))

.SUFFIXES: .$(SrcSuf) .$(ObjSuf)

###

#all: drawPhotonJet_2bkg drawPhotonJetExtrap draw_all_methods_vs_pt draw_all_methods_vs_npv draw_crosschecks
all: drawPhotonJet_2bkg drawPhotonJetExtrap draw_all_methods_vs_pt draw_ratios_vs_pt draw_new_extrap draw_vs_npv drawEnergyFractions draw_vs_run drawPhotonJet_2bkg_NOData

clean:
	@rm *.o;

drawPhotonJet_2bkg: drawBase.o fitTools.o drawPhotonJet_2bkg.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_vs_npv: drawBase.o fitTools.o draw_vs_npv.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_new_extrap: drawBase.o fitTools.o draw_new_extrap.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

drawPhotonJetExtrap: drawBase.o fitTools.o drawExtrap.o drawPhotonJetExtrap.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_all_methods_vs_pt: drawBase.o fitTools.o draw_all_methods_vs_pt.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_ratios_vs_pt: drawBase.o fitTools.o draw_ratios_vs_pt.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_all_methods_vs_npv: $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/DrawBase.o $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/fitTools.o draw_all_methods_vs_npv.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_crosschecks: $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/DrawBase.o $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/fitTools.o draw_crosschecks.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

drawSystPhotonJetExtrap: $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/DrawBase.o $(CMSSW_BASE)/src/UserCode/SBrochet/CommonTools/fitTools.o drawSystPhotonJetExtrap.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

drawEnergyFractions: drawBase.o fitTools.o drawEnergyFractions.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

draw_vs_run: drawBase.o fitTools.o draw_vs_run.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

drawPhotonJet_2bkg_NOData: drawBase.o fitTools.o drawPhotonJet_2bkg_NOData.o
	$(LD) $(LIBS) $(SOFLAGS) $(LDFLAGS) $+ -o $@

#draw_vs_run: draw_vs_run.cc
#		 g++ -o $@ $< `root-config --glibs --cflags`	

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


# Make the dependencies
%.d: %.cpp
	@echo "Generating dependencies for $<"
	@set -e; $(CXX) -M $(CXXFLAGS) $< \
	| sed 's%\($*\)\.o[ :]*%\1.o $@ : %g' > $@; \
	[ -s $@ ] || rm -f $@

ifneq ($(MAKECMDGOALS), clean) 
-include $(DEPENDS) 
endif 
