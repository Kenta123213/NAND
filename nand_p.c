#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>


typedef struct list Tlist;
struct nand {
    unsigned n;                 //ilosc wejsc bramki
    int number_of_inputs;       //ilosc zajetych wejsc
    int number_of_outputs;      //ilosc wyjsc
    bool *boolSignal;           //wskaznik na sygnal
    bool isGate;                //okresla czy struktura jest bramka czy sygnalem
    struct nand **inputs;       //tablica wejsc
    Tlist *outputs;             //lista wyjsc
    bool evaluated;             //okresla czy bramka zostala poddana ewaluacji
    bool checkedForCycle1;      //flaga1 do sprawdzania cyklu, okresla czy algorytm przeszedl przez dana bramke
    bool checkedForCycle2;      //flaga2 do sprawdzania cyklu, okresla czy wynik bramki zostal wyliczony
    bool signal;                //wyliczony sygnal bramki
    ssize_t critical;           //dlugosc sciezki krytycznej
};
typedef struct nand nand_t;

//struktura bedaca lista wyjsc bramki
struct list {
    nand_t *a;                  //wskaznik na bramke
    struct list *next;
};

//struktura pamagajaca w ewaluacji
struct evaluate {
    bool result;
    ssize_t critical;
};
typedef struct evaluate Tevaluate;

void nand_delete(nand_t *g);

//funkcja pomocnicza usuwajaca polaczenie dwoch bramek
void delete_output(nand_t *out, nand_t *in) {
    if (out == NULL || in == NULL) return;

    if (out->outputs->a == in) {
        if (out->number_of_outputs == 1) {
            free(out->outputs);
            out->number_of_outputs = 0;
        } else {
            Tlist *temp = out->outputs->next;
            free(out->outputs);
            out->outputs = temp;
            (out->number_of_outputs)--;
        }
    } else {
        Tlist *temp1 = out->outputs;
        Tlist *temp2 = out->outputs->next;
        while (temp2->a != in) {
            temp1 = temp1->next;
            temp2 = temp2->next;
        }
        temp1->next = temp2->next;
        free(temp2);
        (out->number_of_outputs)--;
    }
}

//funkcja tworzaca nowa bramke
nand_t *nand_new(unsigned n) {
    nand_t *temp = malloc(sizeof(nand_t));
    if (temp == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    temp->n = n;
    temp->number_of_inputs = 0;
    temp->number_of_outputs = 0;
    temp->isGate = true;
    temp->evaluated = false;
    temp->checkedForCycle1 = false;
    temp->checkedForCycle2 = false;
    if (n != 0) {
        temp->inputs = malloc(n * sizeof(nand_t *));
        if (temp->inputs == NULL) {
            free(temp);
            errno = ENOMEM;
            return NULL;
        }
    }
    for (int i = 0; i < (int) n; i++) {
        temp->inputs[i] = NULL;
    }
    return temp;
}

//funkcja usuwajaca polaczenie dwoch bramek
void disconnect_once(nand_t *out, nand_t *in) {
    delete_output(out, in);
    for (unsigned i = 0; i < in->n; i++) {
        if (in->inputs[i] == out) {
            if (out->isGate == false) {
                nand_delete(out);
            }
            (in->number_of_inputs)--;
            in->inputs[i] = NULL;
            break;
        }
    }
}

//funkcja usuwajaca bramke
void nand_delete(nand_t *g) {
    if (g == NULL) {
        return;
    }
    for (int i = 0; i < (int) g->n; i++) {
        if (g->inputs[i] != NULL) {
            if (g->inputs[i]->isGate == false) {
                nand_delete(g->inputs[i]);
            } else {
                delete_output(g->inputs[i], g);
            }
        }
    }
    if ((int) g->n > 0) free(g->inputs);
    if (g->number_of_outputs > 0) {
        Tlist *temp1 = g->outputs;
        Tlist *temp2 = g->outputs;
        while (temp2 != NULL) {
            for (int i = 0; i < (int) temp2->a->n; i++) {
                if (temp2->a->inputs[i] == g) {
                    (temp2->a->number_of_inputs)--;
                    temp2->a->inputs[i] = NULL;
                }
            }
            temp2 = temp2->next;
            free(temp1);
            temp1 = temp2;
        }
    }
    free(g);
}

//funkcja podlaczajaca bramke out to wejscia k bramki in
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
    if (g_out == NULL || g_in == NULL || k >= g_in->n)
    {
        errno = EINVAL;
        return -1;
    }

    Tlist *temp1 = malloc(sizeof(Tlist));
    if (temp1 == NULL) {
        errno = ENOMEM;
        return -1;
    }
    if (g_in->inputs[k] != NULL) {
        disconnect_once(g_in->inputs[k], g_in);
    }
    g_in->inputs[k] = g_out;
    (g_in->number_of_inputs)++;
    temp1->a = g_in;
    temp1->next = NULL;
    if (g_out->number_of_outputs == 0) {
        g_out->outputs = temp1;
    } else {
        Tlist *temp2 = g_out->outputs;
        while (temp2->next != NULL) {
            temp2 = temp2->next;
        }
        temp2->next = temp1;
    }
    (g_out->number_of_outputs)++;
    return 0;
}

