/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
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
 *               paramètres, voir le fichier `tstree.h'.
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
#ifndef _TSTREE_H_
#define _TSTREE_H_

/* En-têtes locaux */
#include "bool.h"

/* Traitement spécial si utilisation dans un programme C++ (début) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de données */
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


/* Traitement spécial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_TSTREE_H_ */

/* Fin du fichier */
