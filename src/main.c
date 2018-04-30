/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : main.c
 *
 * Description : Fonction principale du programme.
 *
 * Commentaire : Si un serveur X n'est pas trouv� (c'est-�-dire si la variable
 *               d'environnement DISPLAY n'est pas d�finie), lance une
 *               interface textuelle basique.
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
#include <stdio.h>
#include <string.h>

/* En-t�tes locaux */
#include "interface.h"
#include "dict.h"
#include "huffman.h"


/*****************************************************************************
 *
 * FONCTION PRINCIPALE
 *
 */

/**
 * Fonction principale du programme, appel�e par le syst�me.
 */
#ifdef USE_NONE
int main( void )
#else /* USE_NONE */
int main( int argc, char **argv )
#endif /* !USE_NONE */
{
    /* Variables locales */
    int         i;          /* Compteur                  */
    char        word[128];  /* Mot lu                    */
    char        *str;       /* Tampon                    */
    char        **res;      /* R�sultat des propositions */
    dict_t      dict;       /* Dictionnaire              */
#ifdef USE_GTK1
    interface_t interface;  /* Objet interface           */
    bool_t      result;     /* R�sultat de l'ex�cution   */

    if (getenv( "DISPLAY" )) {
	/* Cr�ation de l'objet interface */
	if (!(interface = interface_new( argc, argv )))
	    return 1;

	/* Ex�cution et lib�ration de la m�moire */
	result = interface_exec( interface );
	interface_delete( interface );

	/* Fin du programme */
	return result ? 0 : 1;
    }
#endif /* USE_GTK1 */

    /* Cr�ation du dictionnaire */
    dict = dict_new();

    /* Message d'accueil */
    puts("Act : Auto-Completion Tree\n"
	 "Copyright (c) 2004 Benjamin Gaillard\n"
	 "Ce programme est sous licence GPL ; lisez `LICENSE' pour plus "
	 "d'informations.\n\n"
#ifdef NDEBUG
	 "Si vous rencontrez des probl�mes avec ce programme, compilez-le en "
	 "mode d�bogage.\n"
	 "Ainsi, vous serez plus � m�me de pister les erreurs...\n\n"
#elif defined(DEBUG)
	 "Ce programme a �t� compil� en mode d�bogage.\n"
	 "Il est donc possible que les performances ne soient pas au "
	 "rendez-vous...\n\n"
#endif
	 "Bienvenue dans l'interface en mode texte d'Act !\n"
	 "Si vous obtenez cette interface, il manque le serveur X.\n"
	 "Pour ajouter un mot au dictionnaire, entrez-le directement.\n"
	 "Sinon, entrez une commande. Pour les conna�tre, tapez \"?\" puis "
	 "validez.\n");

    /* Boucle principale */
    while (scanf( "%128s", word ) == 1)
	if (word[0] == '*') {
	    if ((res = dict_get_most_used( dict, word + 1, 0 ))) {
		for (i = 0; res[i]; i++)
		    printf( "    %s\n", res[i] );
		free( res );
	    } else
		fputs( "Erreur de recherche des mots !\n", stderr );
	} else if (word[0] == '<') {
	    if (huffman_read( word[1] == '\0' ? "dict.hdc" : word + 1,
			      &str, NULL )) {
		if (!dict_add_words_from_string( dict, str ))
		    fputs( "Erreur d'ajout de dictionnaire !\n", stderr );
		free( str );
	    } else
		fputs( "Erreur de lecture !\n", stderr );
	} else if (word[0] == '>') {
	    if ((str = dict_get_words_into_string( dict ))) {
		if (huffman_write( word[1] == '\0' ? "dict.hdc" : word + 1,
				   str, (unsigned int) -1 ))
		    free(str);
		else
		    fputs( "Erreur d'�criture !\n", stderr );
	    } else
		puts( "Erreur de recherche des mots du dictionnaire !" );
	} else if (word[0] == '?')
	    puts( "Commandes disponibles :\n"
		  "    *[mot]     : recherche les mots commen�ant par `mot'\n"
		  "    <[fichier] : ajoute les mots au dictionnaire\n"
		  "    >[fichier] : enregistre le dictionnaire\n"
		  "    ?          : affiche ce message d'aide\n"
		  "    .          : quitte le programme\n" );
	else if (word[0] == '.')
	    break;
	else if (!dict_add( dict, word ))
	    fputs( "Erreur d'ajout de mot !\n", stderr );

    /* Destruction du dictionnaire */
    dict_delete( dict );

    /* Fin sans erreur */
    return 0;
}
