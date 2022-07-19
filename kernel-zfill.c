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
#include <omp.h>

#include "stream.h"


/* The zfill distance must be large enough to be ahead of the L2 prefetcher */
static const int ZFILL_DISTANCE = 100;

/* 256-byte cache lines */
static const int ELEM_PER_CACHE_LINE = 256 / sizeof(stream_t);

/* Offset from a[j] to zfill */
static const int ZFILL_OFFSET = ZFILL_DISTANCE * ELEM_PER_CACHE_LINE;


static inline void zfill(stream_t * a){
  asm volatile("dc zva, %0": : "r"(a));
}


void stream_allocate(stream_t ** a, stream_t ** b, stream_t ** c)
{
  if (STREAM_ARRAY_SIZE % ELEM_PER_CACHE_LINE) {
    fprintf(stderr, "STREAM_ARRAY_SIZE=%zd is not a multiple of ELEM_PER_CACHE_LINE=%zd\n",
        STREAM_ARRAY_SIZE, ELEM_PER_CACHE_LINE);
    exit(EXIT_FAILURE);
  }
  size_t const size = sizeof(STREAM_TYPE) * (STREAM_ARRAY_SIZE + OFFSET);
  if (NULL == (*a = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for a\n", size);
    exit(EXIT_FAILURE);
  }
  if (NULL == (*b = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for b\n", size);
    exit(EXIT_FAILURE);
  }
  if (NULL == (*c = (STREAM_TYPE*)malloc(size))) {
    fprintf(stderr, "Failed to allocated %zd bytes for c\n", size);
    exit(EXIT_FAILURE);
  }
}


void stream_copy(stream_t * c, stream_t * a)
{
  #pragma omp parallel
  {
    int const tid = omp_get_thread_num();
    int const nthreads = omp_get_num_threads();
    size_t const chunk = STREAM_ARRAY_SIZE / nthreads;

    stream_t * const zfill_limit = c + (tid+1)*chunk - ZFILL_OFFSET;

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const cj = c + j;
      stream_t const * restrict const aj = a + j;

      if (cj+ZFILL_OFFSET < zfill_limit) {
        zfill(cj+ZFILL_OFFSET);
      }

      for (int i=0; i<ELEM_PER_CACHE_LINE; ++i) {
        cj[i] = aj[i];
      }
    }

  } // parallel
}

void stream_scale(stream_t scalar, stream_t * b, stream_t * c)
{
  #pragma omp parallel
  {
    int const tid = omp_get_thread_num();
    int const nthreads = omp_get_num_threads();
    size_t const chunk = STREAM_ARRAY_SIZE / nthreads;

    stream_t * const zfill_limit = b + (tid+1)*chunk - ZFILL_OFFSET;

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const bj = b + j;
      stream_t const * restrict const cj = c + j;

      if (bj+ZFILL_OFFSET < zfill_limit) {
        zfill(bj+ZFILL_OFFSET);
      }

      for (int i=0; i<ELEM_PER_CACHE_LINE; ++i) {
        bj[i] = scalar*cj[i];
      }
    }

  } // parallel
}

void stream_add(stream_t * c, stream_t * a, stream_t * b)
{
  #pragma omp parallel
  {
    int const tid = omp_get_thread_num();
    int const nthreads = omp_get_num_threads();
    size_t const chunk = STREAM_ARRAY_SIZE / nthreads;

    stream_t * const zfill_limit = c + (tid+1)*chunk - ZFILL_OFFSET;

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const cj = c + j;
      stream_t const * restrict const aj = a + j;
      stream_t const * restrict const bj = b + j;

      if (cj+ZFILL_OFFSET < zfill_limit) {
        zfill(cj+ZFILL_OFFSET);
      }

      for (int i=0; i<ELEM_PER_CACHE_LINE; ++i) {
        cj[i] = aj[i] + bj[i];
      }
    }

  } // parallel
}

void stream_triad(stream_t scalar, stream_t * a, stream_t * b, stream_t * c)
{
  #pragma omp parallel
  {
    int const tid = omp_get_thread_num();
    int const nthreads = omp_get_num_threads();
    size_t const chunk = STREAM_ARRAY_SIZE / nthreads;

    stream_t * const zfill_limit = a + (tid+1)*chunk - ZFILL_OFFSET;

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const aj = a + j;
      stream_t const * restrict const bj = b + j;
      stream_t const * restrict const cj = c + j;

      if (aj+ZFILL_OFFSET < zfill_limit) {
        zfill(aj+ZFILL_OFFSET);
      }

      for (int i=0; i<ELEM_PER_CACHE_LINE; ++i) {
        aj[i] = bj[i] + scalar*cj[i];
      }
    }

  } // parallel
} 
