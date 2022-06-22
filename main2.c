/**
 * Résolveur de MasterMind intelligent.
 *
 * Notations :
 *  - BC = bonne couleur (bien et mal placé)
 *  - BP = bien placé
*/

#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<locale.h>
#include<assert.h>

// Supprime toutes les informations données en cours de compilation ==> Diminue la durée d'exécution du programme
#define NO_LOG

#define NB_CAR 5
#define NB_COUL 8 // Accepte jusqu'à 256
#define NB_LIGN 12
#define NB_COMBIS 32768 // NB_COMBIS=pow(NB_COUL,NB_CAR)
#define NB_ANSW 21 // NB_ANSW=(NB_CAR+1) * (NB_CAR+2) / 2

#define NOM_SOLUTION "Solution - MM5P8C.txt"
#define NOM_STATS "Stats - MM5P8C.csv"


const char color_list[NB_COUL] = { 'R','B','V','J','M','O','W','N' };

typedef unsigned char Couleur;

struct Result {
    unsigned char nb_bp;
    unsigned char nb_bc;
};

// Structure qui sauvegarde la manière dont est fragmenté la liste `combis`
struct CombisOrderer {
    Couleur* c_begin;
    size_t c_size;
    struct CombisOrderer* answers[NB_ANSW];
};

// Variable globale
Couleur combis[NB_COMBIS * NB_CAR];
struct CombisOrderer* main_order;
size_t nb_coup[NB_LIGN];
size_t max_coup;

// La fonction puissance pour les entiers
int Fpow(int b, int p) {
    int res = 1;
    for (int i = 0; i < p; ++i)
        res *= b;
    return res;
}

// Initialise la variable `combis` pour qu'elle contienne toute les combinaisons
void init_combi(void) {
    for (int i = 0; i < NB_COMBIS; ++i)
        for (int j = 0; j < NB_CAR; ++j)
            combis[i*NB_CAR + j] = (Couleur)((int)(i / (int)Fpow(NB_COUL, j)) % NB_COUL); // Changement de base (base 10 -> base NB_CAR)
    main_order = malloc(sizeof(struct CombisOrderer));
    if (main_order == NULL) {
        perror("Ca commence mal :( ...");
        exit(1);
    }
    main_order->c_begin = combis;
    main_order->c_size = sizeof(combis);
}

// Libère toutes les instances de 'struct CombisOrderer' qui ont été allouées
void free_orderer(struct CombisOrderer* order) {
    if (order == NULL)
        return;
    for (int i = 0; i < NB_ANSW; ++i) {
        free_orderer(order->answers[i]);
    }
    free(order);
}

/**
* Compare 2 listes de `Couleur`
* \return Le nombre de BP et de BC
*/
struct Result compare(Couleur* cs1, Couleur* cs2, size_t taille) {
    struct Result res = {0,0};
    char* utilise_j = malloc(taille);
    if (utilise_j == NULL) {
        perror("Bad stuff going on... [compare]");
        exit(1);
    }
    memset(utilise_j, 0, taille);
    for (int i = 0; i < taille; ++i) {
        for (int j = 0; j < taille; ++j) {
            if (cs1[i] == cs2[j] && !utilise_j[j]) {
                ++res.nb_bc;
                utilise_j[j] = 1;
                break;
            }
        }
    }
    for (int i = 0; i < taille; ++i)
        if (cs1[i] == cs2[i])
            ++res.nb_bp;
    free(utilise_j);
    return res;
}

// Calcule l'entropie d'une combinaison
double get_info(Couleur combi[NB_CAR], Couleur* rest_combis, size_t taille) {
    unsigned int infos[NB_ANSW];
    memset(infos, 0, NB_ANSW * sizeof(int));
    for (int i = 0; i < taille; ++i) {
        struct Result res = compare(combi, rest_combis + i * NB_CAR, NB_CAR);
        ++infos[(int)(res.nb_bc * (res.nb_bc + 1) * .5) + res.nb_bp];
    }

    double resultat = 0;
    for (int i = 0; i < NB_ANSW; ++i)
        if (infos[i] != 0)
            resultat += infos[i] / (double)taille * log2(taille / (double)infos[i]);
    return resultat;
}

// Trouve la combinaison avec la plus grande entropie
unsigned int find_combi(Couleur* rest_combis, size_t taille) {
    double max_info = 0;
    int max_info_combi = 0;
    for (int i = 0; i < taille; ++i) {
        double info = get_info(rest_combis + i * NB_CAR, rest_combis, taille);
        if (max_info < info) {
            max_info = info;
            max_info_combi = i;
        }
    }

#ifndef NO_LOG
    printf_s("Combinaison : ");
    for (int i = 0; i < NB_CAR; ++i)
        putchar(color_list[rest_combis[max_info_combi * NB_CAR + i]]);
    printf_s(" -> Information espéré : %.3f bites\n", max_info);
#endif // !NO_LOG

    return max_info_combi;
}

