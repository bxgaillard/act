/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : alpha.h
 *
 * Description : Ce fichier contient quelques macros bien utiles concernant
 *               l'identification des lettre et de leur casse.
 *
 * Commentaire : Ces macros ne fonctionnent qu'avec le charset ISO-8859-1.
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
#ifndef _ALPHA_H_
#define _ALPHA_H_


/* Macro servant à déterminer si un caractère est une lettre, c'est-à-dire
 * respectant l'expression rationnelle [A-Za-zÀ-ÖØ-Þà-öø-þ] */
#define IS_ALPHA( c ) (((c) >= 'A' && (c) <= 'Z') || \
		       ((c) >= 'a' && (c) <= 'z') || \
		       ((c) >= 'À' && (c) <= 'Ö') || \
		       ((c) >= 'Ø' && (c) <= 'Þ') || \
		       ((c) >= 'à' && (c) <= 'ö') || \
		       ((c) >= 'ø' && (c) <= 'þ'))

/* Macro servant à déterminer si un caractère est un chiffre, c'est-à-dire
 * respectant l'expression rationnelle [0-9] */
#define IS_DIGIT( c ) ((c) >= '0' && (c) <= '9')

/* Macro servant à déterminer si une lettre est majuscule */
#define IS_UPPER_CASE( c ) (((c) >= 'A' && (c) <= 'Z') || \
			    ((c) >= 'À' && (c) <= 'Ö') || \
			    ((c) >= 'Ø' && (c) <= 'Þ'))

/* Macro servant à déterminer si une lettre est minuscule */
#define IS_LOWER_CASE( c ) (((c) >= 'a' && (c) <= 'z') || \
			    ((c) >= 'à' && (c) <= 'ö') || \
			    ((c) >= 'ø' && (c) <= 'þ'))

/* Macro servant à convertir une majuscule en minuscule */
#define UPPER_TO_LOWER_CASE( c ) ((c) + ('a' - 'A'))

/* Macro servant à convertir une minuscule en majuscule */
#define LOWER_TO_UPPER_CASE( c ) ((c) - ('a' - 'A'))


#endif /* !_ALPHA_H_ */

/* Fin du fichier */