//funkcja podlaczajaca sygnal do wejscia k bramki g
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (g == NULL || s == NULL || k >= g->n) {
        errno = EINVAL;
        return -1;
    }

    //tworzenie nowej struktury nand reprezentujacej sygnal
    nand_t *temp1 = malloc(sizeof(nand_t));
    if (temp1 == NULL) {
        free(temp1);
        errno = ENOMEM;
        return -1;
    }
    temp1->n = 0;
    temp1->number_of_inputs = 0;
    temp1->number_of_outputs = 1;
    temp1->boolSignal = (bool *) s;
    temp1->isGate = false;
    temp1->evaluated = true;
    temp1->critical = 0;
    temp1->checkedForCycle1 = false;
    temp1->checkedForCycle2 = true;
    temp1->outputs = malloc(sizeof(Tlist));
    if (temp1->outputs == NULL) {
        free(temp1);
        errno = ENOMEM;
        return -1;
    }
    temp1->outputs->a = g;
    temp1->outputs->next = NULL;
    if (g->inputs[k] != NULL) disconnect_once(g->inputs[k], g);
    g->inputs[k] = temp1;
    (g->number_of_inputs)++;
    return 0;
}

ssize_t nand_fan_out(nand_t const *g) {
    if (g == NULL) {
        errno = EINVAL;
        return -1;
    }
    return (ssize_t) g->number_of_outputs;
}

void *nand_input(nand_t const *g, unsigned k) {
    if (g == NULL || k >= g->n) {
        errno = EINVAL;
        return NULL;
    }

    if (g->inputs[k] == NULL) {
        errno = 0;
        return NULL;
    } else {
        if (g->inputs[k]->isGate) {
            return g->inputs[k];
        } else {
            return g->inputs[k]->boolSignal;
        }
    }
}

nand_t *nand_output(nand_t const *g, ssize_t k) {
    if (g == NULL || k >= nand_fan_out(g) || k < 0) return NULL;

    int i = 0;
    Tlist *temp = g->outputs;
    while (i != k) {
        i++;
        temp = temp->next;
    }
    return temp->a;
}

//rekurencyjne wyliczanie wyniku i sciezki krytycznej bramki g
Tevaluate evaluate(nand_t *g) {
    Tevaluate x;
    x.result = false;
    x.critical = 0;
    if (g->evaluated == false) {
        ssize_t max = 0;
        for (int i = 0; i < (int) g->n; i++) {
            if (g->inputs[i] != NULL) {
                Tevaluate temp = evaluate(g->inputs[i]);
                if (temp.result == false) x.result = true;
                if (temp.critical > max) max = temp.critical;
            }
        }
        if ((int) g->n == 0) {
            g->critical = 0;
            x.critical = 0;
        } else {
            g->critical = (max + 1);
            x.critical = (max + 1);
        }
        g->evaluated = true;
        g->signal = x.result;
    } else {
        if (g->isGate == false) {
            x.result = *(g->boolSignal);
        } else {
            x.result = g->signal;
        }
        x.critical = g->critical;
    }
    return x;
}

//funkcja sprawdza czy bramki potrzebne do wyliczenia outputu bramki g nie są w cyklu
bool is_cyclic_or_invalid(nand_t *g) {
    bool result = false;
    if (g->checkedForCycle1 == false) {
        g->checkedForCycle1 = true;
        for (int i = 0; i < (int) g->n; i++) {
            if (g->inputs[i] != NULL) {
                if (is_cyclic_or_invalid(g->inputs[i])) result = true;
            }
        }
        if (g->number_of_inputs != (int) g->n) result = true;
        g->checkedForCycle2 = true;
    } else {
        if (g->checkedForCycle2 == false) result = true;
    }
    return result;
}

//funkcja przywracajaca domyslne wartosci zmiennych zmienione przez funckje is_cyclic_or_invalid
void rm_cyclic_or_invalid(nand_t *g) {
    if (g == NULL) return;
    if (g->checkedForCycle1 != true) return;

    g->checkedForCycle1 = false;
    g->checkedForCycle2 = false;
    for (int i = 0; i < (int) g->n; i++) {
        rm_cyclic_or_invalid(g->inputs[i]);
    }
}

//funckja przywracaja domyslne wartosci zmiennych zmienione przez funckje evaluate
void rm_eveluate(nand_t *g) {
    if (g == NULL) return;
    if (!(g->evaluated && g->isGate)) return;

    g->evaluated = false;
    for (int i = 0; i < (int) g->n; i++) {
        rm_eveluate(g->inputs[i]);
    }
}

//funkcja zwracajaca najdluzsza sciezke krytyczna tablicy bramek, oraz ich wynniki
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    bool check_for_null = true;
    if (g == NULL || m <= 0) {
        errno = EINVAL;
        return -1;
    }

    for (int i = 0; i < (int) m; i++) {
        if (g[i] == NULL) check_for_null = false;
    }
    if (!check_for_null || s == NULL) {
        errno = EINVAL;
        return -1;
    }

    bool check_for_cycle = true;
    for (int i = 0; i < (int) m; i++) {
        if (is_cyclic_or_invalid(g[i]) == true) check_for_cycle = false;
        rm_cyclic_or_invalid(g[i]);
    }

    if (!check_for_cycle) {
        errno = ECANCELED;
        return -1;
    }

    ssize_t max = 0;
    for (int i = 0; i < (int) m; i++) {
        Tevaluate temp = evaluate(g[i]);
        s[i] = temp.result;
        if (temp.critical > max) max = temp.critical;
    }

    for (int i = 0; i < (int) m; i++) {
        rm_eveluate(g[i]);
    }
    return max;
}