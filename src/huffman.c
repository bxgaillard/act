/*
 * ---------------------------------------------------------------------------
 *
 * Act : Auto-Completion Tree -- Implémentation d'un arbre d'auto-complétion
 * Copyright (c) 2004 Benjamin Gaillard
 *
 * ---------------------------------------------------------------------------
 *
 * Fichier     : huffman.c
 *
 * Description : Deux fonctions permettant la lecture et l'écriture de
 *               fichiers compressés avec l'algorithme de Huffman.
 *
 * Commentaire : La fonction de compression (écriture) utilise trois fonctions
 *               statiques permettant de gérer une queue de priorité de façon
 *               optimisée.
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
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

/* En-têtes locaux */
#include "bool.h"
#include "huffman.h"


/*****************************************************************************
 *
 * CONSTANTES
 *
 */

/* Nombre maximum de caractères différents */
#define NUM_CHARS (1 << 8) /* 256 */

/* Nombre maximum de noeuds dans l'arbre de Huffman */
#define NUM_NODES (2 * NUM_CHARS - 1) /* 511 */

/* Taille du tampon pour les opérations de lecture et écriture de fichier */
#define BUFFER_SIZE 1024 /* 1 Ko */

/* En-tête du fichier compressé */
#define HEADER0 'H'
#define HEADER1 'U'
#define HEADER2 'F'
#define HEADER3 'F'

/* Supprime le flag de grand fichier s'il n'est pas supporté */
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif /* defined(O_LARGEFILE) */


/*****************************************************************************
 *
 * MACROS
 *
 */

/* Macro permettant d'obtenir simplement la taille de la queue */
#define PQ_SIZE( pq ) (*((unsigned int *) pq))

/* Les macros suivantes ne fonctionnent que dans huffman_read() */

/* Macro permettant d'assrer la présence d'un bit dans le tampon */
#define GET_BIT if (rbuffer.bb_remain == 0 &&         \
		    !rbuffer_read_byte( &rbuffer )) { \
		    free( *buffer );                  \
		    return FALSE;                     \
		}

/* Macro permettant d'accéder au premier bit du tampon */
#define THE_BIT (rbuffer.byte_buffer & 1)

/* Macro déchargeant le premier bit du tampon */
#define GOT_BIT { rbuffer.byte_buffer >>= 1; rbuffer.bb_remain--; }

/* Macro permettant d'assurer la présence de `number' bits dans le tampon */
#define GET_BITS( number ) if (rbuffer.bb_remain < (number) &&   \
			       !rbuffer_read_byte( &rbuffer )) { \
			       free( *buffer );                  \
			       return FALSE;                     \
			   }

/* Macro permettant d'accéder aux `number' premiers bits du tampon */
#define THE_BITS( number ) (rbuffer.byte_buffer & ((1 << (number)) - 1))

/* Macro déchargeant les `number' premiers bits du tampon */
#define GOT_BITS( number ) { rbuffer.byte_buffer >>= (number); \
			     rbuffer.bb_remain -= (number); }


/*****************************************************************************
 *
 * TYPES DE DONNÉES
 *
 */

/* Structure contenant l'arbre de Huffman utilisé pour la compression
 * parent > 0 : fils de gauche (bit 0)
 * parent < 0 : fils de droite (bit 1) */
typedef struct huffman_cnode
{
    unsigned int freq;   /* Fréquence du caractère */
    signed   int parent; /* Noeud parent           */
}
huffman_cnode_s_t, *huffman_cnode_t, huffman_ctree_t[NUM_NODES];

/* Structure contenant l'arbre de Huffman utilisé pour la décompression
 * fils <= 0 : caractère
 * fils >  0 : noeud */
typedef struct huffman_dnode
{
    signed short children[2]; /* Fils */
}
huffman_dnode_s_t, *huffman_dnode_t, huffman_dtree_t[NUM_NODES + 1];

/* Tableau des codes associés aux caractères */
typedef struct huffman_code
{
    unsigned char size;                /* Taille d'un code en bits  */
    unsigned char bits[NUM_CHARS / 8]; /* Code, de maximum 256 bits */
}
huffman_code_s_t, *huffman_code_t, huffman_codes_t[NUM_CHARS];

