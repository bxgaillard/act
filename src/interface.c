/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : interface.c
 *
 * Description : Fonctions d'interface utilisateur graphique.
 *
 * Commentaire : L'interface utilisateur est en GTK+ 1.2.x. Ce code ne
 *               compilera pas avec GTK+ 2.x, bien que son adaptation ne
 *               devrait pas être trop compliquée.
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
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef USE_GTK1
#include <gtk/gtk.h>

/* En-têtes locaux */
#include "interface.h"
#include "dict.h"
#include "huffman.h"
#include "alpha.h"


/*****************************************************************************
 *
 * CONSTANTES
 *
 */


/* Nombre maximum de propositions à chercher dans le dictionnaire */
#define NUM_WORDS 10

/* Valeurs identifiant des boutons dans les boîtes de dialogue */
#define DIALOG_YES    0
#define DIALOG_NO     1
#define DIALOG_CANCEL 2


/*****************************************************************************
 *
 * DÉFINITIONS ET MACROS
 *
 */

/* GTK+ 2.x n'est pas supporté */
#ifdef USE_GTK2
#error "GTK+ 2.x n'est pas supporté."
#endif /* USE_GTK2 */

/* Pour éviter des avertissements de comparaison d'entiers signés et non
 * signés lors de la compilation */
#ifdef USE_GTK1
#undef GTK_TEXT_INDEX
#define GTK_TEXT_INDEX( t, index )                                        \
    (((t)->use_wchar)                                                     \
	? ((index) < (t)->gap_position ? (t)->text.wc[index] :            \
	   (t)->text.wc[(index) + (t)->gap_size])                         \
	: ((index) < (t)->gap_position ? (unsigned) (t)->text.ch[index] : \
	   (unsigned) (t)->text.ch[(index) + (t)->gap_size]))
#endif /* USE_GTK1 */

/* Définition utilisée pour supprimer les avertissements de paramètres
 * inutilisés lors de la compilation */
#ifdef __GNUC__
#define UNUSED __attribute__ ((__unused__))
#else /* __GNUC__ */
#define UNUSED
#endif /* !__GNUC__ */


/*****************************************************************************
 *
 * TYPES DE DONNÉES
 *
 */

/* Variables communes utilisées pour l'interface, partagées entre plusieurs
 * fonctions */
typedef struct interface
{
    bool_t       modified;  /* Si le texte a été modifié */
    char         *filename; /* Nom du fichier            */
    dict_t       dict;      /* Dictionnaire              */
    unsigned int length;    /* Longueur du début de mot  */
    char         **used;    /* Meilleures propositions   */
    unsigned int selected;  /* Mot sélectionné           */
    GtkWidget    *window;   /* Fenêtre principale        */
    GtkWidget    *text;     /* Texte édité               */
    GtkWidget    *list;     /* Liste de propositions     */
    GtkWidget    *button;   /* Bouton d'insertion        */
}
interface_s_t;


/*****************************************************************************
 *
 * PROTOTYPES DES FONCTIONS STATIQUES
 *
 */

/* Fonctions d'affichage de boîtes de dialogue */
static void         dialog_callback( GtkButton *button,
				     unsigned int selected );
static int          dialog_box( const char *title, const char *message,
				const char *const *buttons,
				unsigned int default_button );
static void         dialog_alert( const char *message );
static unsigned int dialog_yes_no( const char *message,
				   unsigned int default_button );
static unsigned int dialog_yes_no_cancel( const char *message,
					  unsigned int default_button );
static void         dialog_file_callback( GtkButton *button,
					  GtkFileSelection *selector);
static char        *dialog_file( const char *title, const char *mask,
				 bool_t save );

/* Ouverture et enregistrement de fichier */
static bool_t text_load( const char *filename, GtkText *text );
static bool_t text_save( const char *filename, GtkText *text );

/* Changement de titre de la fenêtre */
static void set_main_title( GtkWindow *window, const char *filename );

/* Callbacks des widgets principaux */
static char   **find_words( GtkText *text, interface_t interface );
static void     text_changed( GtkText *text, interface_t interface );
static void     text_inserted( GtkText *text, gchar *new_text,
			       gint new_text_length, gint *position,
			       interface_t interface );
static void     select_word( GtkList *list, GtkWidget *widget,
			     interface_t interface );
static void     insert_word( GtkButton *button, interface_t interface );
static gboolean delete_window( GtkWindow *window, GdkEvent *event,
			       interface_t interface );


/* Callbacks pour les menus */
static void menu_new( interface_t interface );
static void menu_open( interface_t interface );
static void menu_save( interface_t interface );
static void menu_save_as( interface_t interface );
static void menu_quit( interface_t interface );
static void menu_cut( interface_t interface );
static void menu_copy( interface_t interface );
static void menu_paste( interface_t interface );
static void menu_select_all( interface_t interface );
static void menu_clear_dict( interface_t interface );
static void menu_open_dict( interface_t interface );
static void menu_add_dict( interface_t interface );
static void menu_save_dict( interface_t interface );
static void menu_about( interface_t interface );


