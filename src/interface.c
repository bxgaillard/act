/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Impl�mentation d'un arbre d'auto-compl�tion
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
 *               devrait pas �tre trop compliqu�e.
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
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef USE_GTK1
#include <gtk/gtk.h>

/* En-t�tes locaux */
#include "interface.h"
#include "dict.h"
#include "huffman.h"
#include "alpha.h"


/*****************************************************************************
 *
 * CONSTANTES
 *
 */


/* Nombre maximum de propositions � chercher dans le dictionnaire */
#define NUM_WORDS 10

/* Valeurs identifiant des boutons dans les bo�tes de dialogue */
#define DIALOG_YES    0
#define DIALOG_NO     1
#define DIALOG_CANCEL 2


/*****************************************************************************
 *
 * D�FINITIONS ET MACROS
 *
 */

/* GTK+ 2.x n'est pas support� */
#ifdef USE_GTK2
#error "GTK+ 2.x n'est pas support�."
#endif /* USE_GTK2 */

/* Pour �viter des avertissements de comparaison d'entiers sign�s et non
 * sign�s lors de la compilation */
#ifdef USE_GTK1
#undef GTK_TEXT_INDEX
#define GTK_TEXT_INDEX( t, index )                                        \
    (((t)->use_wchar)                                                     \
	? ((index) < (t)->gap_position ? (t)->text.wc[index] :            \
	   (t)->text.wc[(index) + (t)->gap_size])                         \
	: ((index) < (t)->gap_position ? (unsigned) (t)->text.ch[index] : \
	   (unsigned) (t)->text.ch[(index) + (t)->gap_size]))
#endif /* USE_GTK1 */

/* D�finition utilis�e pour supprimer les avertissements de param�tres
 * inutilis�s lors de la compilation */
#ifdef __GNUC__
#define UNUSED __attribute__ ((__unused__))
#else /* __GNUC__ */
#define UNUSED
#endif /* !__GNUC__ */


/*****************************************************************************
 *
 * TYPES DE DONN�ES
 *
 */

/* Variables communes utilis�es pour l'interface, partag�es entre plusieurs
 * fonctions */
typedef struct interface
{
    bool_t       modified;  /* Si le texte a �t� modifi� */
    char         *filename; /* Nom du fichier            */
    dict_t       dict;      /* Dictionnaire              */
    unsigned int length;    /* Longueur du d�but de mot  */
    char         **used;    /* Meilleures propositions   */
    unsigned int selected;  /* Mot s�lectionn�           */
    GtkWidget    *window;   /* Fen�tre principale        */
    GtkWidget    *text;     /* Texte �dit�               */
    GtkWidget    *list;     /* Liste de propositions     */
    GtkWidget    *button;   /* Bouton d'insertion        */
}
interface_s_t;


/*****************************************************************************
 *
 * PROTOTYPES DES FONCTIONS STATIQUES
 *
 */

/* Fonctions d'affichage de bo�tes de dialogue */
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

/* Changement de titre de la fen�tre */
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

/* Variables utilis�es pour les bo�tes de dialogue */
static unsigned int dialog_selected;  /* Bouton s�lectionn� */
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
    {"/_�dition",                          NULL,         NULL,              0,
     "<Branch>"},
    {"/�dition/_Couper",                   "<control>X", menu_cut,          0,
     NULL},
    {"/�dition/Co_pier",                   "<control>C", menu_copy,         0,
     NULL},
    {"/�dition/Co_ller",                   "<control>V", menu_paste,        0,
     NULL},
    {"/�dition/-",                         NULL,         NULL,              0,
     "<Separator>"},
    {"/�dition/_Tout s�lectionner",        "<control>Z", menu_select_all,   0,
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
    {"/Aide/_� propos...",                 "<control>Y", menu_about,        0,
     NULL}
};


/*****************************************************************************
 *
 * FONCTIONS EXTERNES
 *
 */

/**
 * Cr�e un nouvel objet interface.
 */
