MAINFILE = main
SRC = $(shell pwd)

# ***********************************************************
# ****** MAIN

.PHONY: all
all:  # all do nothing by default

.PHONY: pdf
pdf: pdflatex pdflatex

# ***********************************************************
# ****** BUILD RECIPES

# build using pdflatex
pdflatex:
	pdflatex -shell-escape -interaction=nonstopmode $(MAINFILE)

# read
read:
	evince $(MAINFILE).pdf &

.PHONY: pdflatex bib read

# ***********************************************************
# ****** CLEAN RECIPES
.PHONY: clean
clean:
	-rm -f *.aux
	-rm -f *.log
	-rm -f *.toc
	-rm -f *.bbl
	-rm -f *.blg
	-rm -f *.out
	-rm -f make/bib

.PHONY: cleanall
cleanall: clean
	-rm -f *.pdf
	-rm -f *.ps
	-rm -f *.dvi
	-rm -rf ./make
	-rm -f *.mtc*
	-rm -f *.maf
	-rm -f *.listing
	-rm -f *.loa
	-rm -f *.lof
	-rm -f *.lot
	-rm -f *.bcf
	-rm -f *.fls
	-rm -f *.pyg
	-rm -f *.run.xml