/*****************************************************************************
 *
 * VARIABLES GLOBALES STATIQUES
 *
 */

/* Variables utilisées pour les boîtes de dialogue */
static unsigned int dialog_selected;  /* Bouton sélectionné */
static char         *dialog_filename; /* Nom du fichier     */

/* Menus */
static GtkItemFactoryEntry menu_entries[] = {
    {"/_Fichier",                          NULL,         NULL,              0,
     "<Branch>"},
    {"/Fichier/_Nouveau",                  "<control>T", menu_new,          0,
     NULL},
    {"/Fichier/_Ouvrir...",                "<control>O", menu_open,         0,
     NULL},
    {"/Fichier/_Enregistrer",              "<control>S", menu_save,         0,
     NULL},
    {"/Fichier/Enregistrer _sous...",      "<control>R", menu_save_as,      0,
     NULL},
    {"/Fichier/-",                         NULL,         NULL,              0,
     "<Separator>"},
    {"/Fichier/_Quitter",                  "<control>Q", menu_quit,         0,
     NULL},
    {"/_Édition",                          NULL,         NULL,              0,
     "<Branch>"},
    {"/Édition/_Couper",                   "<control>X", menu_cut,          0,
     NULL},
    {"/Édition/Co_pier",                   "<control>C", menu_copy,         0,
     NULL},
    {"/Édition/Co_ller",                   "<control>V", menu_paste,        0,
     NULL},
    {"/Édition/-",                         NULL,         NULL,              0,
     "<Separator>"},
    {"/Édition/_Tout sélectionner",        "<control>Z", menu_select_all,   0,
     NULL},
    {"/_Dictionnaire",                     NULL,         NULL,              0,
     "<Branch>"},
    {"/Dictionnaire/_Effacer",             "<control>G", menu_clear_dict,   0,
     NULL},
    {"/Dictionnaire/_Ouvrir...",           "<control>I", menu_open_dict,    0,
     NULL},
    {"/Dictionnaire/_Ajouter...",          "<control>J", menu_add_dict,     0,
     NULL},
    {"/Dictionnaire/E_nregistrer...",      "<control>L", menu_save_dict,    0,
     NULL},
    {"/_Aide",                             NULL,         NULL,              0,
     "<Branch>"},
    {"/Aide/_À propos...",                 "<control>Y", menu_about,        0,
     NULL}
};


/*****************************************************************************
 *
 * FONCTIONS EXTERNES
 *
 */

/**
 * Crée un nouvel objet interface.
 */
