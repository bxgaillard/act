/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : dict.c
 *
 * Description : Fonctions permettant de gérer un dictionnaire de mots
 *
 * Commentaire : Le dictionnaire utilise un arbre de recherche ternaire au
 *               moyen des fonctions implémentées dans tstree.c.
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
#include <string.h>
#include <assert.h>

/* En-têtes locaux */
#include "dict.h"
#include "tstree.h"
#include "alpha.h"


/*****************************************************************************
 *
 * TYPES DE DONNÉES
 *
 */

/* Objet dictionnaire */
typedef struct dict
{
    tstree_t tree; /* Arbre ternaire de recherche */
}
dict_s_t;

typedef struct callback_data
{
    unsigned int  max;    /* Nombre maximum d'éléments  */
    unsigned int  size;   /* Taille totale des éléments */
    tstree_node_t *nodes; /* Tableau d'éléments         */
}
callback_data_t;


/*****************************************************************************
 *
 * PROTOTYPES DES FONCTIONS STATIQUES
 *
 */

/* Callbacks */
static bool_t dict_used_callback( const tstree_node_t node,
				  callback_data_t *data );
static bool_t dict_string_callback( const tstree_node_t node,
				    callback_data_t *data );


/*****************************************************************************
 *
 * FONCTIONS EXTERNES
 *
 */

/**
 * Crée un nouvel objet dictionnaire.
 */
dict_t dict_new( void )
{
    /* Variables locales */
    dict_t dict = malloc( sizeof (dict_s_t) ); /* Dictionnaire */

    /* Initialisation de l'arbre */
    if (dict) {
	if ((dict->tree = tstree_new()))
	    return dict;
	free( dict );
    }

    /* Erreur */
    return NULL;
}

/**
 * Détruit un objet dictionnaire.
 */
void dict_delete( dict_t dict )
{
    /* Contrôle des paramètres */
    assert( dict );

    /* Libération de la mémoire */
    tstree_delete( dict->tree );
    free( dict );
}

/**
 * Ajoute un mot au dictionnaire.
 */
bool_t dict_add( dict_t dict, char *word )
{
    /* Variables locales */
    int i; /* Compteur */

    /* Contrôle des paramètres */
    assert( dict );
    assert( word );

    /* Il faut un mot d'au moins deux caractères */
    if (word[0] == '\0' || word[1] == '\0')
	return FALSE;

    /* Conversion en minuscules */
    for (i = 0; word[i]; i++)
	if (IS_UPPER_CASE( word[i] ))
	    word[i] = UPPER_TO_LOWER_CASE( word[i] );

    /* Ajout du mot */
    return tstree_add_key( dict->tree, word ) ? TRUE : FALSE;
}

/**
 * Cherche les `number' mots les plus utilisés dans le dictionnaire.
 */
char **dict_get_most_used( const dict_t dict, char *word,
			   unsigned int number )
{
    /* Variables locales */
    unsigned int    i;        /* Compteur                         */
    char            *pos;     /* Position courante dans le tampon */
    char            **result; /* Résultat : tableau de chaînes    */
    callback_data_t data;     /* Données pour le callback         */

    /* Contrôle des paramètres */
    assert( dict );

    /* Conversion en minuscules */
    if (word) {
	for (i = 0; word[i]; i++)
	    if (IS_UPPER_CASE( word[i] ))
		word[i] = UPPER_TO_LOWER_CASE( word[i] );
    } else
	word = "";

    /* Nombre maximal de mots à trouver */
    if (number == 0)
	number = tstree_get_key_number( dict->tree ) + 1;

    /* Allocation du tableau de mots */
    if (!(data.nodes = malloc( number * sizeof (tstree_node_t) )))
	return NULL;

    /* Initialisation des données */
    result        = NULL;
    data.max      = number;
    data.size     = 0;
    data.nodes[0] = NULL;

    /* Recherche des mots */
    if (tstree_get_keys( dict->tree, word,
			 (tstree_callback_t) dict_used_callback, &data ) &&
	(result = malloc( number * sizeof (char *) +
			  data.size * sizeof (char) ))) {
	pos = (char *) (result + number);

	/* Copie des mots dans le résultat */
	for (i = 0; i < number && data.nodes[i]; i++) {
	    if (!tstree_node_get_key_in_buffer( data.nodes[i], pos, 0 ))
		break;

	    result[i] = pos;
	    pos += tstree_node_get_depth( data.nodes[i] ) + 1;
	}

	/* Initialisation à zéro des résultats inoccupés dans le tampon */
	if (i < number) {
	    if (!data.nodes[i])
		while (i < number)
		    result[i++] = NULL;
	    else {
		free( result );
		result = NULL;
	    }
	}
    }

    /* Libération de la mémoire et retour du résultat */
    free( data.nodes );
    return result;
}

/**
 * Convertit le dictionnaire en une chaîne de caractères afin de le
 * sauvegarder dans un fichier.
 */
