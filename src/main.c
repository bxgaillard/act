/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : main.c
 *
 * Description : Fonction principale du programme.
 *
 * Commentaire : Si un serveur X n'est pas trouvé (c'est-à-dire si la variable
 *               d'environnement DISPLAY n'est pas définie), lance une
 *               interface textuelle basique.
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


/* En-têtes standard */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* En-têtes locaux */
#include "interface.h"
#include "dict.h"
#include "huffman.h"


/*****************************************************************************
 *
 * FONCTION PRINCIPALE
 *
 */

/**
 * Fonction principale du programme, appelée par le système.
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
    char        **res;      /* Résultat des propositions */
    dict_t      dict;       /* Dictionnaire              */
#ifdef USE_GTK1
    interface_t interface;  /* Objet interface           */
    bool_t      result;     /* Résultat de l'exécution   */

    if (getenv( "DISPLAY" )) {
	/* Création de l'objet interface */
	if (!(interface = interface_new( argc, argv )))
	    return 1;

	/* Exécution et libération de la mémoire */
	result = interface_exec( interface );
	interface_delete( interface );

	/* Fin du programme */
	return result ? 0 : 1;
    }
#endif /* USE_GTK1 */

    /* Création du dictionnaire */
    dict = dict_new();

    /* Message d'accueil */
    puts("Act : Auto-Completion Tree\n"
	 "Copyright (c) 2004 Benjamin Gaillard\n"
	 "Ce programme est sous licence GPL ; lisez `LICENSE' pour plus "
	 "d'informations.\n\n"
#ifdef NDEBUG
	 "Si vous rencontrez des problèmes avec ce programme, compilez-le en "
	 "mode débogage.\n"
	 "Ainsi, vous serez plus à même de pister les erreurs...\n\n"
#elif defined(DEBUG)
	 "Ce programme a été compilé en mode débogage.\n"
	 "Il est donc possible que les performances ne soient pas au "
	 "rendez-vous...\n\n"
#endif
	 "Bienvenue dans l'interface en mode texte d'Act !\n"
	 "Si vous obtenez cette interface, il manque le serveur X.\n"
	 "Pour ajouter un mot au dictionnaire, entrez-le directement.\n"
	 "Sinon, entrez une commande. Pour les connaître, tapez \"?\" puis "
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
		    fputs( "Erreur d'écriture !\n", stderr );
	    } else
		puts( "Erreur de recherche des mots du dictionnaire !" );
	} else if (word[0] == '?')
	    puts( "Commandes disponibles :\n"
		  "    *[mot]     : recherche les mots commençant par `mot'\n"
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
