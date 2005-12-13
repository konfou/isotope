################
# Call child makefiles
################
SUBDIRS= _Engine _MachineObjects _GaspSDL _Command

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
Command: Engine MachineObjects GaspSDL

################
# Clean
################
clean:
	$(MAKE) -C Command clean