interface_t interface_new( int argc, char **argv )
{
    /* Variables locales */
    GtkWidget      *vbox;      /* Bo�te principale            */
    GtkWidget      *menubar;   /* Barre de menu               */
    GtkWidget      *hpaned;    /* Bo�te horizontale           */
    GtkWidget      *text_hbox; /* Texte                       */
    GtkWidget      *scroll;    /* Ascenseur                   */
    GtkWidget      *list_vbox; /* Bo�te pour la partie droite */
    GtkWidget      *frame;     /* Cadre pour la liste         */
    GtkWidget      *sep;       /* S�parateur pour la liste    */
    GtkItemFactory *factory;   /* Fabrique de menus           */
    GtkAccelGroup  *accel;     /* Groupe de raccourcis        */
    interface_t    interface;  /* Objet de l'interface        */

    /* Cr�e les objets */
    if (!(interface = malloc( sizeof (interface_s_t) )) ||
	!(interface->dict = dict_new()))
	return NULL;

    /* Initialisation de GTK+ */
    gtk_init( &argc, &argv );

    /* Fen�tre */
    interface->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    set_main_title( GTK_WINDOW( interface->window ), NULL );
    gtk_window_set_default_size( GTK_WINDOW( interface->window ), 400, 300 );
    gtk_signal_connect( GTK_OBJECT( interface->window ), "delete-event",
			GTK_SIGNAL_FUNC( delete_window ), interface );
    gtk_signal_connect( GTK_OBJECT( interface->window ), "destroy",
			GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

    /* Bo�te pour le menu et le reste en-dessous */
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

    /* Bo�te avec s�parateur pour le texte et la liste */
    hpaned = gtk_hpaned_new();
    gtk_paned_set_gutter_size( GTK_PANED( hpaned ), 16 );
    gtk_box_pack_start( GTK_BOX( vbox ), hpaned, TRUE, TRUE, 0 );

    /* Bo�te horizontale pour le texte et l'ascenseur */
    text_hbox = gtk_hbox_new( FALSE, 0 );
    gtk_paned_pack1( GTK_PANED( hpaned ), text_hbox, TRUE, FALSE );

    /* Bo�te de texte */
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

    /* Bo�te verticale pour la liste et le bouton d'insertion */
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

    /* S�parateur */
    sep = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX( list_vbox ), sep, FALSE, FALSE, 0 );

    /* Bouton d'insertion */
    interface->button = gtk_button_new_with_label( "Ins�rer" );
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

    /* Retour de l'objet cr�� */
    return interface;
}

/**
 * D�truit un objet interface.
 */
void interface_delete( interface_t interface )
{
    /* Contr�le des param�tres */
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
    /* Contr�le des param�tres */
    assert( interface );

    /* Affichage de la fen�tre */
    gtk_widget_show( interface->window );

    /* Boucle principale de l'interface graphique */
    gtk_main();

    /* Pas d'erreur */
    return TRUE;
}


/*****************************************************************************
 *
 * FONCTIONS D'AFFICHAGE DE BO�TES DE DIALOGUE
 *
 */

/**
 * Callback servant � enregistrer le bouton cliqu�.
 */
static void dialog_callback( GtkButton *button UNUSED, unsigned int selected )
{
    if (dialog_selected == (unsigned int) -1)
	dialog_selected = selected;
}

/**
 * Affichage d'une bo�te de dialogue g�n�rique.
 */