interface_t interface_new( int argc, char **argv )
{
    /* Variables locales */
    GtkWidget      *vbox;      /* Boîte principale            */
    GtkWidget      *menubar;   /* Barre de menu               */
    GtkWidget      *hpaned;    /* Boîte horizontale           */
    GtkWidget      *text_hbox; /* Texte                       */
    GtkWidget      *scroll;    /* Ascenseur                   */
    GtkWidget      *list_vbox; /* Boîte pour la partie droite */
    GtkWidget      *frame;     /* Cadre pour la liste         */
    GtkWidget      *sep;       /* Séparateur pour la liste    */
    GtkItemFactory *factory;   /* Fabrique de menus           */
    GtkAccelGroup  *accel;     /* Groupe de raccourcis        */
    interface_t    interface;  /* Objet de l'interface        */

    /* Crée les objets */
    if (!(interface = malloc( sizeof (interface_s_t) )) ||
	!(interface->dict = dict_new()))
	return NULL;

    /* Initialisation de GTK+ */
    gtk_init( &argc, &argv );

    /* Fenêtre */
    interface->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    set_main_title( GTK_WINDOW( interface->window ), NULL );
    gtk_window_set_default_size( GTK_WINDOW( interface->window ), 400, 300 );
    gtk_signal_connect( GTK_OBJECT( interface->window ), "delete-event",
			GTK_SIGNAL_FUNC( delete_window ), interface );
    gtk_signal_connect( GTK_OBJECT( interface->window ), "destroy",
			GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

    /* Boîte pour le menu et le reste en-dessous */
    vbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( interface->window ), vbox );

    /* Menus */
    accel = gtk_accel_group_new();
    factory = gtk_item_factory_new( GTK_TYPE_MENU_BAR, "<main>", accel );
    gtk_item_factory_create_items( GTK_ITEM_FACTORY( factory ),
				   sizeof (menu_entries) /
				   sizeof (*menu_entries), menu_entries,
				   interface );
    gtk_accel_group_attach( accel, GTK_OBJECT( interface->window ) );
    menubar = gtk_item_factory_get_widget( GTK_ITEM_FACTORY( factory ),
					   "<main>" );
    gtk_box_pack_start( GTK_BOX( vbox ), menubar, FALSE, FALSE, 0 );

    /* Boîte avec séparateur pour le texte et la liste */
    hpaned = gtk_hpaned_new();
    gtk_paned_set_gutter_size( GTK_PANED( hpaned ), 16 );
    gtk_box_pack_start( GTK_BOX( vbox ), hpaned, TRUE, TRUE, 0 );

    /* Boîte horizontale pour le texte et l'ascenseur */
    text_hbox = gtk_hbox_new( FALSE, 0 );
    gtk_paned_pack1( GTK_PANED( hpaned ), text_hbox, TRUE, FALSE );

    /* Boîte de texte */
    interface->text = gtk_text_new( NULL, NULL );
    gtk_text_set_editable( GTK_TEXT( interface->text ), TRUE );
    gtk_text_set_word_wrap( GTK_TEXT( interface->text ), TRUE );
    gtk_signal_connect( GTK_OBJECT( interface->text ), "changed",
			GTK_SIGNAL_FUNC( text_changed ), interface );
    gtk_signal_connect_after( GTK_OBJECT( interface->text ), "insert-text",
			      GTK_SIGNAL_FUNC( text_inserted ), interface );
    gtk_box_pack_start( GTK_BOX( text_hbox ), interface->text,
			TRUE, TRUE, 0 );
    gtk_widget_grab_focus( interface->text );

    /* Ascenseur vertical */
    scroll = gtk_vscrollbar_new( GTK_TEXT( interface->text )->vadj );
    gtk_box_pack_end( GTK_BOX( text_hbox ), scroll, FALSE, FALSE, 0 );
    gtk_widget_grab_focus( interface->text );

    /* Boîte verticale pour la liste et le bouton d'insertion */
    list_vbox = gtk_vbox_new( FALSE, 4 );
    gtk_paned_pack2( GTK_PANED( hpaned ), list_vbox, TRUE, FALSE );

    /* Cadre de la liste */
    frame = gtk_frame_new( NULL );
    gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
    gtk_box_pack_start( GTK_BOX( list_vbox ), frame, TRUE, TRUE, 0 );

    /* Liste */
    interface->list = gtk_list_new();
    gtk_list_set_selection_mode( GTK_LIST( interface->list ),
				 GTK_SELECTION_BROWSE );
    gtk_signal_connect( GTK_OBJECT( interface->list ), "select-child",
			GTK_SIGNAL_FUNC( select_word ), interface );
    gtk_container_add( GTK_CONTAINER( frame ), interface->list );

    /* Séparateur */
    sep = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX( list_vbox ), sep, FALSE, FALSE, 0 );

    /* Bouton d'insertion */
    interface->button = gtk_button_new_with_label( "Insérer" );
    gtk_widget_set_sensitive( interface->button, FALSE );
    gtk_signal_connect( GTK_OBJECT( interface->button ), "clicked",
			GTK_SIGNAL_FUNC( insert_word ), interface );
    gtk_box_pack_end( GTK_BOX( list_vbox ), interface->button,
		      FALSE, FALSE, 0 );
    GTK_WIDGET_SET_FLAGS( interface->button, GTK_CAN_DEFAULT );
    gtk_widget_grab_default( interface->button );

    /* Affichage de tous les objets */
    gtk_widget_show_all( vbox );

    /* Initialisation des champs de l'objet */
    interface->modified = FALSE;
    interface->filename = NULL;
    interface->length   = 0;
    interface->used     = NULL;

    /* Retour de l'objet créé */
    return interface;
}

/**
 * Détruit un objet interface.
 */
void interface_delete( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );
    assert( interface->dict );

    /* Destruction des objets */
    if (interface->filename)
	free( interface->filename );
    if (interface->used)
	free( interface->used );
    dict_delete( interface->dict );
    free( interface );
}

/**
 * Lance l'interface.
 */
bool_t interface_exec( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Affichage de la fenêtre */
    gtk_widget_show( interface->window );

    /* Boucle principale de l'interface graphique */
    gtk_main();

    /* Pas d'erreur */
    return TRUE;
}


/*****************************************************************************
 *
 * FONCTIONS D'AFFICHAGE DE BOÎTES DE DIALOGUE
 *
 */

/**
 * Callback servant à enregistrer le bouton cliqué.
 */
static void dialog_callback( GtkButton *button UNUSED, unsigned int selected )
{
    if (dialog_selected == (unsigned int) -1)
	dialog_selected = selected;
}

/**
 * Affichage d'une boîte de dialogue générique.
 */
