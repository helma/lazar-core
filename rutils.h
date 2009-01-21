/* Copyright (C) 2005  Christoph Helma <helma@in-silico.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef RPCA_H
#define RPCA_H

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <R.h>
#include <Rinternals.h>
#include <Rembedded.h>
#include <Rdefines.h>

using namespace std;

void init_R(int argc, char **argv);
void end_R();
SEXP R_exec4 (const char* command, SEXP structure1, SEXP structure2, SEXP structure3);
SEXP R_exec3 (const char* command, SEXP structure1, SEXP structure2);
SEXP R_exec (const char* command, SEXP structure);
SEXP get_list_element(SEXP list, char *str);
void print_matrix(gsl_matrix* m);
void print_vector(gsl_vector* v);
void matrix_gsl2R(SEXP* mr, gsl_matrix* m);
void vector_gsl2R(SEXP* vr, gsl_vector* v);
void vector_gsl2R_str(SEXP* vr, gsl_vector* v);
void matrix_R2gsl(gsl_matrix* m, SEXP* mr);
void vector_R2gsl(gsl_vector* v, SEXP* vr);
float getVectorMean(gsl_vector* v);
void transposeMatrix(gsl_matrix* m, gsl_matrix* mt);
void cov(gsl_matrix* m, gsl_matrix* covm);
void get_mean(gsl_matrix* m, gsl_vector* mean);
float mahal(gsl_vector* x, gsl_matrix* m, gsl_matrix* covm);
gsl_matrix* pca_cols(gsl_matrix* feature_matrix, gsl_vector* means, unsigned int no_c);
gsl_matrix* pca(gsl_matrix* feature_matrix, gsl_vector* means, float sig_limit);
gsl_matrix* transformData (gsl_matrix* data_c, gsl_matrix* rot, gsl_vector* means);
gsl_matrix* reconstructData (gsl_matrix* t_data, gsl_matrix* rot, gsl_vector* means);

#endif