static int dialog_box( const char *title, const char *message,
		       const char *const *buttons,
		       unsigned int default_button )
{
    /* Variables locales */
    unsigned int i;
    GtkWidget    *dialog, *label, *button_box, *button; /* Widgets */

    /* Contr�le des param�tres */
    assert( title );
    assert( message );
    assert( buttons );

    /* Bo�te de dialogue */
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

    /* Bo�te pour les boutons */
    button_box = gtk_hbox_new( TRUE, 10 );
    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( dialog )->action_area ),
		       button_box );

    /* Boutons */
    i = 0;
    while (*buttons) {
	/* Cr�ation du bouton */
	button = gtk_button_new_with_label( *buttons );
	gtk_box_pack_start( GTK_BOX( button_box ), button, TRUE, TRUE, 0 );
	GTK_WIDGET_SET_FLAGS( button, GTK_CAN_DEFAULT );
	gtk_signal_connect( GTK_OBJECT( button ), "clicked",
			    GTK_SIGNAL_FUNC( dialog_callback ),
			    (gpointer) i );
	gtk_signal_connect_object( GTK_OBJECT( button ), "clicked",
				   GTK_SIGNAL_FUNC( gtk_widget_destroy ),
				   GTK_OBJECT( dialog ) );

	/* Bouton par d�faut */
	if (i == default_button) {
	    gtk_widget_grab_default( button );
	    gtk_widget_grab_focus( button );
	}

	buttons++;
	i++;
    }

    /* Affiche la bo�te de dialogue et attend la validation */
    gtk_widget_show_all( dialog );
    dialog_selected = (unsigned int) -1;
    while (dialog_selected == (unsigned int) -1)
	gtk_main_iteration();

    /* Retour du r�sultat */
    return dialog_selected;
}

/**
 * Affichage d'un avertissement dans une bo�te de dialogue
 */
static void dialog_alert( const char *message )
{
    /* Variables locales */
    static const char *const buttons[] = {"OK", NULL}; /* Boutons */

    /* Contr�le des param�tres */
    assert( message );

    /* Affichage de la bo�te de dialogue */
    dialog_box( "Alerte", message, buttons, 0 );
}

/**
 * Affichage d'une bo�te de dialogue demandant � l'utilisateur de choisir
 * entre "Oui" et "Non".
 */
static unsigned int dialog_yes_no( const char *message,
				   unsigned int default_button )
{
    /* Variables locales */
    static const char *const buttons[] = {"Oui", "Non", NULL}; /* Boutons */

    /* Contr�le des param�tres */
    assert( message );

    /* Affichage de la bo�te de dialogue */
    return dialog_box( "Question", message, buttons, default_button );
}

/**
 * Affichage d'une bo�te de dialogue demandant � l'utilisateur de choisir
 * entre "Oui", "Non" et "Annuler".
 */
static unsigned int dialog_yes_no_cancel( const char *message,
					  unsigned int default_button )
{
    /* Variables locales */
    static const char *const buttons[] = /* Boutons */
	{"Oui", "Non", "Annuler", NULL};

    /* Contr�le des param�tres */
    assert( message );

    /* Affichage de la bo�te de dialogue */
    return dialog_box( "Question", message, buttons, default_button );
}

/**
 * Callback utilis� pour enregistrer le nom de fichier s�lectionn�.
 */
static void dialog_file_callback( GtkButton *button UNUSED,
				  GtkFileSelection *selector)
{
    /* Variables locales */
    char *filename; /* Nom du fichier */

    if (dialog_selected == 0) {
	if (selector) {
	    /* Bouton OK : copie du nom de fichier dans un tampon allou� */
	    filename = gtk_file_selection_get_filename( selector );
	    if ((dialog_filename = malloc( strlen( filename ) + 1 )))
		strcpy( dialog_filename, filename );
	    dialog_selected = 1;
	} else {
	    /* Bouton Annuler ou fermeture de la fen�tre */
	    dialog_filename = NULL;
	    dialog_selected = 1;
	}
    }
}

/**
 * Affichage d'une bo�te de dialogue permettant de s�lectionner un fichier.
 */
