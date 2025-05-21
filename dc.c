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
void herstel_effect(Speler *s) {
    s->hp = s->max_hp;
    printf("HP hersteld naar %d/%d!\n", s->hp, s->max_hp);
}

void verhoog_damage(Speler *s) {
    s->damage += 2;
    printf("Damage verhoogd naar %d!\n", s->damage);
}

Monster *maak_monster(char *naam, int hp, int dmg) {
    Monster *m = malloc(sizeof(Monster));
    m->naam = naam;
    m->hp = hp;
    m->damage = dmg;
    return m;
}

Item *maak_item(char *naam, void (*effect)(Speler *)) {
    Item *i = malloc(sizeof(Item));
    i->naam = naam;
    i->effect = effect;
    return i;
}

void vul_kamers(Dungeon *d) {
    for (int i = 0; i < d->aantal_kamers; i++) {
        Kamer *k = d->kamers[i];
        if (k->heeft_schat) continue;

        int kans = rand() % 100;
        if (kans < 50) {
            k->monster = (rand() % 2) ? maak_monster("Goblin", 8, 3) : maak_monster("Trol", 15, 5);
        } else if (kans < 75) {
            k->item = (rand() % 2) ? maak_item("Health Potion", herstel_effect) : maak_item("Power Ring", verhoog_damage);
        }
    }
}
void voer_gevecht_uit(Speler *s, Monster *m) {
    printf("\nEr is een %s in de kamer! (%d HP, %d damage)\n", m->naam, m->hp, m->damage);
    printf("Gevecht begint!\n");
    
    while (1) {
        int bits = rand() % 16;
        printf("\nAanvalvolgorde: ");
        for (int i = 3; i >= 0; i--) printf("%d", (bits >> i) & 1);
        printf("\n");

        for (int i = 3; i >= 0; i--) {
            if ((bits >> i) & 1) {
                m->hp -= s->damage;
                if (m->hp > 0) {
                    printf("- Je raakt de %s voor %d damage (%d HP over)\n", m->naam, s->damage, m->hp);
                } else {
                    printf("- Je verslaat de %s!\n", m->naam);
                    free(m);
                    return;
                }
            } else {
                s->hp -= m->damage;
                printf("- De %s raakt je voor %d damage (%d/%d HP)\n", m->naam, m->damage, s->hp, s->max_hp);
                if (s->hp <= 0) {
                    printf("\nJe bent verslagen! Game Over!\n");
                    exit(0);
                }
            }
        }
    }
}

Kamer *huidige_kamer(Dungeon *d, Speler *s) {
    return d->kamers[s->huidige_kamer_id];
}

void toon_deuren(Kamer *k) {
    printf("\nBeschikbare deuren:");
    for (Verbinding *v = k->verbindingen; v != NULL; v = v->volgende) {
        printf(" %d%s", v->doel->id, v->doel->bezocht ? "" : "*");
    }
    printf("\n(* = nog niet bezocht)\n");
}

void behandel_kamer(Kamer *k, Speler *s) {
    printf("\n=== Kamer %d ===\n", k->id);
    
    if (k->bezocht) {
        printf("Je bent hier al eerder geweest.\n");
    } else {
        k->bezocht = 1;

        if (k->heeft_schat) {
            printf("\nGEFELICITEERD! Je hebt de schat gevonden!\n");
            exit(0);
        }

        if (k->monster) {
            voer_gevecht_uit(s, k->monster);
            k->monster = NULL;
        }

        if (k->item) {
            printf("\nJe vindt een %s!\n", k->item->naam);
            k->item->effect(s);
            free(k->item);
            k->item = NULL;
        }
        
        if (!k->monster && !k->item && !k->heeft_schat) {
            printf("De kamer is leeg.\n");
        }
    }
    
    printf("\nStatus: HP %d/%d | Damage: %d\n", s->hp, s->max_hp, s->damage);
}
