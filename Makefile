# ----------------------------------------------------------------------------
#
# Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
# Copyright (c) 2004 Benjamin Gaillard
#
# ----------------------------------------------------------------------------
#
# Fichier     : Makefile
#
# Description : Le makefile principal chargé d'appeler les autres makefiles.
#
# Commentaire : Utiliser `make' pour tout compiler, `make run' pour exécuter
#               le programme, `make docs' pour générer la documentation et
#               `make clean' pour tout nettoyer.
#
# ----------------------------------------------------------------------------
#
# Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le
# modifier conformément aux dispositions de la Licence Publique Générale GNU,
# telle que publiée par la Free Software Foundation ; version 2 de la
# licence, ou encore (à votre convenance) toute version ultérieure.
#
# Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE
# GARANTIE ; sans même la garantie implicite de COMMERCIALISATION ou
# D'ADAPTATION À UN OBJET PARTICULIER. Pour plus de détail, voir la Licence
# Publique Générale GNU.
#
# Vous devez avoir reçu un exemplaire de la Licence Publique Générale GNU en
# même temps que ce programme ; si ce n'est pas le cas, écrivez à la Free
# Software Foundation Inc., 675 Mass Ave, Cambridge, MA 02139, États-Unis.
#
# ----------------------------------------------------------------------------


##############################################################################
#
# Paramètres de compilation éditables
#
#

# Commandes par défaut
RM ?= rm -f
CP ?= cp -f

# Répertoires
TOPDIR = .
SRCDIR = $(TOPDIR)/src
DOCDIR = $(TOPDIR)/docs

# Programme résultant de la compilation
EXE = act


##############################################################################
#
# À partir de ce point, ne rien éditer
#
#

# Cibles phoniques
.PHONY: default all exe run docs clean

# Cible par défaut
default: all

# Tout compiler
all: exe docs

# Compiler l'exécutable
exe:
	$(MAKE) -C $(SRCDIR)
	$(CP) $(SRCDIR)/$(EXE) $(TOPDIR)

# Exécuter le programme
run: exe
	$(TOPDIR)/$(EXE)

# Générer la documentation
docs:
	$(MAKE) -C $(DOCDIR)

# Nettoyer le répertoire
clean:
	$(MAKE) -C $(SRCDIR) clean
	$(MAKE) -C $(DOCDIR) clean
	$(RM) $(EXE)
