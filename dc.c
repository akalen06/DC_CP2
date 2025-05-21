#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Structs ---
typedef struct Speler Speler;
typedef struct Kamer Kamer;

typedef struct Monster {
    char *naam;
    int hp;
    int damage;
} Monster;

typedef struct Item {
    char *naam;
    void (*effect)(Speler *);
} Item;

typedef struct Verbinding {
    Kamer *doel;
    struct Verbinding *volgende;
} Verbinding;

struct Kamer {
    int id;
    Verbinding *verbindingen;
    Monster *monster;
    Item *item;
    int heeft_schat;
    int bezocht;
};

typedef struct Dungeon {
    Kamer **kamers;
    int aantal_kamers;
} Dungeon;

struct Speler {
    int huidige_kamer_id;
    int hp;
    int max_hp;
    int damage;
};
