/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : dict.c
 *
 * Description : Fonctions permettant de g�rer un dictionnaire de mots
 *
 * Commentaire : Le dictionnaire utilise un arbre de recherche ternaire au
 *               moyen des fonctions impl�ment�es dans tstree.c.
 *
 * ---------------------------------------------------------------------------
 *
 * Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le
 * modifier conform�ment aux dispositions de la Licence Publique G�n�rale GNU,
 * telle que publi�e par la Free Software Foundation ; version 2 de la
 * licence, ou encore (� votre convenance) toute version ult�rieure.
 *
 * Ce programme est distribu� dans l'espoir qu'il sera utile, mais SANS AUCUNE
 * GARANTIE ; sans m�me la garantie implicite de COMMERCIALISATION ou
 * D'ADAPTATION � UN OBJET PARTICULIER. Pour plus de d�tail, voir la Licence
 * Publique G�n�rale GNU.
 *
 * Vous devez avoir re�u un exemplaire de la Licence Publique G�n�rale GNU en
 * m�me temps que ce programme ; si ce n'est pas le cas, �crivez � la Free
 * Software Foundation Inc., 675 Mass Ave, Cambridge, MA 02139, �tats-Unis.
 *
 * ---------------------------------------------------------------------------
 */


/* En-t�tes standard */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* En-t�tes locaux */
#include "dict.h"
#include "tstree.h"
#include "alpha.h"


/*****************************************************************************
 *
 * TYPES DE DONN�ES
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
    unsigned int  max;    /* Nombre maximum d'�l�ments  */
    unsigned int  size;   /* Taille totale des �l�ments */
    tstree_node_t *nodes; /* Tableau d'�l�ments         */
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
 * Cr�e un nouvel objet dictionnaire.
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
 * D�truit un objet dictionnaire.
 */
void dict_delete( dict_t dict )
{
    /* Contr�le des param�tres */
    assert( dict );

    /* Lib�ration de la m�moire */
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

    /* Contr�le des param�tres */
    assert( dict );
    assert( word );

    /* Il faut un mot d'au moins deux caract�res */
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
 * Cherche les `number' mots les plus utilis�s dans le dictionnaire.
 */
char **dict_get_most_used( const dict_t dict, char *word,
			   unsigned int number )
{
    /* Variables locales */
    unsigned int    i;        /* Compteur                         */
    char            *pos;     /* Position courante dans le tampon */
    char            **result; /* R�sultat : tableau de cha�nes    */
    callback_data_t data;     /* Donn�es pour le callback         */

    /* Contr�le des param�tres */
    assert( dict );

    /* Conversion en minuscules */
    if (word) {
	for (i = 0; word[i]; i++)
	    if (IS_UPPER_CASE( word[i] ))
		word[i] = UPPER_TO_LOWER_CASE( word[i] );
    } else
	word = "";

    /* Nombre maximal de mots � trouver */
    if (number == 0)
	number = tstree_get_key_number( dict->tree ) + 1;

    /* Allocation du tableau de mots */
    if (!(data.nodes = malloc( number * sizeof (tstree_node_t) )))
	return NULL;

    /* Initialisation des donn�es */
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

	/* Copie des mots dans le r�sultat */
	for (i = 0; i < number && data.nodes[i]; i++) {
	    if (!tstree_node_get_key_in_buffer( data.nodes[i], pos, 0 ))
		break;

	    result[i] = pos;
	    pos += tstree_node_get_depth( data.nodes[i] ) + 1;
	}

	/* Initialisation � z�ro des r�sultats inoccup�s dans le tampon */
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

    /* Lib�ration de la m�moire et retour du r�sultat */
    free( data.nodes );
    return result;
}

/**
 * Convertit le dictionnaire en une cha�ne de caract�res afin de le
 * sauvegarder dans un fichier.
 */
char *dict_get_words_into_string( const dict_t dict )
{
    /* Variables locales */
    unsigned int    i, j;    /* Compteurs                     */
    unsigned int    len;     /* Longueur d'un mot             */
    unsigned int    count;   /* Fr�quence d'un mot            */
    char            *result; /* R�sultat : le tampon          */
    char            *pos;    /* Position dans le tampon       */
    char            *copy;   /* Position pour la copie de mot */
    callback_data_t data;    /* Donn�es pour le callback      */

    /* Contr�le des param�tres */
    assert( dict );

    /* Allocation du tableau de mots */
    if (!(data.nodes = malloc( tstree_get_key_number( dict->tree ) *
			       sizeof (tstree_node_t) )))
	return NULL;

    /* Initialisation des donn�es */
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

	    /* Copie le mot plusieurs fois suivant sa fr�quence */
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

	/* Z�ro terminal et gestion d'erreur */
	if (i == data.max)
	    *pos = '\0';
	else {
	    free( result );
	    result = NULL;
	}
    }

    /* Lib�ration de la m�moire et retour du r�sultat */
    free( data.nodes );
    return result;
}

/**
 * Ajoute des mots au dictionnaire depuis une cha�ne de caract�res : tr�s
 * utile pour charger un dictionnaire depuis un fichier.
 */
bool_t dict_add_words_from_string( dict_t dict, char *string )
{
    char *pos;   /* Position courante dans la cha�ne                    */
    char *start; /* D�but d'un mot                                      */
    char save;   /* Sauvegarde du caract�re pr�c�dent pour le restaurer */

    /* Contr�le des param�tres */
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

	/* Parcourt les caract�res */
	start = pos++;
	while (IS_ALPHA( *pos ))
	    pos++;

	/* Si on mot a �t� trouv� */
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
 * Callback utilis� pour la d�couverte des mots.
 */
static bool_t dict_used_callback( const tstree_node_t node,
				  callback_data_t *data )
{
    /* Variables locales */
    unsigned int i, j;  /* Compteurs          */
    unsigned int count; /* Fr�quence d'un mot */

    /* Contr�le des param�tres */
    assert( node );
    assert( data );

    /* Calcul du nombre d'occurences du mot courant */
    count = tstree_node_get_count( node );

    /* Recherche d'une place pour l'insertion du mot */
    for (i = 0; i < data->max && data->nodes[i]; i++)
	if (tstree_node_get_count( data->nodes[i] ) < count)
	    break;

    /* D�calage des propositions suivantes et insertion du mot courant */
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
 * Callback utilis� pour la conversion du dictionnaire en cha�ne.
 */
static bool_t dict_string_callback( const tstree_node_t node,
				    callback_data_t *data )
{
    /* Contr�le des param�tres */
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