static int dialog_box( const char *title, const char *message,
		       const char *const *buttons,
		       unsigned int default_button )
{
    /* Variables locales */
    unsigned int i;
    GtkWidget    *dialog, *label, *button_box, *button; /* Widgets */

    /* Contrôle des paramètres */
    assert( title );
    assert( message );
    assert( buttons );

    /* Boîte de dialogue */
    dialog = gtk_dialog_new();
    gtk_window_set_title( GTK_WINDOW( dialog ), title );
    gtk_window_set_modal( GTK_WINDOW( dialog ), TRUE );
    gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER );
    gtk_signal_connect( GTK_OBJECT( dialog ), "destroy",
			GTK_SIGNAL_FUNC( dialog_callback ),
			(gpointer) default_button );

    /* Label contenant le message */
    label = gtk_label_new( message );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG( dialog )->vbox ),
			label, TRUE, TRUE, 10 );

    /* Boîte pour les boutons */
    button_box = gtk_hbox_new( TRUE, 10 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( dialog )->action_area ),
		       button_box );

    /* Boutons */
    i = 0;
    while (*buttons) {
	/* Création du bouton */
	button = gtk_button_new_with_label( *buttons );
	gtk_box_pack_start( GTK_BOX( button_box ), button, TRUE, TRUE, 0 );
	GTK_WIDGET_SET_FLAGS( button, GTK_CAN_DEFAULT );
	gtk_signal_connect( GTK_OBJECT( button ), "clicked",
			    GTK_SIGNAL_FUNC( dialog_callback ),
			    (gpointer) i );
	gtk_signal_connect_object( GTK_OBJECT( button ), "clicked",
				   GTK_SIGNAL_FUNC( gtk_widget_destroy ),
				   GTK_OBJECT( dialog ) );

	/* Bouton par défaut */
	if (i == default_button) {
	    gtk_widget_grab_default( button );
	    gtk_widget_grab_focus( button );
	}

	buttons++;
	i++;
    }

    /* Affiche la boîte de dialogue et attend la validation */
    gtk_widget_show_all( dialog );
    dialog_selected = (unsigned int) -1;
    while (dialog_selected == (unsigned int) -1)
	gtk_main_iteration();

    /* Retour du résultat */
    return dialog_selected;
}

/**
 * Affichage d'un avertissement dans une boîte de dialogue
 */
static void dialog_alert( const char *message )
{
    /* Variables locales */
    static const char *const buttons[] = {"OK", NULL}; /* Boutons */

    /* Contrôle des paramètres */
    assert( message );

    /* Affichage de la boîte de dialogue */
    dialog_box( "Alerte", message, buttons, 0 );
}

/**
 * Affichage d'une boîte de dialogue demandant à l'utilisateur de choisir
 * entre "Oui" et "Non".
 */
static unsigned int dialog_yes_no( const char *message,
				   unsigned int default_button )
{
    /* Variables locales */
    static const char *const buttons[] = {"Oui", "Non", NULL}; /* Boutons */

    /* Contrôle des paramètres */
    assert( message );

    /* Affichage de la boîte de dialogue */
    return dialog_box( "Question", message, buttons, default_button );
}

/**
 * Affichage d'une boîte de dialogue demandant à l'utilisateur de choisir
 * entre "Oui", "Non" et "Annuler".
 */
static unsigned int dialog_yes_no_cancel( const char *message,
					  unsigned int default_button )
{
    /* Variables locales */
    static const char *const buttons[] = /* Boutons */
	{"Oui", "Non", "Annuler", NULL};

    /* Contrôle des paramètres */
    assert( message );

    /* Affichage de la boîte de dialogue */
    return dialog_box( "Question", message, buttons, default_button );
}

/**
 * Callback utilisé pour enregistrer le nom de fichier sélectionné.
 */
static void dialog_file_callback( GtkButton *button UNUSED,
				  GtkFileSelection *selector)
{
    /* Variables locales */
    char *filename; /* Nom du fichier */

    if (dialog_selected == 0) {
	if (selector) {
	    /* Bouton OK : copie du nom de fichier dans un tampon alloué */
	    filename = gtk_file_selection_get_filename( selector );
	    if ((dialog_filename = malloc( strlen( filename ) + 1 )))
		strcpy( dialog_filename, filename );
	    dialog_selected = 1;
	} else {
	    /* Bouton Annuler ou fermeture de la fenêtre */
	    dialog_filename = NULL;
	    dialog_selected = 1;
	}
    }
}

/**
 * Affichage d'une boîte de dialogue permettant de sélectionner un fichier.
 */
