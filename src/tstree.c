/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : tstree.c
 *
 * Description : Ensemble de fonctions permettant de gérer un arbre de
 *               recherche ternaire.
 *
 * Commentaire : Un arbre ternaire permet au mieux d'effectuer des recherches
 *               dichotomiques et au pire est aussi rapide qu'un arbre
 *               binaire
 *
 * ---------------------------------------------------------------------------
 *
 * Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le
 * modifier conformément aux dispositions de la Licence Publique Générale GNU,
 * telle que publiée par la Free Software Foundation ; version 2 de la
 * licence, ou encore (à votre convenance) toute version ultérieure.
 *
 * Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE
 * GARANTIE ; sans même la garantie implicite de COMMERCIALISATION ou
 * D'ADAPTATION À UN OBJET PARTICULIER. Pour plus de détail, voir la Licence
 * Publique Générale GNU.
 *
 * Vous devez avoir reçu un exemplaire de la Licence Publique Générale GNU en
 * même temps que ce programme ; si ce n'est pas le cas, écrivez à la Free
 * Software Foundation Inc., 675 Mass Ave, Cambridge, MA 02139, États-Unis.
 *
 * ---------------------------------------------------------------------------
 */


/* En-têtes standard */
#include <stdlib.h>
#include <assert.h>

/* En-têtes locaux */
#include "tstree.h"


/*****************************************************************************
 *
 * TYPES DE DONNÉES
 *
 */

/* Objet arbre */
typedef struct tstree
{
    tstree_node_t root;  /* Racine                */
    unsigned int  count; /* Nombre de noeuds      */
    unsigned int  depth; /* Profondeur de l'arbre */
}
tstree_s_t;

/* Noeud de l'arbre */
typedef struct tstree_node
{
    tstree_node_t parent, brothers[2], child; /* Parents, frères et fils */
    unsigned int  depth;                      /* Profondeur du noeud     */
    char          chr;                        /* Caractère correspondant */
    unsigned int  count;                      /* Fréquence du mot        */
}
tstree_node_s_t;


/*****************************************************************************
 *
 * VARIABLES STATIQUES
 *
 */

static tstree_callback_t walk_callback;  /* Callback utilisé         */
static void              *callback_data; /* Données pour le callback */


/*****************************************************************************
 *
 * PROTOTYPES DES VARIABLES STATIQUES
 *
 */

static tstree_node_t tstree_node_new( const tstree_node_t parent, char chr );
static void          tstree_node_delete( tstree_node_t node );
static tstree_node_t tstree_get_node( const tstree_t tree, const char *key );
static bool_t        tstree_walk_subnodes( const tstree_node_t node );


/*****************************************************************************
 *
 * FONCTIONS EXTERNES
 *
 */

/**
 * Crée un nouvel objet arbre.
 */
tstree_t tstree_new( void )
{
    /* Variables locales */
    tstree_t tree = malloc( sizeof (tstree_s_t) ); /* L'arbre créé */

    /* Initialisation de l'objet */
    if (tree) {
	tree->root  = NULL;
	tree->count = 0;
	tree->depth = 0;

	return tree;
    }

    /* Erreur */
    return NULL;
}

/**
 * Détruit un objet arbre.
 */
void tstree_delete( tstree_t tree )
{
    /* Contrôle des paramètres */
    assert( tree );

    /* Libération de la mémoire */
    if (tree->root)
	tstree_node_delete( tree->root );
    free( tree );
}

/**
 * Retourne le noeud racine de l'arbre.
 */
tstree_node_t tstree_get_root( const tstree_t tree )
{
    assert( tree );
    return tree->root;
}

/**
 * Retourne la profondeur de l'arbre.
 */
unsigned int tstree_get_depth( const tstree_t tree )
{
    assert( tree );
    return tree->depth;
}

/**
 * Retourne le nombre de clés (mots) contenus dans l'arbre.
 */
unsigned int tstree_get_key_number( const tstree_t tree )
{
    assert( tree );
    return tree->count;
}

/**
 * Ajoute une clé (un mot) dans l'arbre.
 */
tstree_node_t tstree_add_key( tstree_t tree, const char *key )
{
    /* Variables locales */
    unsigned int    pos;    /* Caractère courant de la clé */
    tstree_node_t   node;   /* Noeud courant               */
    tstree_node_t   parent; /* Noeud parent                */
    tstree_node_t   *next;  /* Noeud suivant               */
    tstree_node_s_t root;   /* Racine de l'arbre           */

    /* Vérification des paramètres */
    assert( tree );
    assert( key );
    assert( key[0] != '\0' );

    /* Initialisation des données */
    root.child = tree->root;
    root.depth = 0;
    node = &root;

    /* Parcourt chaque caractère de la chaîne */
    for (pos = 0; key[pos]; pos++)
	if (!node->child) {
	    /* L'enfant existe, passe au caractère suivant */
	    if ((node->child = tstree_node_new( node, key[pos] )))
		node = node->child;
	    else
		return NULL;
	} else {
	    /* Création de l'enfant */
	    parent = node;
	    node = node->child;

	    while (node->chr != key[pos]) {
		next = node->brothers + (node->chr > key[pos] ? 0 : 1);

		if (*(next))
		    node = *next;
		else {
		    if ((*next = tstree_node_new( parent, key[pos] ))) {
			node = *next;
			break;
		    }
		    return NULL;
		}
	    }
	}

    /* Ajout de la clé au compteur */
    if (node->count == 0)
	tree->count++;
    node->count++;

    /* Mise à jour de la profondeur de l'arbre */
    tree->root = root.child;
    if (tree->depth < pos)
	tree->depth = pos;

    /* Retour du noeud créé */
    return node;
}