// Trie la variable `combis` de manière à séparer chaque combinaison en fonction des indications auxquelles elles correspondent
// Met à jour la variable `main_order` de manière à indiquer les séparations réalisées
void combi_sorting(Couleur spliter[NB_CAR], struct CombisOrderer* combi_order) {
    const size_t taille = combi_order->c_size / NB_CAR;
    const Couleur* rest_combis = combi_order->c_begin;
    if (taille == 1) {
        // Evite d'effectuer tout tri si il ne reste plus qu'un élément
        // Evite une récursion infinie dans le 'combi_order'
        for (int i = 0; i < NB_ANSW; ++i)
            combi_order->answers[i] = NULL;
        return;
    }
    Couleur* split_combis = malloc(NB_ANSW * taille * NB_CAR);
    if (split_combis == NULL) {
        perror("Erreur d'allocation de 'splitcombis'...");
        exit(1);
    }
    unsigned int indexes[NB_ANSW];
    memset(indexes, 0, NB_ANSW * sizeof(int));
    
    for (int i = 0; i < taille; ++i) {
        struct Result res = compare(spliter, rest_combis + i * NB_CAR, NB_CAR);
        int answ_id = (int)(res.nb_bc * (res.nb_bc + 1) * .5) + res.nb_bp;
        memcpy(split_combis + answ_id * taille * NB_CAR + indexes[answ_id], rest_combis + i * NB_CAR, NB_CAR * sizeof(Couleur));
        indexes[answ_id] += NB_CAR;
    }
    unsigned int index_sum = 0;
    for (int i = 0; i < NB_ANSW; ++i) {
        if (indexes[i] == 0) {
            // Evite d'allouer de la méoire si le résultat est impossible
            combi_order->answers[i] = NULL;
            continue;
        }
        memcpy(rest_combis + index_sum, split_combis + i * NB_CAR * taille, indexes[i]);
        struct CombisOrderer* c_order = malloc(sizeof(struct CombisOrderer)); // Alloue une 'struct CombisOrderer -> Appeler pour désallouer 'free_orderer(...)'
        if (c_order == NULL) {
            perror("Erreur d'allocation dans le CombisOrderer !");
            free(split_combis);
            exit(1);
        }
        combi_order->answers[i] = c_order;
        combi_order->answers[i]->c_begin = rest_combis + index_sum;
        index_sum += indexes[i];
        combi_order->answers[i]->c_size = indexes[i];
    }
    free(split_combis);
}

// Trouve tous les suites de combinaisons à jouer pour chaque combinaison différente
void solver(struct CombisOrderer* order, size_t recursion) {
#ifndef NO_LOG
    printf_s("%d. ", recursion);
#endif // !NO_LOG

    int index = find_combi(order->c_begin, order->c_size / NB_CAR);
    combi_sorting(order->c_begin + index * NB_CAR, order);
    for (int i = 0; i < NB_ANSW; ++i)
        if (order->answers[i] != NULL)
            solver(order->answers[i], recursion + 1);
}

// Met à jour les statistiques (pour le fichier .csv)
void mono_stat(size_t recursion) {
    if (recursion >= max_coup)
        max_coup = recursion + 1;
    if (recursion < NB_LIGN)
        ++nb_coup[recursion];
}

// Sauvegarde les données 
// Fonction codée de manière bizarre ==> Moche
static data_saver(struct CombisOrderer* order, FILE* file, size_t recursion) {
    for (int i = 0; i < NB_ANSW; ++i) {
        if (order->answers[i] != NULL) {
            for (int j = 0; j < 4 * recursion; ++j)
                fputc(' ', file);
            if (order->answers[i]->answers[NB_ANSW - 1] == NULL) {
                if (i != NB_ANSW - 1) {
                    fprintf_s(file, "%d -> ", i);
                    for (int j = 0; j < NB_CAR; ++j)
                        fputc(color_list[*(order->answers[i]->c_begin + j)], file);
                    fputc('\n', file);
                    for (int j = 0; j < 4 * recursion + 4; ++j)
                        fputc(' ', file);
                    fprintf_s(file, "%d -> Victoire !!\n", NB_ANSW - 1);
                    mono_stat(recursion);
                }
                else {
                    fprintf_s(file, "%d -> Victoire !!\n", NB_ANSW - 1);
                    mono_stat(recursion - 1);
                }
            }
            else {
                fprintf_s(file, "%d -> ", i);
                for (int j = 0; j < NB_CAR; ++j)
                    fputc(color_list[*(order->answers[i]->answers[NB_ANSW - 1]->c_begin + j)], file);
                fputc('\n', file);
                data_saver(order->answers[i], file, recursion + 1);
            }
        }
    }
}

// Sauvegarde les données
void save_data(struct CombisOrderer* order, FILE* file) {
    if (order == NULL)
        return;
    for (int i = 0; i < NB_CAR; ++i)
        fputc(color_list[*(order->answers[NB_ANSW - 1]->c_begin + i)], file);
    fputc('\n', file);
    data_saver(order, file, 1);
}

int main() {
    // Check
    if (NB_COMBIS != Fpow(NB_COUL, NB_CAR)) exit(5);
    if (NB_ANSW != (NB_CAR + 1) * (NB_CAR + 2) / 2) exit(5);



    init_combi();
    printf_s("");
    solver(main_order, 0);

    FILE* f;
    fopen_s(&f, NOM_SOLUTION, "w");
    if (f == NULL)
        return -1;
    save_data(main_order, f);
    if (fclose(f) == EOF)
        return -2;

    free_orderer(main_order);

    // --- Statistiques --- //
    double moy = 0;
    for (int i = 0; i < min(max_coup, NB_LIGN); ++i)
        moy += (i + 1) * ((float)(nb_coup[i]) / NB_COMBIS);
    printf_s("\n\n--- Statistiques ---\n\nMaximum de coup : %d\nMoyenne de coup : %.3f", max_coup, moy);
    fopen_s(&f, NOM_STATS, "w");
    if (f == NULL)
        return -1;
    if (setlocale(LC_ALL, "") == NULL)
        perror("Echec du changement de locale !");
    fprintf_s(f, "Nombre de coup;Nombre de combinaisons;Probabilité\n");
    for (int i = 0; i < min(max_coup, NB_LIGN); ++i)
        fprintf_s(f, "%d;%d;%.6f\n", i + 1, nb_coup[i], (float)(nb_coup[i]) / NB_COMBIS);
    fprintf_s(f, "\nMoyenne;%.6f", moy);
    if (fclose(f) == EOF)
        return -2;
    // -------------------- //


    return 0;
}