static char *dialog_file( const char *title, const char *mask, bool_t save )
{
    /* Variables locales */
    GtkWidget   *selector; /* Sélecteur de fichier        */
    struct stat stats;     /* Informations sur le fichier */

    /* Contrôle des paramètres */
    assert( title );
    assert( mask );

    /* Création de la boîte de dialogue */
    selector = gtk_file_selection_new( title );
    gtk_file_selection_complete( GTK_FILE_SELECTION( selector ), mask );
    gtk_window_set_modal( GTK_WINDOW( selector ), TRUE );
    if (save)
	gtk_file_selection_show_fileop_buttons
	    ( GTK_FILE_SELECTION( selector ) );
    gtk_signal_connect( GTK_OBJECT( selector ), "destroy",
			GTK_SIGNAL_FUNC( dialog_file_callback ), NULL );

    /* Signaux du bouton OK */
    gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( selector )
				    ->ok_button ), "clicked",
			GTK_SIGNAL_FUNC( dialog_file_callback ),
			selector );
    gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( selector )
					   ->ok_button ), "clicked",
			       GTK_SIGNAL_FUNC( gtk_widget_destroy ),
			       GTK_OBJECT( selector ) );

    /* Signaux du bouton Annuler */
    gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( selector )
				    ->cancel_button ), "clicked",
			GTK_SIGNAL_FUNC( dialog_file_callback ), NULL );
    gtk_signal_connect_object( GTK_OBJECT( GTK_FILE_SELECTION( selector )
					   ->cancel_button ), "clicked",
			       GTK_SIGNAL_FUNC( gtk_widget_destroy ),
			       GTK_OBJECT( selector ) );

    /* Affiche la boîte de dialogue et attend la saisie */
    gtk_widget_show( selector );
    dialog_selected = 0;
    while (dialog_selected == 0)
	gtk_main_iteration();

    /* Vérifie l'existence du fichier */
    if (save && stat( dialog_filename, &stats ) == 0 &&
	dialog_yes_no( "Le fichier existe.\n"
		       "Souhaitez-vous l'écraser ?", DIALOG_NO )
	!= DIALOG_YES) {
	free( dialog_filename );
	return NULL;
    }

    /* Retour du nom de fichier */
    return dialog_filename;
}

/*****************************************************************************
 *
 * OUVERTURE ET ENREGISTREMENT DE FICHIER
 *
 */

/**
 * Charge un fichier dans la boîte de texte.
 */
static bool_t text_load( const char *filename, GtkText *text )
{
    /* Variables locales */
    int          fd;           /* Descripteur de fichier  */
    int          position;     /* Position dans le texte  */
    unsigned int size;         /* Taille des données lues */
    char         buffer[1024]; /* Tampon de lecture       */

    /* Contrôle des paramètres */
    assert( filename );
    assert( text );

    /* Ouvre le fichier */
    if ((fd = open( filename, O_RDONLY )) == -1) {
	dialog_alert( "Le fichier n'a pas pu être ouvert." );
	return FALSE;
    }

    /* Supprime tout le texte présent */
    gtk_text_freeze( text );
    gtk_editable_delete_text( GTK_EDITABLE( text ), 0, -1 );
    position = 0;

    /* Lit le fichier */
    while ((size = read( fd, buffer, sizeof (buffer) )) != 0) {
	gtk_editable_insert_text( GTK_EDITABLE( text ), buffer, size,
				  &position );
	if (size < sizeof (buffer))
	    break;
    }

    /* Ferme le fichier et place le curseur au début */
    close( fd );
    gtk_editable_set_position( GTK_EDITABLE( text ), 0 );
    gtk_text_thaw( text );
    return TRUE;
}

/**
 * Enregistre un fichier depuis la boîte de texte.
 */
static bool_t text_save( const char *filename, GtkText *text )
{
    /* Variables locales */
    int  fd;      /* Descripteur de fichier     */
    int  size;    /* Taille des données écrites */
    char *buffer; /* Tampon d'écriture          */

    /* Contrôle des paramètres */
    assert( filename );
    assert( text );

    /* Crée fichier */
    if ((fd = creat( filename, 0666 )) == -1) {
	dialog_alert( "Le fichier n'a pas pu être écrit." );
	return FALSE;
    }

    /* Écrit le fichier */
    buffer = gtk_editable_get_chars( GTK_EDITABLE( text ), 0, -1 );
    size = write( fd, buffer, gtk_text_get_length( text ) );

    /* Libère la mémoire et ferme le fichier */
    g_free( buffer );
    close( fd );

    /* Contrôle d'erreur */
    if (size == -1) {
	dialog_alert( "Erreur d'écriture du fichier." );
	return FALSE;
    }
    return TRUE;
}


/*****************************************************************************
 *
 * CHANGEMENT DE TITRE DE LA FENÊTRE
 *
 */

/**
 * Change le titre de la fenêtre en fonction du nom de fichier édité.
 */
