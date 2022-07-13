#include "lp_lib.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int ncols = 2;

    lprec *lp = make_lp(0, ncols);

    if (!lp)
    {
        printf("Erro na criacao do LP\n");
        return EXIT_FAILURE;
    }

    int *colno = calloc(ncols, sizeof(*colno));
    REAL *row = calloc(ncols, sizeof(*row));

    if (!colno || !row)
    {
        printf("Erro ao alocar memoria\n");
        return EXIT_FAILURE;
    }

    /* makes building the model faster if it is done rows by row */
    set_add_rowmode(lp, TRUE);

    // 120x + 210y <= 15000

    colno[0] = 1; // Primeira coluna
    row[0] = 120;

    colno[1] = 2; // Segunda coluna
    row[1] = 210;

    if (!add_constraintex(lp, ncols, row, colno, LE, 15000))
    {
        printf("Erro ao adicionar linha\n");
        return EXIT_FAILURE;
    }

    // 110x + 30y <= 4000
    colno[0] = 1;
    row[0] = 110;

    colno[1] = 2;
    row[1] = 30;

    if (!add_constraintex(lp, ncols, row, colno, LE, 4000))
    {
        printf("Erro ao adicionar linha\n");
        return EXIT_FAILURE;
    }

    // x + y <= 75
    if (!add_constraintex(lp, ncols, (double[]){1, 1}, (int[]){1, 2}, LE, 75))
    {
        printf("Erro ao adicionar linha\n");
        return EXIT_FAILURE;
    }

    /* rowmode should be turned off again when done building the model */
    set_add_rowmode(lp, FALSE);

    // funcao objetivo 143x + 60y
    if (!set_obj_fnex(lp, ncols, (double[]){143, 60}, (int[]){1, 2}))
    {
        printf("Erro ao adicionar linha\n");
        return EXIT_FAILURE;
    }

    // objetivo -> maximizar
    set_maxim(lp);

    // imprime LP na tela
    write_LP(lp, stdout);

    /* I only want to see important messages on screen while solving */
    set_verbose(lp, IMPORTANT);

    /* Now let lpsolve calculate a solution */
    int ret = solve(lp);

    if (ret != OPTIMAL)
    {
        printf("Erro ao resolver PL\n");
        return EXIT_FAILURE;
    }

    puts("\n\nResultados");

    /* objective value */
    printf("Objective value: %f\n", get_objective(lp));

    /* variable values */
    get_variables(lp, row);
    for (int j = 0; j < ncols; j++)
        printf("%s: %lf\n", get_col_name(lp, j + 1), row[j]);

    free(row);
    free(colno);
    delete_lp(lp);

    return 0;
}
