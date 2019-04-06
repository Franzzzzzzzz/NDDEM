dem       := Dem
texturing := Texturing
coarsening := CoarseGraining
alltarget := $(dem) $(texturing) $(coarsening)
results := res

.PHONY: all $(result) $(alltarget)
all: $(alltarget) 

clean :
	find * -name '*.o' -exec rm {} \;
	rm bin/*

$(results) $(alltarget):
	$(MAKE) all --directory=$@
	find $(alltarget) -type f -executable -exec cp {} bin \;

$(results): $(alltarget)
