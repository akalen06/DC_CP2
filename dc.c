#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

#define SAVE_FILE "dungeon_save.dat"

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
    m->naam = strdup(naam);
    m->hp = hp;
    m->damage = dmg;
    return m;
}

Item *maak_item(char *naam, void (*effect)(Speler *)) {
    Item *i = malloc(sizeof(Item));
    i->naam = strdup(naam);
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
                    free(m->naam);
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
            remove(SAVE_FILE); // Verwijder save bij winst
            exit(0);
        }

        if (k->monster) {
            voer_gevecht_uit(s, k->monster);
            k->monster = NULL;
        }

        if (k->item) {
            printf("\nJe vindt een %s!\n", k->item->naam);
            k->item->effect(s);
            free(k->item->naam);
            free(k->item);
            k->item = NULL;
        }
        
        if (!k->monster && !k->item && !k->heeft_schat) {
            printf("De kamer is leeg.\n");
        }
    }
    
    printf("\nStatus: HP %d/%d | Damage: %d\n", s->hp, s->max_hp, s->damage);
}

void save_game(Dungeon *d, Speler *s) {
    FILE *file = fopen(SAVE_FILE, "wb");
    if (!file) {
        perror("Fout bij openen save bestand");
        return;
    }

    // Schrijf dungeon basisinfo
    fwrite(&d->aantal_kamers, sizeof(int), 1, file);

    // Schrijf speler data
    fwrite(s, sizeof(Speler), 1, file);

    // Schrijf elke kamer
    for (int i = 0; i < d->aantal_kamers; i++) {
        Kamer *k = d->kamers[i];
        fwrite(&k->id, sizeof(int), 1, file);
        fwrite(&k->heeft_schat, sizeof(int), 1, file);
        fwrite(&k->bezocht, sizeof(int), 1, file);

        // Schrijf monster data (indien aanwezig)
        int heeft_monster = (k->monster != NULL);
        fwrite(&heeft_monster, sizeof(int), 1, file);
        if (heeft_monster) {
            int naam_len = strlen(k->monster->naam) + 1;
            fwrite(&naam_len, sizeof(int), 1, file);
            fwrite(k->monster->naam, sizeof(char), naam_len, file);
            fwrite(&k->monster->hp, sizeof(int), 1, file);
            fwrite(&k->monster->damage, sizeof(int), 1, file);
        }

        // Schrijf item data (indien aanwezig)
        int heeft_item = (k->item != NULL);
        fwrite(&heeft_item, sizeof(int), 1, file);
        if (heeft_item) {
            int naam_len = strlen(k->item->naam) + 1;
            fwrite(&naam_len, sizeof(int), 1, file);
            fwrite(k->item->naam, sizeof(char), naam_len, file);
            // Sla item type op (0=health, 1=damage)
            int item_type = (k->item->effect == herstel_effect) ? 0 : 1;
            fwrite(&item_type, sizeof(int), 1, file);
        }
    }

    // Schrijf verbindingen
    for (int i = 0; i < d->aantal_kamers; i++) {
        Kamer *k = d->kamers[i];
        int aantal_verbindingen = 0;
        for (Verbinding *v = k->verbindingen; v; v = v->volgende) {
            aantal_verbindingen++;
        }
        fwrite(&aantal_verbindingen, sizeof(int), 1, file);

        for (Verbinding *v = k->verbindingen; v; v = v->volgende) {
            fwrite(&v->doel->id, sizeof(int), 1, file);
        }
    }

    fclose(file);
    printf("\nSpel opgeslagen!\n");
}

int load_game(Dungeon **d, Speler **s) {
    FILE *file = fopen(SAVE_FILE, "rb");
    if (!file) {
        return 0;
    }

    // Lees dungeon basisinfo
    int aantal_kamers;
    fread(&aantal_kamers, sizeof(int), 1, file);

    // Maak nieuwe dungeon en speler
    *d = malloc(sizeof(Dungeon));
    (*d)->aantal_kamers = aantal_kamers;
    (*d)->kamers = malloc(sizeof(Kamer *) * aantal_kamers);
    
    *s = malloc(sizeof(Speler));
    fread(*s, sizeof(Speler), 1, file);

    // Lees kamers
    for (int i = 0; i < aantal_kamers; i++) {
        Kamer *k = maak_kamer(i);
        (*d)->kamers[i] = k;

        fread(&k->id, sizeof(int), 1, file);
        fread(&k->heeft_schat, sizeof(int), 1, file);
        fread(&k->bezocht, sizeof(int), 1, file);

        // Lees monster
        int heeft_monster;
        fread(&heeft_monster, sizeof(int), 1, file);
        if (heeft_monster) {
            int naam_len;
            fread(&naam_len, sizeof(int), 1, file);
            char *naam = malloc(naam_len);
            fread(naam, sizeof(char), naam_len, file);
            
            int hp, damage;
            fread(&hp, sizeof(int), 1, file);
            fread(&damage, sizeof(int), 1, file);
            
            k->monster = malloc(sizeof(Monster));
            k->monster->naam = naam;
            k->monster->hp = hp;
            k->monster->damage = damage;
        }

        // Lees item
        int heeft_item;
        fread(&heeft_item, sizeof(int), 1, file);
        if (heeft_item) {
            int naam_len;
            fread(&naam_len, sizeof(int), 1, file);
            char *naam = malloc(naam_len);
            fread(naam, sizeof(char), naam_len, file);
            
            int item_type;
            fread(&item_type, sizeof(int), 1, file);
            
            k->item = malloc(sizeof(Item));
            k->item->naam = naam;
            k->item->effect = (item_type == 0) ? herstel_effect : verhoog_damage;
        }
    }

    // Lees verbindingen
    for (int i = 0; i < aantal_kamers; i++) {
        Kamer *k = (*d)->kamers[i];
        int aantal_verbindingen;
        fread(&aantal_verbindingen, sizeof(int), 1, file);

        for (int j = 0; j < aantal_verbindingen; j++) {
            int doel_id;
            fread(&doel_id, sizeof(int), 1, file);
            
            Verbinding *nieuw = malloc(sizeof(Verbinding));
            nieuw->doel = (*d)->kamers[doel_id];
            nieuw->volgende = k->verbindingen;
            k->verbindingen = nieuw;
        }
    }

    fclose(file);
    return 1;
}

