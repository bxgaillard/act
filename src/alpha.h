/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
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
#ifndef _ALPHA_H_
#define _ALPHA_H_


/* Macro servant � d�terminer si un caract�re est une lettre, c'est-�-dire
 * respectant l'expression rationnelle [A-Za-z�-��-��-��-�] */
#define IS_ALPHA( c ) (((c) >= 'A' && (c) <= 'Z') || \
		       ((c) >= 'a' && (c) <= 'z') || \
		       ((c) >= '�' && (c) <= '�') || \
		       ((c) >= '�' && (c) <= '�') || \
		       ((c) >= '�' && (c) <= '�') || \
		       ((c) >= '�' && (c) <= '�'))

/* Macro servant � d�terminer si un caract�re est un chiffre, c'est-�-dire
 * respectant l'expression rationnelle [0-9] */
#define IS_DIGIT( c ) ((c) >= '0' && (c) <= '9')

/* Macro servant � d�terminer si une lettre est majuscule */
#define IS_UPPER_CASE( c ) (((c) >= 'A' && (c) <= 'Z') || \
			    ((c) >= '�' && (c) <= '�') || \
			    ((c) >= '�' && (c) <= '�'))

/* Macro servant � d�terminer si une lettre est minuscule */
#define IS_LOWER_CASE( c ) (((c) >= 'a' && (c) <= 'z') || \
			    ((c) >= '�' && (c) <= '�') || \
			    ((c) >= '�' && (c) <= '�'))

/* Macro servant � convertir une majuscule en minuscule */
#define UPPER_TO_LOWER_CASE( c ) ((c) + ('a' - 'A'))

/* Macro servant � convertir une minuscule en majuscule */
#define LOWER_TO_UPPER_CASE( c ) ((c) - ('a' - 'A'))


#endif /* !_ALPHA_H_ */

/* Fin du fichier */
