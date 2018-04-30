/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
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
 *               param�tres, voir le fichier `dict.h'.
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


/* Pour ne pas include plusieurs fois cet en-t�te */
#ifndef _DICT_H_
#define _DICT_H_

/* En-t�tes locaux */
#include "bool.h"

/* Traitement sp�cial si utilisation dans un programme C++ (d�but) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de donn�es */
typedef struct dict *dict_t; /* Objet dictionnaire */

/* Prototypes des fonctions externes */
dict_t dict_new( void );
void   dict_delete( dict_t dict );
bool_t dict_add( dict_t dict, char *word );
char **dict_get_most_used( const dict_t dict, char *word,
			   unsigned int number );
char  *dict_get_words_into_string( const dict_t dict );
bool_t dict_add_words_from_string( dict_t dict, char *string );


/* Traitement sp�cial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_DICT_H_ */

/* Fin du fichier */
