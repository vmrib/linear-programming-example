#include <assert.h>
#include "lp_lib.h"

#define CHECK(condition, message) assert((condition) && message)
typedef enum coltype_t
{
    TI_T,
    WI_H,
    ZI_H,
    VOUTI_H,
    VI_H,
} coltype_t;

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

    // ti_T*n + wi_H*n + zi_H*n + vouti_H*n + vi_H*n, n = meses
    int ncols = meses * 5;
    // double sparserow[3];
    // int colno[3];

    lprec *lp = make_lp(0, ncols);
    CHECK(lp, "Erro na criacao do PL");

    set_add_rowmode(lp, TRUE);

    // colno[0] = get_colno(VI_H, meses, 0);
    // sparserow[0] = 1;
    // 1 * v0 = vini
    // CHECK(
    //     add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(VI_H, meses, 0)}, EQ, vini),
    //     "Erro ao inserir restricao");

    for (int i = 0; i < meses; i++)
    {
        // 1 * ti_T <= tmax_T
        CHECK(
            add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(TI_T, meses, i)}, LE, tmax),
            "Erro ao inserir restricao");

        // 1 * ti_T + k_H * vouti_H >= di
        add_constraintex(lp, 2,
                         (double[]){1, k},
                         (int[]){get_colno(TI_T, meses, i), get_colno(VOUTI_H, meses, i)},
                         GE, demandas[i]);

        // vi_H <= vmax
        add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(VI_H, meses, i)}, LE, vmax);

        // vi_H >= vmin
        add_constraintex(lp, 1, (double[]){1}, (int[]){get_colno(VI_H, meses, i)}, GE, vmin);

        // vouti_H + wi_H - zi_H = yi_H
        add_constraintex(lp, 3,
                         (double[]){1, 1, -1},
                         (int[]){
                             get_colno(VOUTI_H, meses, i),
                             get_colno(WI_H, meses, i),
                             get_colno(ZI_H, meses, i),
                         },
                         EQ, afluencias[i]);

        // vi_H - v(i-1)_H + vouti_H = yi_H
        if (i == 0)
            add_constraintex(lp, 2,
                             (double[]){1, 1},
                             (int[]){
                                 get_colno(VI_H, meses, 0),
                                 get_colno(VOUTI_H, meses, i),
                             },
                             EQ, vini + afluencias[i]);
        else
            add_constraintex(lp, 3,
                             (double[]){1, -1, 1},
                             (int[]){
                                 get_colno(VI_H, meses, i),
                                 get_colno(VI_H, meses, i - 1),
                                 get_colno(VOUTI_H, meses, i),
                             },
                             EQ, afluencias[i]);
    }

    set_add_rowmode(lp, FALSE);

    int *objcolno = calloc(meses * 3, sizeof(*objcolno));
    double *objrow = calloc(meses * 3, sizeof(*objrow));

    for (int i = 0; i < meses; i++)
    {
        objcolno[i] = get_colno(TI_T, meses, i);
        objrow[i] = ct;

        objcolno[i + meses] = get_colno(WI_H, meses, i);
        objrow[i + meses] = ca;

        objcolno[i + 2 * meses] = get_colno(ZI_H, meses, i);
        objrow[i + 2 * meses] = ca;
    }

    set_obj_fnex(lp, meses * 3, objrow, objcolno);
    set_minim(lp);

    write_LP(lp, stdout);

    int ret = solve(lp);

    printf("Objective value: %f\n", get_objective(lp));

    double *vars;
    get_ptr_variables(lp, &vars);
    for (int i = 0; i < ncols; i++)
        printf("%s: %lf\n", get_col_name(lp, i + 1), vars[i]);

    free(demandas);
    free(afluencias);
    free(objrow);
    free(objcolno);
    delete_lp(lp);

    return 0;
}
