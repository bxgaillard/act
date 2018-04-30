/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : tstree.h
 *
 * Description : Ce fichier contient les prototypes des fonctions externes du
 *               fichier `tstree.h' pour pouvoir les utiliser dans d'autres
 *               modules.
 *
 * Commentaire : Pour plus d'informations sur les fonctions et leurs
 *               param�tres, voir le fichier `tstree.h'.
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
#ifndef _TSTREE_H_
#define _TSTREE_H_

/* En-t�tes locaux */
#include "bool.h"

/* Traitement sp�cial si utilisation dans un programme C++ (d�but) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de donn�es */
typedef struct tstree      *tstree_t;      /* Objet arbre          */
typedef struct tstree_node *tstree_node_t; /* Noeud de l'arbre     */
                                           /* Fonction de callback */
typedef bool_t            (*tstree_callback_t)( const tstree_node_t node,
						void *data );

/* Prototypes des fonctions externes */
tstree_t      tstree_new( void );
void          tstree_delete( tstree_t tree );
tstree_node_t tstree_get_root( const tstree_t tree );
unsigned int  tstree_get_depth( const tstree_t tree );
unsigned int  tstree_get_key_number( const tstree_t tree );
tstree_node_t tstree_add_key( tstree_t tree, const char *key );
bool_t        tstree_get_keys( const tstree_t tree, const char *key,
			       tstree_callback_t callback, void *data );

char         *tstree_node_get_key( const tstree_node_t node );
bool_t        tstree_node_get_key_in_buffer( const tstree_node_t node,
					     char *buffer,
					     unsigned int size );
unsigned int  tstree_node_get_depth( const tstree_node_t node );
unsigned int  tstree_node_get_count( const tstree_node_t node );


/* Traitement sp�cial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_TSTREE_H_ */

/* Fin du fichier */
