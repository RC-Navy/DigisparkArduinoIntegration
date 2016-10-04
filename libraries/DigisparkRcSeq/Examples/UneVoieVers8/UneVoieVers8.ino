#include <RcSeq.h>

#include <TinyPinChange.h>  /* Ne pas oublier d'inclure la librairie <TinyPinChange>  qui est utilisee par la librairie <RcSeq> */
#include <SoftRcPulseIn.h>  /* Ne pas oublier d'inclure la librairie <SoftRcPulseIn>  qui est utilisee par la librairie <RcSeq> */
#include <Rcul.h>        /* Ne pas oublier d'inclure la librairie <Rcul>        qui est utilisee par la librairie <SoftRcPulseIn> */

/*
IMPORTANT:
Pour compiler ce sketch, RC_SEQ_WITH_SOFT_RC_PULSE_IN_SUPPORT et RC_SEQ_WITH_SHORT_ACTION_SUPPORT doivent etre definie
dans ChemainDesLibraires/(Digispark)RcSeq/RcSeq.h et RC_SEQ_WITH_SOFT_RC_PULSE_OUT_SUPPORT doit etre mis en commentaire.

RC Navy 2013
http://p.loussouarn.free.fr
*/

/*================= COMMMANDE DE 8 SORTIES ON/OFF PAR 8 INTERS POUSSOIR  ========================
   Les 8 relais ou sont connectés aux prise n°1,2,3,4,5,6,7,8 d'un ATtiny84
   La voie du récepteur est connecté à la prise n°0 de l'ATtiny84
   Un appui furtif sur un bouton fait actionne le relais correspondant qui reste collé.
   Un deuxième appui furtif sur le même bouton fait décoller le relais correspondant.
   Version avec librairie RcSeq d'apres l'exemple de http://bateaux.trucs.free.fr/huit_sorties.html
   Test possible en l'etat sur un arduino UNO
================================================================================================*/

/* Declaration des voies */
enum {RC_VOIE, NBR_VOIES_RC}; /* Ici, comme il n'y a qu'une voie, on aurait pu faire un simple "#define RC_VOIE 0" a la place de l'enumeration */

//==============================================================================================
/* Declaration du signal du recepteur */
#define BROCHE_SIGNAL_RECEPTEUR_VOIE  0

//==============================================================================================
/* Declaration d'un clavier "Maison": les impulsions des Boutons-Poussoirs n'ont pas besoin d'etre equidistantes */
enum {BP1, BP2, BP3, BP4, BP5, BP6, BP7, BP8, NBR_BP};
#define TOLERANCE  40 /* Tolerance en + ou en - (en micro-seconde): ATTENTION, il ne doit pas y avoir recouvrement entre 2 zones actives adjascentes. Zone active = 2 x TOLERANCE (us) */
const KeyMap_t ClavierMaison[] PROGMEM ={  {VALEUR_CENTRALE_US(1100,TOLERANCE)}, /* BP1: +/-40 us */
                                           {VALEUR_CENTRALE_US(1200,TOLERANCE)}, /* BP2: +/-40 us */
                                           {VALEUR_CENTRALE_US(1300,TOLERANCE)}, /* BP3: +/-40 us */
                                           {VALEUR_CENTRALE_US(1400,TOLERANCE)}, /* BP4: +/-40 us */
                                           {VALEUR_CENTRALE_US(1600,TOLERANCE)}, /* BP5: +/-40 us */
                                           {VALEUR_CENTRALE_US(1700,TOLERANCE)}, /* BP6: +/-40 us */
                                           {VALEUR_CENTRALE_US(1800,TOLERANCE)}, /* BP7: +/-40 us */
                                           {VALEUR_CENTRALE_US(1900,TOLERANCE)}, /* BP8: +/-40 us */
                                        };
//==============================================================================================
/* Astuce: une table pour associer un identifiant a un n° de broche (l'identifiant 0 ne sera pas utiliser ici: on commence a Idx = 1 */
                                   /* Idx: 0,   1,   2,   3,   4,   5,   6,   7,   8 */
const uint8_t IdxVersBroche[] PROGMEM ={   0,   1,   2,   3,   4,   5,   6,   7,   8}; /* Adapter ici les N° de broches si ils doivent etre differents de l'identifiant */
/* Une macro pour recuperer le n° de broche correspondant a un identifiant (ameliore la lisibilite du programme) */
#define BROCHE_POUR_IDX(Idx)    (uint8_t)pgm_read_byte(&IdxVersBroche[Idx])

//==============================================================================================
/* Astuce: une macro pour n'ecrire qu'une seule fois la fonction ActionX() */
#define DECLARE_ACTION(Idx)                              \
void Action##Idx(void)                                   \
{                                                        \
static uint32_t DebutMs = millis();                      \
static boolean Etat = HIGH;                              \
/* Depuis la version 2.0 de la lib <RcSeq>, pour      */ \
/* des raisons de reactivite, la tempo inter-commande */ \
/* doit etre geree dans le sketch utilisateur.        */ \
  if(millis() - DebutMs >= 500UL)                        \
  {                                                      \
    DebutMs = millis();                                  \
    digitalWrite(BROCHE_POUR_IDX(Idx), Etat);            \
    Etat = !Etat;                                        \
  }                                                      \
}

/* Declaration des actions en utilisant la macro DECLARE_ACTION(Idx) avec Idx = le numero de l'action et de la pin (le ##Idx sera remplace automatiquement par la valeur de Idx */
DECLARE_ACTION(1)
DECLARE_ACTION(2)
DECLARE_ACTION(3)
DECLARE_ACTION(4)
DECLARE_ACTION(5)
DECLARE_ACTION(6)
DECLARE_ACTION(7)
DECLARE_ACTION(8)

//==============================================================================================
void setup()
{
    RcSeq_Init();
    RcSeq_DeclareSignal(RC_VOIE, BROCHE_SIGNAL_RECEPTEUR_VOIE);
    RcSeq_DeclareClavierMaison(RC_VOIE, RC_CLAVIER_MAISON(ClavierMaison));
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP1, Action1); pinMode(BROCHE_POUR_IDX(1), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP2, Action2); pinMode(BROCHE_POUR_IDX(2), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP3, Action3); pinMode(BROCHE_POUR_IDX(3), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP4, Action4); pinMode(BROCHE_POUR_IDX(4), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP5, Action5); pinMode(BROCHE_POUR_IDX(5), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP6, Action6); pinMode(BROCHE_POUR_IDX(6), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP7, Action7); pinMode(BROCHE_POUR_IDX(7), OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE, BP8, Action8); pinMode(BROCHE_POUR_IDX(8), OUTPUT);
}
//==============================================================================================
void loop()
{
    RcSeq_Rafraichit();  
}
//============================ FIN DU PROGRAMME =================================================