void spel_loop(Dungeon *d, Speler *s) {
    while (1) {
        Kamer *huidige = huidige_kamer(d, s);
        behandel_kamer(huidige, s);
        toon_deuren(huidige);

        printf("\nKies een deur, 's' om op te slaan, of 'q' om te stoppen: ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        
        if (input[0] == 'q') {
            printf("\nSpel afgesloten.\n");
            exit(0);
        } else if (input[0] == 's') {
            save_game(d, s);
            continue;
        }

        int keuze;
        if (sscanf(input, "%d", &keuze) != 1) {
            printf("Ongeldige invoer!\n");
            continue;
        }

        int geldig = 0;
        for (Verbinding *v = huidige->verbindingen; v; v = v->volgende) {
            if (v->doel->id == keuze) {
                geldig = 1;
                break;
            }
        }

        if (geldig) {
            s->huidige_kamer_id = keuze;
            printf("\nJe loopt naar kamer %d...\n", keuze);
        } else {
            printf("Er is geen deur naar kamer %d!\n", keuze);
        }
    }
}

void cleanup(Dungeon *d, Speler *s) {
    for (int i = 0; i < d->aantal_kamers; i++) {
        Kamer *k = d->kamers[i];
        Verbinding *v = k->verbindingen;
        while (v) {
            Verbinding *next = v->volgende;
            free(v);
            v = next;
        }
        if (k->monster) {
            free(k->monster->naam);
            free(k->monster);
        }
        if (k->item) {
            free(k->item->naam);
            free(k->item);
        }
        free(k);
    }
    free(d->kamers);
    free(d);
    free(s);
}

int main(int argc, char *argv[]) {
    Dungeon *dungeon = NULL;
    Speler *speler = NULL;
    int loaded = 0;

    // Check voor save file bij opstart
    if (access(SAVE_FILE, F_OK) == 0) {
        printf("\nEr is een opgeslagen spel gevonden. Wil je:\n");
        printf("1. Het opgeslagen spel laden\n");
        printf("2. Een nieuw spel starten\n");
        printf("Keuze: ");
        
        int keuze;
        if (scanf("%d", &keuze) != 1) {
            printf("Ongeldige invoer\n");
            return 1;
        }
        getchar(); // Newline consumeren
        
        if (keuze == 1) {
            loaded = load_game(&dungeon, &speler);
            if (!loaded) {
                printf("Fout bij laden van spel. Nieuw spel starten...\n");
            } else {
                printf("Spel geladen!\n");
            }
        }
    }

    if (!loaded) {
        if (argc < 3) {
            printf("Gebruik: %s -n [aantal_kamers]\n", argv[0]);
            return 1;
        }

        if (strcmp(argv[1], "-n") == 0) {
            int aantal = atoi(argv[2]);
            if (aantal < 5) {
                printf("Minimaal 5 kamers nodig\n");
                return 1;
            }
            
            printf("\nGenereren van dungeon met %d kamers...\n", aantal);
            dungeon = genereer_dungeon(aantal);
            plaats_schat(dungeon);
            vul_kamers(dungeon);
            speler = maak_speler();
        } else {
            printf("Ongeldige optie.\n");
            return 1;
        }
    }

    printf("\n=== DUNGEON CRAWLER ===\n");
    printf("Je start in kamer %d. Zoek de schat!\n", speler->huidige_kamer_id);
    printf("Gebruik 's' om op te slaan, 'q' om te stoppen.\n\n");
    
    spel_loop(dungeon, speler);
    cleanup(dungeon, speler);
    return 0;
}
