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
// --- Functies ---
Kamer *maak_kamer(int id) {
    Kamer *kamer = malloc(sizeof(Kamer));
    kamer->id = id;
    kamer->verbindingen = NULL;
    kamer->monster = NULL;
    kamer->item = NULL;
    kamer->heeft_schat = 0;
    kamer->bezocht = 0;
    return kamer;
}

void voeg_verbinding_toe(Kamer *van, Kamer *naar) {
    Verbinding *nieuw = malloc(sizeof(Verbinding));
    nieuw->doel = naar;
    nieuw->volgende = van->verbindingen;
    van->verbindingen = nieuw;
}

int is_reeds_verbonden(Kamer *a, Kamer *b) {
    Verbinding *v = a->verbindingen;
    while (v) {
        if (v->doel == b) return 1;
        v = v->volgende;
    }
    return 0;
}

void verbind_kamers(Kamer *a, Kamer *b) {
    if (!is_reeds_verbonden(a, b)) {
        voeg_verbinding_toe(a, b);
        voeg_verbinding_toe(b, a);
    }
}

Dungeon *genereer_dungeon(int aantal) {
    srand(time(NULL));
    Dungeon *d = malloc(sizeof(Dungeon));
    d->aantal_kamers = aantal;
    d->kamers = malloc(sizeof(Kamer *) * aantal);
    for (int i = 0; i < aantal; i++) d->kamers[i] = maak_kamer(i);
    for (int i = 1; i < aantal; i++) {
        int prev = rand() % i;
        verbind_kamers(d->kamers[i], d->kamers[prev]);
        int extra = rand() % 3;
        for (int j = 0; j < extra; j++) {
            int target = rand() % aantal;
            if (!is_reeds_verbonden(d->kamers[i], d->kamers[target]))
                verbind_kamers(d->kamers[i], d->kamers[target]);
        }
    }
    return d;
}

void plaats_schat(Dungeon *d) {
    int schat_id = rand() % d->aantal_kamers;
    d->kamers[schat_id]->heeft_schat = 1;
}

Speler *maak_speler() {
    Speler *s = malloc(sizeof(Speler));
    s->huidige_kamer_id = 0;
    s->hp = 20;
    s->max_hp = 20;
    s->damage = 5;
    return s;
}
