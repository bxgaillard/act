/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : dict.h
 *
 * Description : Ce fichier contient les prototypes des fonctions externes du
 *               fichier `dict.h' pour pouvoir les utiliser dans d'autres
 *               modules.
 *
 * Commentaire : Pour plus d'informations sur les fonctions et leurs
 *               paramètres, voir le fichier `dict.h'.
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


/* Pour ne pas include plusieurs fois cet en-tête */
#ifndef _DICT_H_
#define _DICT_H_

/* En-têtes locaux */
#include "bool.h"

/* Traitement spécial si utilisation dans un programme C++ (début) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de données */
typedef struct dict *dict_t; /* Objet dictionnaire */

/* Prototypes des fonctions externes */
dict_t dict_new( void );
void   dict_delete( dict_t dict );
bool_t dict_add( dict_t dict, char *word );
char **dict_get_most_used( const dict_t dict, char *word,
			   unsigned int number );
char  *dict_get_words_into_string( const dict_t dict );
bool_t dict_add_words_from_string( dict_t dict, char *string );


/* Traitement spécial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_DICT_H_ */

/* Fin du fichier */
