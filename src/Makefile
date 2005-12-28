################
# Call child makefiles
################
#SUBDIRS= GClasses MachineLearning Gasp Isotope

SUBDIRS= _GClasses _Gasp _Isotope


DBG_SUBDIRS= $(SUBDIRS:_%=DBG_%)
OPT_SUBDIRS= $(SUBDIRS:_%=OPT_%)


.PHONY: dbg opt $(DBG_SUBDIRS) $(OPT_SUBDIRS)

dbg: $(DBG_SUBDIRS)

opt: $(OPT_SUBDIRS)

$(DBG_SUBDIRS): DBG_% : 
	$(MAKE) -C $*

$(OPT_SUBDIRS): OPT_% : 
	$(MAKE) -C $* opt

################
# Dependencies
################
main: foo

################
# Clean
################
clean:
	$(MAKE) -C Isotope clean
	$(MAKE) -C Gasp clean
	$(MAKE) -C GClasses clean