/* Type de donnée pour la queue de priorité */
typedef huffman_cnode_t pq_t[NUM_NODES + 1];

/* Tampon de fichier pour la lecture */
typedef struct rbuffer
{
    int           fd;                       /* Descripteur de fichier     */
    unsigned int  size;                     /* Taille des données         */
    unsigned int  bb_remain;                /* Bits restants dans l'octet */
    unsigned int  byte_buffer;              /* Tampon d'octets            */
    unsigned int  fb_pos;                   /* Position dans le tampon    */
    unsigned int  fb_size;                  /* Taille des données lues    */
    unsigned char file_buffer[BUFFER_SIZE]; /* Tampon de fichier          */
}
rbuffer_s_t, *rbuffer_t;

/* Tampon de fichier pour l'écriture */
typedef struct wbuffer
{
    int           fd;                       /* Descripteur de fichier      */
    unsigned int  bb_size;                  /* Taille du tampon d'octets   */
    unsigned int  byte_buffer;              /* Tampon de d'octets          */
    unsigned int  fb_size;                  /* Taille du tampon de fichier */
    unsigned char file_buffer[BUFFER_SIZE]; /* Tampon de fichier           */
}
wbuffer_s_t, *wbuffer_t;


/*****************************************************************************
 *
 * PROTOTYPES DES FONCTIONS STATIQUES
 *
 */

/* Gestion de la queue de priorité */
static void            pq_init( pq_t pq );
static void            pq_push( pq_t pq, const huffman_cnode_t elem );
static huffman_cnode_t pq_pop( pq_t pq );

/* Gestion du tampon de lecture */
static bool_t rbuffer_read_byte( rbuffer_t buffer );
static bool_t rbuffer_init( rbuffer_t buffer, const char *filename );

/* Gestion du tampon d'écriture */
static bool_t wbuffer_write( wbuffer_t buffer );
static bool_t wbuffer_flush( wbuffer_t buffer );
static bool_t wbuffer_finish( wbuffer_t buffer );
static bool_t wbuffer_write_bits( wbuffer_t buffer, unsigned int data,
				  unsigned int size );
static bool_t wbuffer_write_code( wbuffer_t buffer, huffman_code_t code );
static bool_t wbuffer_init( wbuffer_t buffer, const char *filename,
			    unsigned int size );


/*****************************************************************************
 *
 * FONCTIONS EXTERNES
 *
 */

/**
 * Lit un fichier et décompresse les données qui s'y trouvent selon
 * l'algorithme de Huffman.
 */
