# ----------------------------------------------------------------------------
#
# Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
# Copyright (c) 2004 Benjamin Gaillard
#
# ----------------------------------------------------------------------------
#
# Fichier     : Makefile
#
# Description : Le fichier permettant de compiler et d'ex�cuter simplement
#               Act.
#
# Commentaire : Utiliser `make' pour compiler, `make depend' pour mettre �
#               jour les d�pendances dans Makefile.dep, `make clean' pour
#               supprimer les fichiers objet et le fichier ex�cutable, et
#               `make run' pour ex�cuter Act apr�s s'�tre assur� qu'il �tait �
#               jour.
#               Attention ! Penser � lancer `make clean' avant de compiler le
#               programme sur une autre architecture.
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

# Flags par d�faut
CC       ?= gcc
CFLAGS   ?= -O3 -fomit-frame-pointer -pipe
DEBUG    ?= -g -Werror -pipe
WARN     ?= -Wall -W -std=c99 -pedantic
CPPFLAGS ?=
LDFLAGS  ?= -s
RM       ?= rm -f
MV       ?= mv -f

# Programme r�sultant de la compilation
EXE = act

# Nom du Makefile et du fichier contenant les d�pendances
MAKEFILE = Makefile
DEPFILE  = Makefile.dep


##############################################################################
#
# � partir de ce point, ne rien �diter
#
#

# Ex�cution d'un script shell pour d�terminer le toolkit � utiliser
## GTK+ 2.x n'est pas support�
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

# D�finition des flags en fonction du toolkit utilis�
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
# R�gles g�n�riques
#
#

# Compilation d'un fichier source C
%.o: %.c $(MAKEFILE)
	echo "Compilation de \`$<'..."
	$(CC) $(CFLAGS) $(WARN) $(CPPFLAGS) -c $< -o $@


##############################################################################
#
# R�gles sp�ciales
#
#

# Ex�cution silencieuse (sans �cho) des commandes
.SILENT:

# Suffixes de fichier utilis�s
.SUFFIXES:
.SUFFIXES: .c .o

# R�gles ne g�n�rant pas de fichiers
.PHONY: default final debug all infos clean run depend depclean


##############################################################################
#
# R�gles g�n�rales
#
#

# Par d�faut, on compile tout
default: final
#default: debug

# Compilation sans information de d�bogage
final: CPPFLAGS += -DNDEBUG
final: all

# Compilation avec informations de d�bogage
debug: CFLAGS    = $(DEBUG)
debug: CPPFLAGS += -DDEBUG
debug: LDFLAGS  := $(filter-out -s,$(LDFLAGS))
debug: all

# D�pendances explicites
include $(DEPFILE)

# Tout compiler revient � lier le fichier ex�cutable
ifeq ($(CLEANDEP),1)
    # Mise � jour des d�pendances si elles sont vides
    all: depend $(EXE)
else
    # D�pendances existantes mises � jour automatiquement
    all: $(EXE)
endif

# Affiche des informations concernant la compilation
infos:
	echo 'C Compiler:    $(CC)'
	echo 'C Flags:       $(CFLAGS)'
	echo 'Warning Flags: $(WARN)'
	echo 'Toolkit:       $(TKNAME)'

# Liaison de l'ex�cutable
$(EXE): $(OBJ) $(MAKEFILE) $(DEPFILE)
	echo "Liaison de \`$@'..."
	$(CC) $(LDFLAGS) $(OBJ) -o $@

# Suppression des fichiers objets et de l'ex�cutable
clean: depclean
	echo 'Nettoyage du r�pertoire...'
	$(RM) $(OBJ) $(EXE) $(DEPFILE).bak *~ \#*\# core

# Ex�cution du programme
run: default
	echo "Ex�cution de \`$(EXE)' :"
	./$(EXE)


##############################################################################
#
# Gestion des d�pendances
#
#

# Mise � jour des d�pendances
$(DEPFILE): $(SRC) $(HDR)
	echo 'Mise � jour des d�pendances ($@)...'
	makedepend -f $@ -- $(CPPFLAGS) -- $^
	$(RM) $(DEPFILE).bak

# Mise � jour des toutes les d�pendances
depend:
	echo 'Mise � jour de toutes les d�pendances ($(DEPFILE))...'
	makedepend -f $(DEPFILE) -- $(CPPFLAGS) -- $(SRC) $(HDR)
	$(RM) $(DEPFILE).bak

# Mise � z�ro des d�pendances
depclean:
	echo 'Mise � z�ro des d�pendances ($(DEPFILE))...'
	head -n `grep 'DO NOT DELETE' -m 1 -n $(DEPFILE) | cut -f 1 -d :` \
	    $(DEPFILE) > $(DEPFILE).tmp
	$(MV) $(DEPFILE).tmp $(DEPFILE)
	echo >> $(DEPFILE)
	echo '# D�pendances vides' >> $(DEPFILE)
	echo 'CLEANDEP = 1' >> $(DEPFILE)

# Fin du fichier