/**
 * Parcourt les noeuds et appelle un callback à chaque clé découverte.
 */
bool_t tstree_get_keys( const tstree_t tree, const char *key,
			tstree_callback_t callback, void *data )
{
    /* Variables locales */
    tstree_node_t node; /* Noeud courant */

    /* Vérification des paramètres */
    assert( tree );
    assert( callback );

    /* Effectue le parcours */
    if ((node = tstree_get_node( tree, key ))) {
	if (node->depth > 1 && node->parent->count != 0 &&
	    !callback( node->parent, data ))
	    return FALSE;

	walk_callback = callback;
	callback_data = data;

	return tstree_walk_subnodes( node );
    }

    /* Erreur */
    return FALSE;
}

/**
 * Obtient la clé (mot) correspondant à un noeud.
 */
char *tstree_node_get_key( const tstree_node_t node )
{
    /* Variables locales */
    unsigned int size;    /* Taille de la clé   */
    char         *buffer; /* Tampon pour la clé */

    /* Vérification des paramètre */
    assert( node );

    /* Calcul de la taille nécessaire pour la clé */
    size = node->depth + 1;

    /* Construction de la clé */
    if ((buffer = malloc( (node->depth + 1) * sizeof (char) ))) {
	if (tstree_node_get_key_in_buffer( node, buffer, size ))
	    return buffer;

	free( buffer );
	return NULL;
    }

    /* Erreur */
    return NULL;
}

/**
 * Obtient la clé (mot) correspondant à un noeud dans un tampon existant.
 */
bool_t tstree_node_get_key_in_buffer( const tstree_node_t node,
				      char *buffer, unsigned int size )
{
    /* Variables locales */
    unsigned int  pos;     /* Position dans la chaîne */
    tstree_node_t current; /* Noeud courant           */

    /* Vérification des paramètres */
    assert( node );
    assert( buffer );

    /* Calcul de la taille si nécessaire */
    if (size == 0)
	size = (unsigned int) -1;

    /* Initialisation de la chaîne */
    pos = node->depth;
    size--;
    buffer[pos < size ? pos : size] = '\0';

    /* Construction de la clé */
    for (current = node; current;current = current->parent) {
	pos--;
	if (pos < size)
	    buffer[pos] = current->chr;
	if (pos == 0)
	    break;
    }

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Retourne la profondeur d'un noeud.
 */
unsigned int tstree_node_get_depth( const tstree_node_t node )
{
    assert( node );
    return node->depth;
}

/**
 * Retourne le nombre d'occurences d'un noeud.
 */
unsigned int tstree_node_get_count( const tstree_node_t node )
{
    assert( node );
    return node->count;
}


/*****************************************************************************
 *
 * FONCTIONS STATIQUES
 *
 */

/**
 * Crée un nouveau noeud.
 */
static tstree_node_t tstree_node_new( const tstree_node_t parent, char chr )
{
    /* Variables locales */
    tstree_node_t node = malloc( sizeof (tstree_node_s_t) ); /* Noeud créé */

    /* Initialisation du noeud */
    if (node) {
	node->parent      = parent && parent->depth != 0 ? parent : NULL;
	node->brothers[0] = NULL;
	node->brothers[1] = NULL;
	node->child       = NULL;
	node->depth       = parent ? parent->depth + 1 : 1;
	node->chr         = chr;
	node->count       = 0;

	return node;
    }

    /* Erreur */
    return NULL;
}

/**
 * Détruit un noeud.
 */
static void tstree_node_delete( tstree_node_t node )
{
    /* Vérification des paramètres */
    assert( node );

    /* Destruction des frères et fils */
    if (node->brothers[0])
	tstree_node_delete( node->brothers[0] );
    if (node->brothers[1])
	tstree_node_delete( node->brothers[1] );
    if (node->child)
	tstree_node_delete( node->child );

    /* Libération de la mémoire */
    free( node );
}

/**
 * Obtient le noeud correspondant à une clé (mot) pas forcément entier.
 */
static tstree_node_t tstree_get_node( const tstree_t tree, const char *key )
{
    /* Variables locales */
    unsigned int    pos;  /* Position dans la chaîne */
    tstree_node_t   node; /* Noeud courant           */
    tstree_node_s_t root; /* Racine de l'arbre       */

    /* Vérification des paramètres */
    assert( tree );

    /* Parcourt les noeuds */
    if (key && key[0] != '\0') {
	root.child = tree->root;
	node = &root;

	for (pos = 0; key[pos]; pos++) {
	    if (!node->child)
		return NULL;

	    node = node->child;

	    while (node->chr != key[pos])
		if (!(node = node->brothers[node->chr > key[pos] ? 0 : 1]))
		    return NULL;
	}

	return node->child;
    }

    return tree->root;
}

/**
 * Parcourt les sous-noeuds d'un noeud récursivement.
 */
static bool_t tstree_walk_subnodes( const tstree_node_t node )
{
    /* Vérification des paramètres */
    assert( node );

    /* Si une clé correspond à ce noeud, appelle le callback */
    if (node->count != 0 && !walk_callback( node, callback_data ))
	return FALSE;

    /* S'appelle récursivement avec les frères et le fils */
    if (node->brothers[0] && !tstree_walk_subnodes( node->brothers[0] ))
	return FALSE;
    if (node->child && !tstree_walk_subnodes( node->child ))
	return FALSE;
    if (node->brothers[1] && !tstree_walk_subnodes( node->brothers[1] ))
	return FALSE;

    /* Pas d'erreur */
    return TRUE;
}

/* Fin du fichier */
