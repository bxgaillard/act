# ----------------------------------------------------------------------------
#
# Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
# Copyright (c) 2004 Benjamin Gaillard
#
# ----------------------------------------------------------------------------
#
# Fichier     : Makefile
#
# Description : Le fichier permettant de compiler et d'exécuter simplement
#               Act.
#
# Commentaire : Utiliser `make' pour compiler, `make depend' pour mettre à
#               jour les dépendances dans Makefile.dep, `make clean' pour
#               supprimer les fichiers objet et le fichier exécutable, et
#               `make run' pour exécuter Act après s'être assuré qu'il était à
#               jour.
#               Attention ! Penser à lancer `make clean' avant de compiler le
#               programme sur une autre architecture.
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

# Flags par défaut
CC       ?= gcc
CFLAGS   ?= -O3 -fomit-frame-pointer -pipe
DEBUG    ?= -g -Werror -pipe
WARN     ?= -Wall -W -std=c99 -pedantic
CPPFLAGS ?=
LDFLAGS  ?= -s
RM       ?= rm -f
MV       ?= mv -f

# Programme résultant de la compilation
EXE = act

# Nom du Makefile et du fichier contenant les dépendances
MAKEFILE = Makefile
DEPFILE  = Makefile.dep


##############################################################################
#
# À partir de ce point, ne rien éditer
#
#

# Exécution d'un script shell pour déterminer le toolkit à utiliser
## GTK+ 2.x n'est pas supporté
#TK := $(shell                                                            \
#    if [ -x "`which pkg-config`" ] && pkg-config --exists gtk+-2.0; then \
#	echo GTK2;                                                       \
#    elif [ -x "`which gtk-config`" ]; then                               \
#	echo GTK1;                                                       \
#    else                                                                 \
#	echo NONE;                                                       \
#    fi                                                                   \
#)
TK := $(shell                                                            \
    if [ -x "`which gtk-config`" ]; then                                 \
	echo GTK1;                                                       \
    else                                                                 \
	echo NONE;                                                       \
    fi                                                                   \
)

# Définition des flags en fonction du toolkit utilisé
ifeq ($(TK),GTK2)
    TKNAME   := GTK+ 2.x
    INCLUDES := $(shell pkg-config --cflags gtk+-2.0)
    LIBS     := $(shell pkg-config --libs   gtk+-2.0)
else
    ifeq ($(TK),GTK1)
	TKNAME   := GTK+ 1.2.x
	INCLUDES := $(shell gtk-config --cflags)
	LIBS     := $(shell gtk-config --libs  )
    else
	TKNAME   := None
	INCLUDES :=
	LIBS     :=
    endif
endif
INCLUDES := -DUSE_$(TK) $(INCLUDES)

# Ajout de ces flags aux flags standard
CPPFLAGS += $(INCLUDES)
LDFLAGS  += $(LIBS)

# Fichiers source et objets
SRC := $(wildcard *.c)
HDR := $(wildcard *.h)
OBJ := $(SRC:.c=.o)


##############################################################################
#
# Règles génériques
#
#

# Compilation d'un fichier source C
%.o: %.c $(MAKEFILE)
	echo "Compilation de \`$<'..."
	$(CC) $(CFLAGS) $(WARN) $(CPPFLAGS) -c $< -o $@


##############################################################################
#
# Règles spéciales
#
#

# Exécution silencieuse (sans écho) des commandes
.SILENT:

# Suffixes de fichier utilisés
.SUFFIXES:
.SUFFIXES: .c .o

# Règles ne générant pas de fichiers
.PHONY: default final debug all infos clean run depend depclean


##############################################################################
#
# Règles générales
#
#

# Par défaut, on compile tout
default: final
#default: debug

# Compilation sans information de débogage
final: CPPFLAGS += -DNDEBUG
final: all

# Compilation avec informations de débogage
debug: CFLAGS    = $(DEBUG)
debug: CPPFLAGS += -DDEBUG
debug: LDFLAGS  := $(filter-out -s,$(LDFLAGS))
debug: all

# Dépendances explicites
include $(DEPFILE)

# Tout compiler revient à lier le fichier exécutable
ifeq ($(CLEANDEP),1)
    # Mise à jour des dépendances si elles sont vides
    all: depend $(EXE)
else
    # Dépendances existantes mises à jour automatiquement
    all: $(EXE)
endif

# Affiche des informations concernant la compilation
infos:
	echo 'C Compiler:    $(CC)'
	echo 'C Flags:       $(CFLAGS)'
	echo 'Warning Flags: $(WARN)'
	echo 'Toolkit:       $(TKNAME)'

# Liaison de l'exécutable
$(EXE): $(OBJ) $(MAKEFILE) $(DEPFILE)
	echo "Liaison de \`$@'..."
	$(CC) $(LDFLAGS) $(OBJ) -o $@

# Suppression des fichiers objets et de l'exécutable
clean: depclean
	echo 'Nettoyage du répertoire...'
	$(RM) $(OBJ) $(EXE) $(DEPFILE).bak *~ \#*\# core

# Exécution du programme
run: default
	echo "Exécution de \`$(EXE)' :"
	./$(EXE)


##############################################################################
#
# Gestion des dépendances
#
#

# Mise à jour des dépendances
$(DEPFILE): $(SRC) $(HDR)
	echo 'Mise à jour des dépendances ($@)...'
	makedepend -f $@ -- $(CPPFLAGS) -- $^
	$(RM) $(DEPFILE).bak

# Mise à jour des toutes les dépendances
depend:
	echo 'Mise à jour de toutes les dépendances ($(DEPFILE))...'
	makedepend -f $(DEPFILE) -- $(CPPFLAGS) -- $(SRC) $(HDR)
	$(RM) $(DEPFILE).bak

# Mise à zéro des dépendances
depclean:
	echo 'Mise à zéro des dépendances ($(DEPFILE))...'
	head -n `grep 'DO NOT DELETE' -m 1 -n $(DEPFILE) | cut -f 1 -d :` \
	    $(DEPFILE) > $(DEPFILE).tmp
	$(MV) $(DEPFILE).tmp $(DEPFILE)
	echo >> $(DEPFILE)
	echo '# Dépendances vides' >> $(DEPFILE)
	echo 'CLEANDEP = 1' >> $(DEPFILE)

# Fin du fichier