char *dict_get_words_into_string( const dict_t dict )
{
    /* Variables locales */
    unsigned int    i, j;    /* Compteurs                     */
    unsigned int    len;     /* Longueur d'un mot             */
    unsigned int    count;   /* Fréquence d'un mot            */
    char            *result; /* Résultat : le tampon          */
    char            *pos;    /* Position dans le tampon       */
    char            *copy;   /* Position pour la copie de mot */
    callback_data_t data;    /* Données pour le callback      */

    /* Contrôle des paramètres */
    assert( dict );

    /* Allocation du tableau de mots */
    if (!(data.nodes = malloc( tstree_get_key_number( dict->tree ) *
			       sizeof (tstree_node_t) )))
	return NULL;

    /* Initialisation des données */
    result    = NULL;
    data.max  = 0;
    data.size = 0;

    /* Recherche des mots */
    if (tstree_get_keys( dict->tree, NULL,
			 (tstree_callback_t) dict_string_callback, &data ) &&
	(result = malloc( (data.size + 1) * sizeof (char) ))) {
	pos = result;

	/* Ajout des mots */
	for (i = 0; i < data.max; i++) {
	    if (!tstree_node_get_key_in_buffer( data.nodes[i], pos, 0 ))
		break;

	    len   = tstree_node_get_depth( data.nodes[i] );
	    count = tstree_node_get_count( data.nodes[i] );

	    /* Copie le mot plusieurs fois suivant sa fréquence */
	    if (count > 1) {
		copy = pos;
		pos += len++;
		*(pos++) = '\n';

		for (j = 1; j < count; j++) {
		    memcpy( pos, copy, len );
		    pos += len;
		}
	    } else {
		pos += len;
		*(pos++) = '\n';
	    }
	}

	/* Zéro terminal et gestion d'erreur */
	if (i == data.max)
	    *pos = '\0';
	else {
	    free( result );
	    result = NULL;
	}
    }

    /* Libération de la mémoire et retour du résultat */
    free( data.nodes );
    return result;
}

/**
 * Ajoute des mots au dictionnaire depuis une chaîne de caractères : très
 * utile pour charger un dictionnaire depuis un fichier.
 */
bool_t dict_add_words_from_string( dict_t dict, char *string )
{
    char *pos;   /* Position courante dans la chaîne                    */
    char *start; /* Début d'un mot                                      */
    char save;   /* Sauvegarde du caractère précédent pour le restaurer */

    /* Contrôle des paramètres */
    assert( dict );
    assert( string );

    /* Ajout des mots */
    pos = string;
    while (*pos != '\0') {
	/* Saute les blancs */
	while (!IS_ALPHA( *pos ))
	    if (*pos != '\0')
		pos++;
	    else
		return TRUE;

	/* Parcourt les caractères */
	start = pos++;
	while (IS_ALPHA( *pos ))
	    pos++;

	/* Si on mot a été trouvé */
	if (pos > start + 1) {
	    save = *pos;
	    *pos = '\0';
	    if (!dict_add( dict, start )) {
		*pos = save;
		return FALSE;
	    }
	    *pos = save;
	}
    }

    /* Pas d'erreur */
    return TRUE;
}


/*****************************************************************************
 *
 * FONCTIONS STATIQUES
 *
 */

/**
 * Callback utilisé pour la découverte des mots.
 */
static bool_t dict_used_callback( const tstree_node_t node,
				  callback_data_t *data )
{
    /* Variables locales */
    unsigned int i, j;  /* Compteurs          */
    unsigned int count; /* Fréquence d'un mot */

    /* Contrôle des paramètres */
    assert( node );
    assert( data );

    /* Calcul du nombre d'occurences du mot courant */
    count = tstree_node_get_count( node );

    /* Recherche d'une place pour l'insertion du mot */
    for (i = 0; i < data->max && data->nodes[i]; i++)
	if (tstree_node_get_count( data->nodes[i] ) < count)
	    break;

    /* Décalage des propositions suivantes et insertion du mot courant */
    if (i < data->max) {
	if (data->nodes[i]) {
	    j = i + 1;
	    while (j < data->max && data->nodes[j])
		j++;

	    if (j == data->max) {
		data->size -= tstree_node_get_depth( data->nodes[--j] ) + 1;
		j--;
	    } else if (j + 1 == data->max)
		j--;

	    do {
		data->nodes[j + 1] = data->nodes[j];

		if (j == 0)
		    break;
		j--;
	    } while (j >= i);
	} else if (i + 1 < data->max)
	    data->nodes[i + 1] = NULL;

	data->size += tstree_node_get_depth( node ) + 1;
	data->nodes[i] = node;
    }

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Callback utilisé pour la conversion du dictionnaire en chaîne.
 */
static bool_t dict_string_callback( const tstree_node_t node,
				    callback_data_t *data )
{
    /* Contrôle des paramètres */
    assert( node );
    assert( data );

    /* Ajout du mot */
    data->size += (tstree_node_get_depth( node ) + 1) *
	tstree_node_get_count( node );
    data->nodes[data->max++] = node;

    /* Pas d'erreur */
    return TRUE;
}

/* Fin du fichier */
