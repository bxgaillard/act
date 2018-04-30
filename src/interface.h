/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : interface.h
 *
 * Description : Ce fichier contient les prototypes des fonctions externes du
 *               fichier `interface.h' pour pouvoir les utiliser dans d'autres
 *               modules.
 *
 * Commentaire : Pour plus d'informations sur les fonctions et leurs
 *               param�tres, voir le fichier `interface.c'.
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
#ifndef _INTERFACE_H_
#define _INTERFACE_H_

/* En-t�tes locaux */
#include "bool.h"

/* Traitement sp�cial si utilisation dans un programme C++ (d�but) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de donn�es */
typedef struct interface *interface_t;

/* Prototypes des fonctions externes */
interface_t interface_new( int argc, char **argv );
void        interface_delete( interface_t interface );
bool_t      interface_exec( interface_t interface );


/* Traitement sp�cial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_INTERFACE_H_ */

/* Fin du fichier */