static void set_main_title( GtkWindow *window, const char *filename )
{
    /* Variables locales */
    int        i;
    int        slash;
    char       *buffer = NULL;
    const char *final;
    const char name[] = "Act - ";
    const char default_name[] = "Act - <Nouveau>";
    const char fallback_name[] = "Act";

    /* Contrôle des paramètres */
    assert( window );

    /* Détermine le titre */
    if (filename && filename[0] != '\0') {
	slash = 0;
	for (i = 0; filename[i]; i++)
	    if (filename[i] == '/')
		slash = i + 1;

	if (slash != i) {
	    if ((buffer = malloc( sizeof (name) + (i - slash) ))) {
		memcpy( buffer, name, sizeof (name) - 1 );
		memcpy( buffer + (sizeof (name) - 1), filename + slash,
			i - slash + 1 );
		final = buffer;
	    } else
		final = fallback_name;
	} else
	    final = default_name;
    } else
	final = default_name;

    /* Change le titre */
    gtk_window_set_title( window, final );

    /* Libère la mémoire */
    if (buffer)
	free( buffer );
}


/*****************************************************************************
 *
 * CALLBACKS DES WIDGETS PRINCIPAUX
 *
 */

/**
 * Trouve les meilleurs propositions du dictionnaire.
 */
static char **find_words( GtkText *text, interface_t interface )
{
    /* Variables locales */
    unsigned int end;      /* Fin du mot                 */
    unsigned int start;    /* Début du mot               */
    char         chr;      /* Caractère courant          */
    char         *word;    /* Mot trouvé                 */
    char         **result; /* Résultat : tableau de mots */

    /* Contrôle des paramètres */
    assert( text );
    assert( interface );
    assert( interface->dict );

    /* Trouve les limites du mot */
    end = gtk_text_get_point( text );
    for (start = end; start != 0; start--) {
	chr = GTK_TEXT_INDEX( text, start - 1 );
	if (!IS_ALPHA( chr ))
	    break;
    }

    /* Calcul de la longueur du début de mot */
    interface->length = end - start;
    if (start == end)
	return NULL;

    /* Cherche tous les mots possibles et retourne le résultat */
    word = gtk_editable_get_chars( GTK_EDITABLE( text ), start, end );
    result = dict_get_most_used( interface->dict, word, NUM_WORDS );
    g_free( word );
    return result;
}

/**
 * Callback appelé lorsque le texte est modifié.
 */
static void text_changed( GtkText *text, interface_t interface )
{
    /* Variables locales */
    unsigned int i;            /* Compteur               */
    GtkWidget    *item;        /* Un item de la liste    */
    GList        *list = NULL; /* Nouvelle liste de mots */

    /* Contrôle des paramètres */
    assert( text );
    assert( interface );
    assert( interface->list );

    /* Efface la liste */
    gtk_list_clear_items( GTK_LIST( interface->list ), 0, -1 );

    /* Cherche les mots dans le dictionnaire */
    if (interface->used)
	free( interface->used );
    interface->used = find_words( text, interface );

    /* Rajoute chaque mot trouvé dans la liste */
    if (interface->used) {
	for (i = 0; i < NUM_WORDS && interface->used[i]; i++) {
	    item = gtk_list_item_new_with_label( interface->used[i] );
	    gtk_widget_show( item );
	    list = g_list_append( list, GTK_LIST_ITEM( item ) );
	}

	if (list) {
	    gtk_list_insert_items( GTK_LIST( interface->list ), list, 0 );
	    gtk_widget_set_sensitive( interface->button, TRUE );
	} else
	    gtk_widget_set_sensitive( interface->button, FALSE );
    } else
	gtk_widget_set_sensitive( interface->button, FALSE );

    /* Le texte est modifié */
    interface->modified = TRUE;
}

/**
 * Callback appelé quand tu texte est inséré.
 */
static void text_inserted( GtkText *text, gchar *new_text UNUSED,
			   gint new_text_length, gint *position,
			   interface_t interface )
{
    /* Variables locales */
    unsigned int pos;   /* Position courange      */
    unsigned int end;   /* Position de fin du mot */
    char         chr;   /* Caractère courant      */
    char         *word; /* Mot à ajouter          */

    /* Contrôle des paramètres */
    assert( text );
    assert( position );
    assert( interface );
    assert( interface->text );

    /* Si rien n'est ajouté, ne rien faire */
    if (new_text_length == 0)
	return;

    /* Se positionne sur le dernier mot */
    pos = *position;
    do {
	pos--;
	chr = GTK_TEXT_INDEX( text, pos );
    } while (pos != 0 && IS_ALPHA( chr ));

    /* Recherche les mots */
    while (pos != 0 && pos >= (unsigned int) *position - new_text_length) {
	/* Saute les blancs */
	while (pos != 0) {
	    chr = GTK_TEXT_INDEX( text, pos );
	    if (IS_ALPHA( chr ))
		break;
	    pos--;
	}
	end = pos + 1;

	/* Parcourt le mot */
	while (pos != 0) {
	    chr = GTK_TEXT_INDEX( text, pos );
	    if (!IS_ALPHA( chr ))
		break;
	    pos--;
	}
	if (!IS_ALPHA( chr ))
	    pos++;

	/* Ajoute le mot */
	if (pos != end) {
	    word = gtk_editable_get_chars( GTK_EDITABLE( text ), pos, end );
	    dict_add( interface->dict, word );
	    g_free( word );
	}

	/* Position suivante */
	if (pos != 0)
	    pos--;
    }
}

