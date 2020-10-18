.PHONY: clean all live pdf

all:
	latexmk -pdf -pv thesis

live:
	latexmk -pdf -pvc -silent thesis

clean:
	latexmk -C

pdf: all
	cp thesis.pdf ../thesis.pdf
