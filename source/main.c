#include <assert.h>
#include "lp_lib.h"

/*
 * Macro para checagem de erros
 */
#define CHECK(condition, message) assert((condition) && message)

typedef enum coltype_t
{
    TI,    // geraçao mensal da termoelétrica
    WI,    // variação positiva do volume do reservatório
    ZI,    // variação negativa do volume do reservatório
    VOUTI, // volume de saída de água mensal do reservatório
    VI,    // volume mensal do reservatório
} coltype_t;

/*
 * Retorna a coluna correnspondente a uma variável no PL.
 * type: tipo da variável
 * n: total de meses no plano de geração de energia
 * index: mês específico - 1 (0..n-1)
 */
int get_colno(coltype_t type, int n, int index)
{
    return type * n + index + 1;
}

int main(int argc, char const *argv[])
{
    int meses;
    double *demandas, *afluencias;
    double vini, vmax, vmin, k;
    double tmax, ct, ca;

    scanf("%d", &meses);

    demandas = calloc(meses, sizeof(*demandas));
    afluencias = calloc(meses, sizeof(*afluencias));

    for (size_t i = 0; i < meses; i++)
        scanf("%lf", &demandas[i]);

    for (size_t i = 0; i < meses; i++)
        scanf("%lf", &afluencias[i]);

    scanf("%lf %lf %lf %lf", &vini, &vmin, &vmax, &k);
    scanf("%lf %lf %lf", &tmax, &ct, &ca);

    // PL tem cinco variáveis "mensais": ti, wi, zi, vouti, vi
    // como i \e [1..n], onde n é o número de meses, obtemos um PL
    // com 5n colunas
    int ncols = meses * 5;

    lprec *lp = make_lp(0, ncols);
    CHECK(lp, "Erro na criacao do PL");

    set_add_rowmode(lp, TRUE); // modo de inserção de linhas

    for (int i = 0; i < meses; i++)
    {
        // Restrições
        // 1*ti <= tmax
        add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(TI, meses, i)}, LE, tmax);

        // 1*ti + k*vouti >= di
        add_constraintex(lp, 2,
                         (double[]){1, k},
                         (int[]){get_colno(TI, meses, i), get_colno(VOUTI, meses, i)},
                         GE, demandas[i]);

        // vi <= vmax
        add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(VI, meses, i)}, LE, vmax);

        // vi >= vmin
        add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(VI, meses, i)}, GE, vmin);

        // vouti + wi - zi = yi
        add_constraintex(lp, 3,
                         (double[]){1, 1, -1},
                         (int[]){
                             get_colno(VOUTI, meses, i),
                             get_colno(WI, meses, i),
                             get_colno(ZI, meses, i),
                         },
                         EQ, afluencias[i]);

        // como vo = vini:
        // v1 + vout1 = vini + y1
        if (i == 0)
            add_constraintex(lp, 2,
                             (double[]){1, 1},
                             (int[]){
                                 get_colno(VI, meses, 0),
                                 get_colno(VOUTI, meses, i),
                             },
                             EQ, vini + afluencias[i]);

        // vi - v(i-1) + vouti = yi
        else
            add_constraintex(lp, 3,
                             (double[]){1, -1, 1},
                             (int[]){
                                 get_colno(VI, meses, i),
                                 get_colno(VI, meses, i - 1),
                                 get_colno(VOUTI, meses, i),
                             },
                             EQ, afluencias[i]);
    }

    set_add_rowmode(lp, FALSE); // fim do modo de inserção

    int *objcolno = calloc(meses * 3, sizeof(*objcolno));
    double *objrow = calloc(meses * 3, sizeof(*objrow));

    // sT = CT*sum(ti, i=1..n) + CA*sum(wi, i=1..n) + CA*sum(zi, i=1..n)
    for (int i = 0; i < meses; i++)
    {
        objcolno[i] = get_colno(TI, meses, i);
        objrow[i] = ct;

        objcolno[i + meses] = get_colno(WI, meses, i);
        objrow[i + meses] = ca;

        objcolno[i + 2 * meses] = get_colno(ZI, meses, i);
        objrow[i + 2 * meses] = ca;
    }

    set_obj_fnex(lp, meses * 3, objrow, objcolno);
    set_minim(lp); // minimizar função objetivo

    write_LP(lp, stdout); // escreve LP no formato lpsolve

// Resolve o PL
#ifdef RESULT

    solve(lp);

    printf("\nResults\n\n");
    printf("Objective value: %f\n", get_objective(lp));

    double *vars;
    get_ptr_variables(lp, &vars);
    for (int i = 0; i < ncols; i++)
        printf("%s: %lf\n", get_col_name(lp, i + 1), vars[i]);
#endif

    free(demandas);
    free(afluencias);
    free(objrow);
    free(objcolno);
    delete_lp(lp);

    return 0;
}
