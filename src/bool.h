/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : bool.h
 *
 * Description : Ce fichier sert à définir un type booléen en C.
 *
 * Commentaire : Le type s'appelle `bool_t'. Les valeurs vrai et faux sont
 *               respectivement `TRUE' et `FALSE'.
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
#ifndef _BOOL_H_
#define _BOOL_H_

/* Traitement spécial si utilisation dans un programme C++ (début) */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Types de données */
typedef char bool_t; /* Booléen */

/* Constantes */
#ifndef TRUE
#define TRUE  1
#endif /* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */


/* Traitement spécial si utilisation dans un programme C++ (fin) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_BOOL_H_ */

/* Fin du fichier */