/**
 * Callback appelé lorsque l'utilisateur sélectionne un mot dans la liste.
 */
static void select_word( GtkList *list, GtkWidget *widget,
			 interface_t interface )
{
    /* Enregistre la sélection */
    interface->selected = gtk_list_child_position( list, widget );
}

/**
 * Callback appelé quand l'utilisateur clique sur le bouton d'insertion.
 */
static void insert_word( GtkButton *button UNUSED, interface_t interface )
{
    /* Variables locales */
    const char *word;    /* Mot à insérer          */
    int        position; /* Position dans le texte */

    /* Insère le mot */
    word = interface->used[interface->selected];
    position = gtk_editable_get_position( GTK_EDITABLE( interface->text ) );
    gtk_editable_insert_text( GTK_EDITABLE( interface->text ),
			      word + interface->length,
			      strlen( word ) - interface->length, &position );

    /* Redonne le focus à la boîte de texte */
    gtk_widget_grab_focus( interface->text );
}

/**
 * Callback appelé lors d'une tentative de fermeture de la fenêtre.
 */
static gboolean delete_window( GtkWindow *window UNUSED,
			       GdkEvent *event UNUSED, interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Sauvegarde du fichier si nécessaire */
    if (interface->modified) {
	switch (dialog_yes_no_cancel( "Le fichier a été modifié.\n"
				      "Souhaitez-vous le sauvegarder ?",
				      DIALOG_CANCEL )) {
	case DIALOG_YES:
	    menu_save( interface );
	    if (interface->modified)
		return TRUE;

	case DIALOG_NO:
	    break;

	default:
	    return TRUE;
	}
    }

    /* Libération de la mémoire */
    if (interface->filename) {
	free( interface->filename );
	interface->filename = NULL;
    }
    if (interface->used) {
	free( interface->used );
	interface->used = NULL;
    }

    /* Destruction de la fenêtre */
    return FALSE;
}


/*****************************************************************************
 *
 * CALLBACKS POUR LES MENUS
 *
 */

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Fichier/Nouveau.
 */