static char *dialog_file( const char *title, const char *mask, bool_t save )
{
    /* Variables locales */
    GtkWidget   *selector; /* S�lecteur de fichier        */
    struct stat stats;     /* Informations sur le fichier */

    /* Contr�le des param�tres */
    assert( title );
    assert( mask );

    /* Cr�ation de la bo�te de dialogue */
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

    /* Affiche la bo�te de dialogue et attend la saisie */
    gtk_widget_show( selector );
    dialog_selected = 0;
    while (dialog_selected == 0)
	gtk_main_iteration();

    /* V�rifie l'existence du fichier */
    if (save && stat( dialog_filename, &stats ) == 0 &&
	dialog_yes_no( "Le fichier existe.\n"
		       "Souhaitez-vous l'�craser ?", DIALOG_NO )
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
 * Charge un fichier dans la bo�te de texte.
 */
static bool_t text_load( const char *filename, GtkText *text )
{
    /* Variables locales */
    int          fd;           /* Descripteur de fichier  */
    int          position;     /* Position dans le texte  */
    unsigned int size;         /* Taille des donn�es lues */
    char         buffer[1024]; /* Tampon de lecture       */

    /* Contr�le des param�tres */
    assert( filename );
    assert( text );

    /* Ouvre le fichier */
    if ((fd = open( filename, O_RDONLY )) == -1) {
	dialog_alert( "Le fichier n'a pas pu �tre ouvert." );
	return FALSE;
    }

    /* Supprime tout le texte pr�sent */
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

    /* Ferme le fichier et place le curseur au d�but */
    close( fd );
    gtk_editable_set_position( GTK_EDITABLE( text ), 0 );
    gtk_text_thaw( text );
    return TRUE;
}

/**
 * Enregistre un fichier depuis la bo�te de texte.
 */
static bool_t text_save( const char *filename, GtkText *text )
{
    /* Variables locales */
    int  fd;      /* Descripteur de fichier     */
    int  size;    /* Taille des donn�es �crites */
    char *buffer; /* Tampon d'�criture          */

    /* Contr�le des param�tres */
    assert( filename );
    assert( text );

    /* Cr�e fichier */
    if ((fd = creat( filename, 0666 )) == -1) {
	dialog_alert( "Le fichier n'a pas pu �tre �crit." );
	return FALSE;
    }

    /* �crit le fichier */
    buffer = gtk_editable_get_chars( GTK_EDITABLE( text ), 0, -1 );
    size = write( fd, buffer, gtk_text_get_length( text ) );

    /* Lib�re la m�moire et ferme le fichier */
    g_free( buffer );
    close( fd );

    /* Contr�le d'erreur */
    if (size == -1) {
	dialog_alert( "Erreur d'�criture du fichier." );
	return FALSE;
    }
    return TRUE;
}


/*****************************************************************************
 *
 * CHANGEMENT DE TITRE DE LA FEN�TRE
 *
 */

/**
 * Change le titre de la fen�tre en fonction du nom de fichier �dit�.
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

    /* Contr�le des param�tres */
    assert( window );

    /* D�termine le titre */
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

    /* Lib�re la m�moire */
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
    unsigned int start;    /* D�but du mot               */
    char         chr;      /* Caract�re courant          */
    char         *word;    /* Mot trouv�                 */
    char         **result; /* R�sultat : tableau de mots */

    /* Contr�le des param�tres */
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

    /* Calcul de la longueur du d�but de mot */
    interface->length = end - start;
    if (start == end)
	return NULL;

    /* Cherche tous les mots possibles et retourne le r�sultat */
    word = gtk_editable_get_chars( GTK_EDITABLE( text ), start, end );
    result = dict_get_most_used( interface->dict, word, NUM_WORDS );
    g_free( word );
    return result;
}

/**
 * Callback appel� lorsque le texte est modifi�.
 */
static void text_changed( GtkText *text, interface_t interface )
{
    /* Variables locales */
    unsigned int i;            /* Compteur               */
    GtkWidget    *item;        /* Un item de la liste    */
    GList        *list = NULL; /* Nouvelle liste de mots */

    /* Contr�le des param�tres */
    assert( text );
    assert( interface );
    assert( interface->list );

    /* Efface la liste */
    gtk_list_clear_items( GTK_LIST( interface->list ), 0, -1 );

    /* Cherche les mots dans le dictionnaire */
    if (interface->used)
	free( interface->used );
    interface->used = find_words( text, interface );

    /* Rajoute chaque mot trouv� dans la liste */
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

    /* Le texte est modifi� */
    interface->modified = TRUE;
}

/**
 * Callback appel� quand tu texte est ins�r�.
 */
static void text_inserted( GtkText *text, gchar *new_text UNUSED,
			   gint new_text_length, gint *position,
			   interface_t interface )
{
    /* Variables locales */
    unsigned int pos;   /* Position courange      */
    unsigned int end;   /* Position de fin du mot */
    char         chr;   /* Caract�re courant      */
    char         *word; /* Mot � ajouter          */

    /* Contr�le des param�tres */
    assert( text );
    assert( position );
    assert( interface );
    assert( interface->text );

    /* Si rien n'est ajout�, ne rien faire */
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
 * Callback appel� lorsque l'utilisateur s�lectionne un mot dans la liste.
 */
static void select_word( GtkList *list, GtkWidget *widget,
			 interface_t interface )
{
    /* Enregistre la s�lection */
    interface->selected = gtk_list_child_position( list, widget );
}

/**
 * Callback appel� quand l'utilisateur clique sur le bouton d'insertion.
 */
static void insert_word( GtkButton *button UNUSED, interface_t interface )
{
    /* Variables locales */
    const char *word;    /* Mot � ins�rer          */
    int        position; /* Position dans le texte */

    /* Ins�re le mot */
    word = interface->used[interface->selected];
    position = gtk_editable_get_position( GTK_EDITABLE( interface->text ) );
    gtk_editable_insert_text( GTK_EDITABLE( interface->text ),
			      word + interface->length,
			      strlen( word ) - interface->length, &position );

    /* Redonne le focus � la bo�te de texte */
    gtk_widget_grab_focus( interface->text );
}

/**
 * Callback appel� lors d'une tentative de fermeture de la fen�tre.
 */
static gboolean delete_window( GtkWindow *window UNUSED,
			       GdkEvent *event UNUSED, interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Sauvegarde du fichier si n�cessaire */
    if (interface->modified) {
	switch (dialog_yes_no_cancel( "Le fichier a �t� modifi�.\n"
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

    /* Lib�ration de la m�moire */
    if (interface->filename) {
	free( interface->filename );
	interface->filename = NULL;
    }
    if (interface->used) {
	free( interface->used );
	interface->used = NULL;
    }

    /* Destruction de la fen�tre */
    return FALSE;
}


/*****************************************************************************
 *
 * CALLBACKS POUR LES MENUS
 *
 */

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu Fichier/Nouveau.
 */
static void menu_new( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Sauvegarde du texte si demand� */
    if (interface->modified)
	switch (dialog_yes_no_cancel( "Le texte a �t� modifi�.\n"
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
 * Callback appel� lorsque l'utilisateur clique sur le menu Fichier/Ouvrir.
 */
static void menu_open( interface_t interface )
{
    /* Variables locales */
    char *filename; /* Nom du fichier            */

    /* Contr�le des param�tres */
    assert( interface );

    /* Ouverture du fichier */
    if ((!interface->modified ||
	 dialog_yes_no( "Le texte a �t� modifi�.\n"
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
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Fichier/Enregistrer.
 */
static void menu_save( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Sauvegarde du fichier */
    if (interface->filename) {
	if (text_save( interface->filename, GTK_TEXT( interface->text ) ))
	    interface->modified = FALSE;
    } else
	menu_save_as( interface );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Fichier/Enregistrer sous.
 */
static void menu_save_as( interface_t interface )
{
    /* Variables locales */
    char *filename; /* Nom du fichier */

    /* Contr�le des param�tres */
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
 * Callback appel� lorsque l'utilisateur clique sur le menu Fichier/Quitter.
 */
static void menu_quit( interface_t interface )
{
    /* Ferme la fen�tre */
    if (!delete_window( GTK_WINDOW( interface->window ), NULL, interface ))
	gtk_widget_destroy( interface->window );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu �dition/Couper.
 */
static void menu_cut( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Coupe */
    gtk_editable_cut_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu �dition/Copier.
 */
static void menu_copy( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Copie */
    gtk_editable_copy_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu �dition/Coller.
 */
static void menu_paste( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* Colle */
    gtk_editable_paste_clipboard( GTK_EDITABLE( interface->text ) );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * �dition/S�lectionner tout.
 */
static void menu_select_all( interface_t interface )
{
    /* Contr�le des param�tres */
    assert( interface );

    /* S�lectionne tout */
    gtk_editable_select_region( GTK_EDITABLE( interface->text ), 0, -1 );
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Effacer.
 */
static void menu_clear_dict( interface_t interface )
{
    /* Variables locales */
    bool_t modified;  /* Sauvegarde de l'�tat modifi� */

    /* Contr�le des param�tres */
    assert( interface );

    /* D�truit puis recr�e le dictionnaire */
    dict_delete( interface->dict );
    if (!(interface->dict = dict_new())) {
	dialog_alert( "Erreur : impossible de cr�er un dictionnaire." );
	menu_quit( interface );
    }

    /* Mise � jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Ouvrir.
 */
static void menu_open_dict( interface_t interface )
{
    /* Variables locales */
    char   *filename; /* Nom du fichier               */
    char   *buffer;   /* Tampon de lecture            */
    dict_t dict;      /* Nouveau dictionnaire         */
    bool_t modified;  /* Sauvegarde de l'�tat modifi� */

    /* R�initialise et ouvre le dictionnaire */
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

    /* Mise � jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Ajouter.
 */
static void menu_add_dict( interface_t interface )
{
    /* Variables locales */
    char *filename;   /* Nom du fichier               */
    char *buffer;     /* Tampon de lecture            */
    bool_t modified;  /* Sauvegarde de l'�tat modifi� */

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

    /* Mise � jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu
 * Dictionnaire/Enregistrer.
 */
static void menu_save_dict( interface_t interface )
{
    /* Variables locales */
    char   *filename; /* Nom du fichier               */
    char   *buffer;   /* Tampon de d'�criture         */
    bool_t modified;  /* Sauvegarde de l'�tat modifi� */

    /* Sauvegarde le dictionnaire */
    if ((filename = dialog_file( "Enregistrer un dictionnaire", "*.hdc",
				 TRUE ))) {
	if ((buffer = dict_get_words_into_string( interface->dict ))) {
	    if (huffman_write( filename, buffer, (unsigned int) -1 ))
		free( buffer );
	    else
		dialog_alert( "Erreur d'�criture du dictionnaire." );
	} else
	    dialog_alert( "Erreur de recherche des mots du dictionnaire." );
	free( filename );
    }

    /* Mise � jour de la liste */
    modified = interface->modified;
    text_changed( GTK_TEXT( interface->text ), interface );
    interface->modified = modified;
}

/**
 * Callback appel� lorsque l'utilisateur clique sur le menu Aide/� propos.
 */
static void menu_about( interface_t interface UNUSED )
{
    /* Variables locales */
    static const char *const buttons[] = {"OK", NULL }; /* Bouton OK */

    /* Affiche la bo�te de dialogue */
    dialog_box( "� propos",
		"Act : Auto-Completion Tree\n"
		"Impl�mentation d'un arbre d'auto-compl�tion\n\n"
		"Projet de Pratique de la Programmation\n"
		"Universit� Louis Pasteur - IUP GMI\n\n"
		"Copyright (c) 2004 Benjamin Gaillard.\n"
		"Tous droits r�serv�s.\n\n"
		"Ce programme est soumis � la GNU General Public Licence.\n"
		"Lisez le fichier LICENSE pour plus d'informations.",
		buttons, 0 );
}

#endif /* USE_GTK1 */

/* Fin du fichier */