bool_t huffman_read( const char *filename, char **buffer, unsigned int *size )
{
    /* Variables locales */
    unsigned int    i, j;      /* Compteurs                     */
    unsigned int    code_size; /* Taille du code courant        */
    int             pos;       /* Position dans l'arbre         */
    huffman_dtree_t tree;      /* Arbre de Huffman              */
    unsigned int    count;     /* Nombre de noeuds dans l'arbre */
    rbuffer_s_t     rbuffer;   /* Tampon de lecture             */

    /* Contrôle des paramètres */
    assert( filename || (!buffer && !size) );

    /* Initialise le tampon */
    if (!rbuffer_init( &rbuffer, filename ))
	return FALSE;

    if (size)
	*size = rbuffer.size;

    /* Si on ne veut pas des données, retourne */
    if (rbuffer.size == 0)
	return TRUE;
    if (!buffer) {
	close( rbuffer.fd );
	return TRUE;
    }

    /* Alloue la mémoire nécessaire pour les données */
    if (!(*buffer = malloc( rbuffer.size ))) {
	/* Il n'y a pas assez de mémoire disponible */
	close( rbuffer.fd );
	return FALSE;
    }

    /* Initialisation de l'arbre */
    tree[1].children[0] = 1;
    tree[1].children[1] = 1;
    count = 2;

    /* Construction de l'arbre */
    for (i = 0; i < NUM_CHARS; i++) {
	/* Obtient la longueur du code */
	GET_BITS( 8 );
	code_size = THE_BITS( 8 );
	GOT_BITS( 8 );

	/* Si le caractère a un code */
	if (code_size != 0) {
	    /* Calcule la position dans l'arbre */
	    pos = 1;
	    for (j = 1; j < code_size; j++) {
		GET_BIT;
		if (tree[pos].children[THE_BIT] <= 1) {
		    /* Crée un noeud */
		    tree[pos].children[THE_BIT] = count;
		    if ((pos = count++) == NUM_NODES + 1) {
			close( rbuffer.fd );
			free( *buffer );
			return FALSE;
		    }
		    tree[pos].children[0] = 1;
		    tree[pos].children[1] = 1;
		} else
		    pos = tree[pos].children[THE_BIT];
		GOT_BIT;
	    }

	    /* Affectation du caractère à la position courante */
	    GET_BIT;
	    tree[pos].children[THE_BIT] = -i;
	    GOT_BIT;
	}
    }

    /* Lecture des données */
    for (i = 0; i < rbuffer.size; i++) {
	/* Initialise la position à la racine de l'arbre */
	pos = 1;

	/* Calcule la position dans l'arbre */
	do {
	    GET_BIT;
	    pos = tree[pos].children[THE_BIT];
	    GOT_BIT;

	    /* Retourne si le fichier est erroné */
	    if (pos == 1) {
		close( rbuffer.fd );
		free( *buffer );
		return FALSE;
	    }
	} while (pos > 0);

	/* Stockage du caractère trouvé dans le tampon */
	(*buffer)[i] = -pos;
    }

    /* Ferme le fichier */
    close( rbuffer.fd );

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Compresse des données selon l'algorithme de Huffman et les écrit dans un
 * fichier.
 */
bool_t huffman_write( const char *filename, const char *buffer,
		      unsigned int size )
{
    /* Variables locales */
    unsigned int    i;                /* Compteur                         */
    unsigned int    count;            /* Nombre de noeuds de l'arbre      */
    unsigned int    byte;             /* Indice de l'octet dans le code   */
    signed int      cur;              /* Index du noeud courant           */
    huffman_cnode_t children[2];      /* Fils pour la création de l'arbre */
    huffman_ctree_t tree;             /* Arbre de Huffman                 */
    huffman_codes_t codes;            /* Codes pour l'arbre de Huffman    */
    pq_t            pq;               /* Queue de priorité                */
    wbuffer_s_t     wbuffer;          /* Tampon d'écriture                */

    /* Contrôle des paramètres */
    assert( filename );
    assert( buffer );

    /* Initialise l'arbre */
    memset( tree, 0, NUM_CHARS * sizeof (*tree) );
    count = NUM_CHARS;

    /* Calcule le nombre d'occurence de chaque octet */
    if (size != (unsigned int) -1)
	for (i = 0; i < size; i++)
	    tree[(int) buffer[i]].freq++;
    else {
	/* Si size vaut -1 : cas spécial d'une chaîne de caractères */
	for (i = 0; buffer[i] != '\0'; i++)
	    tree[(int) buffer[i]].freq++;
	size = i;
    }

    /* Initialise le tampon de sortie */
    if (!wbuffer_init( &wbuffer, filename, size ))
	return FALSE;

    /* Cas spécial : si la taille des données à compresser est nulle */
    if (size == 0)
	return TRUE;

    /* Initialise la queue de priorité */
    pq_init( pq );

    /* Ajoute chaque caractère présent dans la queue */
    for (i = 0; i < NUM_CHARS; i++)
	if (tree[i].freq != 0)
	    pq_push( pq, tree + i );

    if (PQ_SIZE( pq ) > 1) {
	/* Tant qu'il ne reste pas qu'un seul élément dans la queue */
	while (PQ_SIZE( pq ) != 1) {
	    /* Retire les deux éléments en début de queue */
	    children[0] = pq_pop( pq );
	    children[1] = pq_pop( pq );

	    /* Met à jour l'arbre en fonction de ces éléments */
	    tree[count].freq    = children[0]->freq + children[1]->freq;
	    children[0]->parent = count;
	    children[1]->parent = -count;

	    /* Finalement, ajoute ce nouvel élément dans la queue */
	    pq_push( pq, tree + count );
	    count++;
	}

	/* Le tout dernier élément n'a pas de parent (racine) */
	pq_pop( pq )->parent = 0;
    } else {
	/* S'il n'y a qu'un seul type de caractère dans les données */
	pq_pop( pq )->parent = count;
	tree[count++].parent = 0;
    }

    /* Définit le codage de chaque caractère */
    for (i = 0; i < NUM_CHARS; i++) {
	/* Rien par défaut */
	codes[i].size = 0;

	/* Si ce caractère est présent */
	if (tree[i].freq != 0) {
	    /* Parcourt l'arbre jusqu'à la racine pour former le code */
	    byte = 0;
	    cur = tree[i].parent;
	    codes[i].bits[0] = 0;

	    do {
		codes[i].bits[byte] = (codes[i].bits[byte] << 1) |
		    (cur >= 0 ? 0 : 1);
		if (++codes[i].size % 8 == 0)
		    codes[i].bits[++byte] = 0;

		cur = tree[cur >= 0 ? cur : -cur].parent;
	    } while (cur != 0);
	}
    }

    /* Écrit la table de codes */
    for (i = 0; i < NUM_CHARS; i++)
	if (!wbuffer_write_bits( &wbuffer, codes[i].size, 8 ) ||
	    (codes[i].size && !wbuffer_write_code( &wbuffer, codes + i )))
	    return FALSE;

    /* Écrit les caractères */
    for (i = 0; i < size; i++)
	if (!wbuffer_write_code( &wbuffer, codes + (int) buffer[i] ))
	    return FALSE;

    /* Vide le tampon et ferme le fichier */
    return wbuffer_finish( &wbuffer );
}


/*****************************************************************************
 *
 * GESTION DE LA QUEUE DE PRIORITÉ (TAS / HEAP)
 *
 */

/**
 * Initialise la queue de priorité
 */
static void pq_init( pq_t pq )
{
    /* Mise à zéro de la taille de la queue */
    PQ_SIZE( pq ) = 0;
}

/**
 * Ajoute un élément à la queue de priorité
 */
static void pq_push( pq_t pq, const huffman_cnode_t elem )
{
    /* Variables locales */
    int             index; /* Index dans le tableau de la queue    */
    huffman_cnode_t swap;  /* Pour la permutation de deux éléments */

    /* Contrôle de la taille */
    assert( PQ_SIZE( pq ) < NUM_NODES );

    /* Ajoutr l'élément à la fin du tableau */
    index = ++PQ_SIZE( pq );
    pq[index] = elem;

    /* Remonte l'élément dans la queue tant que c'est possible */
    while (index > 1 && pq[index]->freq < pq[index / 2]->freq) {
	swap = pq[index];
	pq[index] = pq[index / 2];

	index /= 2;
	pq[index] = swap;
    }
}

/**
 * Supprime l'élément en début de queue de priorité et le retourne
 */
static huffman_cnode_t pq_pop( pq_t pq )
{
    /* Variables locales */
    huffman_cnode_t result, swap;   /* Résultat et variable de permutation */
    unsigned int   index, greatest; /* Index des éléments manipulés        */

    /* Contrôle de la taille */
    assert( PQ_SIZE( pq ) != 0 );

    /* Sauvegarde le dernier élément et le remplace par le dernier */
    result = pq[1];
    pq[1]  = pq[PQ_SIZE( pq )--];
    index  = 1;

    /* Fait descendre cet élément dans la queue autant que nécessaire */
    while (index <= PQ_SIZE( pq ) / 2 &&
	   (pq[index]->freq > pq[index * 2]->freq ||
	    (index < (PQ_SIZE( pq ) + 1) / 2 &&
	     pq[index]->freq > pq[index * 2 + 1]->freq))) {
	/* Sélection du bon fils du noeud courant pour l'échange */
	greatest = index * 2;
	if (index < (PQ_SIZE( pq ) + 1) / 2 &&
	    pq[index * 2]->freq > pq[index * 2 + 1]->freq)
	    greatest++;

	/* Échange les éléments */
	swap = pq[index];
	pq[index] = pq[greatest];
	pq[greatest] = swap;

	/* Mise à jour de l'élément courant */
	index = greatest;
    }

    /* Retourne le précédent premier élément de la queue */
    return result;
}


/*****************************************************************************
 *
 * GESTION DU TAMPON DE LECTURE
 *
 */

/**
 * Lit des données depuis le fichier vers le tampon
 */
static bool_t rbuffer_read_byte( rbuffer_t buffer )
{
    /* Contrôle des paramètres */
    assert( buffer );

    /* Il ne reste pas assez de caractère dans le tampon */
    if (buffer->fb_pos == buffer->fb_size) {
	/* Lit les données dans le fichier */
	if ((signed int) (buffer->fb_size = read( buffer->fd,
						  buffer->file_buffer,
						  BUFFER_SIZE )) <= 0) {
	    /* Erreur de lecture */
	    close( buffer->fd );
	    return FALSE;
	}

	/* Mise à zéro de l'index dans le tampon */
	buffer->fb_pos = 0;
    }

    /* Ajout de l'octet lu dans le tampon d'entrée */
    buffer->byte_buffer |= (unsigned int)
	buffer->file_buffer[buffer->fb_pos++] << buffer->bb_remain;
    buffer->bb_remain   += 8;

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Initialise le tampon de lecture
 */
static bool_t rbuffer_init( rbuffer_t buffer, const char *filename )
{
    /* Contrôle des paramètres */
    assert( buffer );
    assert( filename );

    /* Ouvre le fichier */
    if ((buffer->fd = open( filename, O_RDONLY )) == -1)
	return FALSE;

    /* Lit le début du fichier et vérifie l'en-tête */
    if ((signed int) (buffer->fb_size = read( buffer->fd, buffer->file_buffer,
					      BUFFER_SIZE )) < 8 ||
	buffer->file_buffer[0] != HEADER0 ||
	buffer->file_buffer[1] != HEADER1 ||
	buffer->file_buffer[2] != HEADER2 ||
	buffer->file_buffer[3] != HEADER3) {
	close( buffer->fd );
	return FALSE;
    }

    /* Calcule la taille des données */
    if ((buffer->size = (unsigned int) buffer->file_buffer[4] |
	 ((unsigned int) buffer->file_buffer[5] << 8) |
	 ((unsigned int) buffer->file_buffer[6] << 16) |
	 ((unsigned int) buffer->file_buffer[7] << 24)) == 0) {
	close( buffer->fd );
	return TRUE;
    }

    /* Il faut au moins un octet supplémentaire */
    if (buffer->fb_size < 9) {
	close( buffer->fd );
	return FALSE;
    }

    /* Initialisation du tampon de lecture */
    buffer->fb_pos      = 9;
    buffer->bb_remain   = 8;
    buffer->byte_buffer = buffer->file_buffer[8];

    /* Pas d'erreur */
    return TRUE;
}


/*****************************************************************************
 *
 * GESTION DU TAMPON D'ÉCRITURE
 *
 */

/**
 * Écrire le tampon entier dans le fichier.
 */
static bool_t wbuffer_write( wbuffer_t buffer )
{
    /* Contrôle des paramètres */
    assert( buffer );

    if (buffer->fb_size != 0) {
	/* Écrit le tampon */
	if (write( buffer->fd, buffer->file_buffer, buffer->fb_size) != -1)
	    /* Remet à zéro la taille du tampon */
	    buffer->fb_size = 0;
	else {
	    /* Erreur */
	    close( buffer->fd );
	    return FALSE;
	}
    }

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Ajoute un caractère dans le tampon de sortie.
 */
static bool_t wbuffer_flush( wbuffer_t buffer )
{
    /* Contrôle des paramètres */
    assert( buffer );

    while (buffer->bb_size >= 8) {
	/* Écrit le caractère et retourne en cas d'erreur */
	buffer->file_buffer[buffer->fb_size++] =
	    (unsigned char) buffer->byte_buffer;
	if (buffer->fb_size == BUFFER_SIZE && !wbuffer_write( buffer ))
	    return FALSE;

	/* Met à jour le tampon et sa taille */
	buffer->byte_buffer >>= 8;
	buffer->bb_size      -= 8;
    }

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Écrit le tampon, même non plein, dans le fichier.
 */
static bool_t wbuffer_finish( wbuffer_t buffer )
{
    /* Contrôle des paramètres */
    assert( buffer );

    /* Écrit les derniers bits */
    if (buffer->bb_size != 0) {
	buffer->file_buffer[buffer->fb_size++] =
	    (unsigned char) buffer->byte_buffer;

	/* Met à jour le tampon et sa taille */
	buffer->bb_size     = 0;
	buffer->byte_buffer = 0;
    }

    /* Ferme le fichier après avoir vidé le tampon */
    if (wbuffer_write( buffer )) {
	close( buffer->fd );
	return TRUE;
    }
    return FALSE;
}

/**
 * Écrit quelques bits (9 ou 25 maximum suivant l'architecture, respectivement
 * 16-bit et 32-bit) dans le tampon.
 */
static bool_t wbuffer_write_bits( wbuffer_t buffer, unsigned int data,
				  unsigned int size )
{
    /* Contrôle des paramètres */
    assert( buffer );

    /* Ajoute les bits */
    buffer->byte_buffer |= data << buffer->bb_size;
    buffer->bb_size     += size;

    /* Écrit le tampon */
    return wbuffer_flush( buffer );
}

/**
 * Écrire un code complet dans le tampon.
 */
static bool_t wbuffer_write_code( wbuffer_t buffer, huffman_code_t code )
{
    /* Variables locales */
    int          byte;  /* Index d'octet du code  */
    unsigned int count; /* Nombre de bits restant */

    /* Contrôle des paramètres */
    assert( buffer );
    assert( code );

    /* Écrit les premiers bits */
    byte = (code->size - 1) / 8;
    if ((count = code->size % 8) != 0 &&
	!wbuffer_write_bits( buffer, code->bits[byte--], count ))
	    return FALSE;

    /* Écrit les octets complets */
    while (byte >= 0)
	if (!wbuffer_write_bits( buffer, code->bits[byte--], 8 ))
	    return FALSE;

    /* Pas d'erreur */
    return TRUE;
}

/**
 * Initialise le tampon d'écriture.
 */
static bool_t wbuffer_init( wbuffer_t buffer, const char *filename,
			    unsigned int size )
{
    /* Contrôle des paramètres */
    assert( buffer );
    assert( filename );
    assert( BUFFER_SIZE >= 8 );
    
    /* Ouvre le fichier en écriture de retourne en cas d'erreur */
    if ((buffer->fd = creat( filename, 0666 )) == -1)
	return FALSE;

    /* Initialise le buffer avec le début du fichier */
    buffer->file_buffer[0] = HEADER0;
    buffer->file_buffer[1] = HEADER1;
    buffer->file_buffer[2] = HEADER2;
    buffer->file_buffer[3] = HEADER3;

    /* Ajoute la taille des données au tampon */
    buffer->file_buffer[4] = (unsigned char) size;
    buffer->file_buffer[5] = (unsigned char) (size >> 8);
    buffer->file_buffer[6] = (unsigned char) (size >> 16);
    buffer->file_buffer[7] = (unsigned char) (size >> 24);

    /* Écrit et ferme immédiatement le fichier si la taille est nulle */
    if (size == 0) {
	write( buffer->fd, buffer->file_buffer, 8 );
	close( buffer->fd );
	return TRUE;
    }

    /* Met à jour la taille des données présentes dans le tampon */
    buffer->fb_size = 8;

    /* Initialise les autres champs */
    buffer->byte_buffer = 0;
    buffer->bb_size     = 0;

    return TRUE;
}

/* Fin du fichier */
