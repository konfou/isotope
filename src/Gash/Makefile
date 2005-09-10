################
# Call child makefiles
################
SUBDIRS= Engine MachineObjects GashSDL Command ../GClasses

.PHONY: children $(SUBDIRS)

children: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

################
# Dependencies
################
Command: Engine MachineObjects GashSDL ../GClasses

################
# Clean
################
clean:
	$(MAKE) -C Command clean
