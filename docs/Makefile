LATEX    = latex >/dev/null
DVIPS    = dvips &>/dev/null
PDFLATEX = pdflatex >/dev/null
THUMBPDF = thumbpdf >/dev/null
RM       = rm -f
CP       = cp -f
MV       = mv -f
GZIP     = gzip --best

TEX  = $(wildcard *.tex)
DVI  = $(TEX:.tex=.dvi)
PS   = $(TEX:.tex=.ps)
PSGZ = $(TEX:.tex=.ps.gz)
PDF  = $(TEX:.tex=.pdf)
TMP  = $(TEX:.tex=.aux) $(TEX:.tex=.log) $(TEX:.tex=.out) $(TEX:.tex=.tpt)

.SUFFIXES: .tex .dvi .ps .pdf
.PHONY:    default all dvi ps psgz pdf clean cleantmp

%.dvi: %.tex
	$(LATEX) $<

%.ps: %.dvi
	$(DVIPS) $< -o $@

%.ps.gz: %.ps
	$(CP) $< $<.save
	$(RM) $@
	$(GZIP) $<
	$(MV) $<.save $<

%.pdf: %.tex
	$(PDFLATEX) $<
	-$(THUMBPDF) $@
	$(PDFLATEX) $<

#default: all
default: pdf cleantmp

all:  dvi ps psgz pdf
dvi:  $(DVI)
ps:   $(PS)
psgz: $(PSGZ)
pdf:  $(PDF)

clean:
	$(RM) $(DVI) $(PS) $(PSGZ) $(PDF) $(TMP)

cleantmp:
	$(RM) $(TMP)
