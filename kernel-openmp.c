/*-----------------------------------------------------------------------*/
/* Program: STREAM                                                       */
/* Revision: $Id: stream.c,v 5.10 2013/01/17 16:01:06 mccalpin Exp mccalpin $ */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2013: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*           "tuned STREAM benchmark results"                            */
/*           "based on a variant of the STREAM benchmark code"           */
/*         Other comparable, clear, and reasonable labelling is          */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "stream.h"


void stream_allocate(stream_t ** a, stream_t ** b, stream_t ** c)
{
  size_t const size = sizeof(STREAM_TYPE) * (STREAM_ARRAY_SIZE + OFFSET);
  if (NULL == (*a = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for a\n", size);
    exit(1);
  }
  if (NULL == (*b = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for b\n", size);
    exit(1);
  }
  if (NULL == (*c = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for c\n", size);
    exit(1);
  }
}

void stream_copy(stream_t * c, stream_t * a)
{
  #pragma omp parallel for
  for (size_t j=0; j<STREAM_ARRAY_SIZE; ++j) {
    c[j] = a[j];
  }
}

void stream_scale(stream_t scalar, stream_t * b, stream_t * c)
{
  #pragma omp parallel for
  for (size_t j=0; j<STREAM_ARRAY_SIZE; ++j) {
    b[j] = scalar*c[j];
  }
}

void stream_add(stream_t * c, stream_t * a, stream_t * b)
{
  #pragma omp parallel for
  for (size_t j=0; j<STREAM_ARRAY_SIZE; ++j) {
    c[j] = a[j] + b[j];
  }
}

void stream_triad(stream_t scalar, stream_t * a, stream_t * b, stream_t * c)
{
  #pragma omp parallel for
  for (size_t j=0; j<STREAM_ARRAY_SIZE; j++) {
    a[j] = b[j] + scalar*c[j];
  }
} 

