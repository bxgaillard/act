# ----------------------------------------------------------------------------
#
# Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
# Copyright (c) 2004 Benjamin Gaillard
#
# ----------------------------------------------------------------------------
#
# Fichier     : Makefile
#
# Description : Le makefile principal charg� d'appeler les autres makefiles.
#
# Commentaire : Utiliser `make' pour tout compiler, `make run' pour ex�cuter
#               le programme, `make docs' pour g�n�rer la documentation et
#               `make clean' pour tout nettoyer.
#
# ----------------------------------------------------------------------------
#
# Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le
# modifier conform�ment aux dispositions de la Licence Publique G�n�rale GNU,
# telle que publi�e par la Free Software Foundation ; version 2 de la
# licence, ou encore (� votre convenance) toute version ult�rieure.
#
# Ce programme est distribu� dans l'espoir qu'il sera utile, mais SANS AUCUNE
# GARANTIE ; sans m�me la garantie implicite de COMMERCIALISATION ou
# D'ADAPTATION � UN OBJET PARTICULIER. Pour plus de d�tail, voir la Licence
# Publique G�n�rale GNU.
#
# Vous devez avoir re�u un exemplaire de la Licence Publique G�n�rale GNU en
# m�me temps que ce programme ; si ce n'est pas le cas, �crivez � la Free
# Software Foundation Inc., 675 Mass Ave, Cambridge, MA 02139, �tats-Unis.
#
# ----------------------------------------------------------------------------


##############################################################################
#
# Param�tres de compilation �ditables
#
#

# Commandes par d�faut
RM ?= rm -f
CP ?= cp -f

# R�pertoires
TOPDIR = .
SRCDIR = $(TOPDIR)/src
DOCDIR = $(TOPDIR)/docs

# Programme r�sultant de la compilation
EXE = act


##############################################################################
#
# � partir de ce point, ne rien �diter
#
#

# Cibles phoniques
.PHONY: default all exe run docs clean

# Cible par d�faut
default: all

# Tout compiler
all: exe docs

# Compiler l'ex�cutable
exe:
	$(MAKE) -C $(SRCDIR)
	$(CP) $(SRCDIR)/$(EXE) $(TOPDIR)

# Ex�cuter le programme
run: exe
	$(TOPDIR)/$(EXE)

# G�n�rer la documentation
docs:
	$(MAKE) -C $(DOCDIR)

# Nettoyer le r�pertoire
clean:
	$(MAKE) -C $(SRCDIR) clean
	$(MAKE) -C $(DOCDIR) clean
	$(RM) $(EXE)