static void menu_new( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Sauvegarde du texte si demandé */
    if (interface->modified)
	switch (dialog_yes_no_cancel( "Le texte a été modifié.\n"
				      "Souhaitez-vous l'enregistrer ?",
				      DIALOG_YES )) {
	case DIALOG_YES:
	    menu_save( interface );
	    if (interface->modified)
		break;

	case DIALOG_CANCEL:
	    return;
	}

    /* Suppression du texte */
    gtk_editable_delete_text( GTK_EDITABLE( interface->text ), 0, -1 );
    interface->modified = FALSE;
    if (interface->filename) {
	free( interface->filename );
	interface->filename = NULL;
    }
    set_main_title( GTK_WINDOW( interface->window ), NULL );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Fichier/Ouvrir.
 */
static void menu_open( interface_t interface )
{
    /* Variables locales */
    char *filename; /* Nom du fichier            */

    /* Contrôle des paramètres */
    assert( interface );

    /* Ouverture du fichier */
    if ((!interface->modified ||
	 dialog_yes_no( "Le texte a été modifié.\n"
			"Souhaitez-vous vraiment le remplacer ?", DIALOG_NO )
	 == DIALOG_YES) &&
	(filename = dialog_file( "Ouverture d'un fichier texte",
				 "*.txt", FALSE ))) {
	if (text_load( filename, GTK_TEXT( interface->text ) )) {
	    interface->modified = FALSE;
	    if (interface->filename)
		free( interface->filename );
	    interface->filename = filename;
	    set_main_title( GTK_WINDOW( interface->window ), filename );
	} else
	    free( filename );
    }
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Fichier/Enregistrer.
 */
static void menu_save( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Sauvegarde du fichier */
    if (interface->filename) {
	if (text_save( interface->filename, GTK_TEXT( interface->text ) ))
	    interface->modified = FALSE;
    } else
	menu_save_as( interface );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Fichier/Enregistrer sous.
 */
static void menu_save_as( interface_t interface )
{
    /* Variables locales */
    char *filename; /* Nom du fichier */

    /* Contrôle des paramètres */
    assert( interface );

    /* Sauvegarde du fichier */
    if ((filename = dialog_file( "Ouverture d'un fichier texte",
				 "*.txt", TRUE ))) {
	if (text_save( filename, GTK_TEXT( interface->text ) )) {
	    interface->modified = FALSE;
	    if (interface->filename)
		free( interface->filename );
	    interface->filename = filename;
	    set_main_title( GTK_WINDOW( interface->window ), filename );
	} else
	    free( filename );
    }
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Fichier/Quitter.
 */
static void menu_quit( interface_t interface )
{
    /* Ferme la fenêtre */
    if (!delete_window( GTK_WINDOW( interface->window ), NULL, interface ))
	gtk_widget_destroy( interface->window );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Édition/Couper.
 */
static void menu_cut( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Coupe */
    gtk_editable_cut_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Édition/Copier.
 */
static void menu_copy( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Copie */
    gtk_editable_copy_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Édition/Coller.
 */
static void menu_paste( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Colle */
    gtk_editable_paste_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Édition/Sélectionner tout.
 */
static void menu_select_all( interface_t interface )
{
    /* Contrôle des paramètres */
    assert( interface );

    /* Sélectionne tout */
    gtk_editable_select_region( GTK_EDITABLE( interface->text ), 0, -1 );
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Effacer.
 */
static void menu_clear_dict( interface_t interface )
{
    /* Variables locales */
    bool_t modified;  /* Sauvegarde de l'état modifié */

    /* Contrôle des paramètres */
    assert( interface );

    /* Détruit puis recrée le dictionnaire */
    dict_delete( interface->dict );
    if (!(interface->dict = dict_new())) {
	dialog_alert( "Erreur : impossible de créer un dictionnaire." );
	menu_quit( interface );
    }

    /* Mise à jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Ouvrir.
 */
static void menu_open_dict( interface_t interface )
{
    /* Variables locales */
    char   *filename; /* Nom du fichier               */
    char   *buffer;   /* Tampon de lecture            */
    dict_t dict;      /* Nouveau dictionnaire         */
    bool_t modified;  /* Sauvegarde de l'état modifié */

    /* Réinitialise et ouvre le dictionnaire */
    if ((filename = dialog_file( "Ouvrir un dictionnaire", "*.hdc",
				 FALSE ))) {
	if (huffman_read( filename, &buffer, NULL )) {
	    if ((dict = dict_new()) &&
		dict_add_words_from_string( dict, buffer )) {
		dict_delete( interface->dict );
		interface->dict = dict;
	    } else
		dialog_alert( "Erreur d'ouverture du dictionnaire." );
	    free( buffer );
	} else
	    dialog_alert( "Le format du fichier est incorrect." );
	free( filename );
    }

    /* Mise à jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Ajouter.
 */
static void menu_add_dict( interface_t interface )
{
    /* Variables locales */
    char *filename;   /* Nom du fichier               */
    char *buffer;     /* Tampon de lecture            */
    bool_t modified;  /* Sauvegarde de l'état modifié */

    /* Ouvre le dictionnaire */
    if ((filename = dialog_file( "Ajouter un dictionnaire", "*.hdc",
				 FALSE ))) {
	if (huffman_read( filename, &buffer, NULL )) {
	    if (!dict_add_words_from_string( interface->dict, buffer ))
		dialog_alert( "Erreur d'ouverture du dictionnaire." );
	    free( buffer );
	} else
	    dialog_alert( "Le format du fichier est incorrect." );
	free( filename );
    }

    /* Mise à jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Enregistrer.
 */
static void menu_save_dict( interface_t interface )
{
    /* Variables locales */
    char   *filename; /* Nom du fichier               */
    char   *buffer;   /* Tampon de d'écriture         */
    bool_t modified;  /* Sauvegarde de l'état modifié */

    /* Sauvegarde le dictionnaire */
    if ((filename = dialog_file( "Enregistrer un dictionnaire", "*.hdc",
				 TRUE ))) {
	if ((buffer = dict_get_words_into_string( interface->dict ))) {
	    if (huffman_write( filename, buffer, (unsigned int) -1 ))
		free( buffer );
	    else
		dialog_alert( "Erreur d'écriture du dictionnaire." );
	} else
	    dialog_alert( "Erreur de recherche des mots du dictionnaire." );
	free( filename );
    }

    /* Mise à jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appelé lorsque l'utilisateur clique sur le menu Aide/À propos.
 */
static void menu_about( interface_t interface UNUSED )
{
    /* Variables locales */
    static const char *const buttons[] = {"OK", NULL }; /* Bouton OK */

    /* Affiche la boîte de dialogue */
    dialog_box( "À propos",
		"Act : Auto-Completion Tree\n"
		"Implémentation d'un arbre d'auto-complétion\n\n"
		"Projet de Pratique de la Programmation\n"
		"Université Louis Pasteur - IUP GMI\n\n"
		"Copyright (c) 2004 Benjamin Gaillard.\n"
		"Tous droits réservés.\n\n"
		"Ce programme est soumis à la GNU General Public Licence.\n"
		"Lisez le fichier LICENSE pour plus d'informations.",
		buttons, 0 );
}

#endif /* USE_GTK1 */

/* Fin du fichier */
