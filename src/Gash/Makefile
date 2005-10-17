################
# Call child makefiles
################
SUBDIRS= _Engine _MachineObjects _GashSDL _Command _../GClasses

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
Command: Engine MachineObjects GashSDL ../GClasses

################
# Clean
################
clean:
	$(MAKE) -C Command clean
