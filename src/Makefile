################
# Call child makefiles
################
#SUBDIRS= GClasses MachineLearning Gash Isotope
SUBDIRS= GClasses Gash Isotope

.PHONY: children $(SUBDIRS)

children: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

################
# Dependencies
################
main: foo

################
# Clean
################
clean:
	$(MAKE) -C Isotope clean
