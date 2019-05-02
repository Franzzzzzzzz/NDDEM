ifndef ${CC}
CC=/opt/local/bin/g++-mp-6 # FG
endif
#CC=g++-8 # BM
#CC=g++

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
	$(MAKE) all CC=$(CC) --directory=$@
	#find $(alltarget) -type f -perm -u=x -exec cp {} bin \;
	cp Dem/DEMND Texturing/Texturing CoarseGraining/IO ./bin

$(results): $(alltarget)
