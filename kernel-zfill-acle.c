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
#include <arm_sve.h>

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

    svbool_t ptrue = svptrue_b64();

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const cj = c + j;
      stream_t const * restrict const aj = a + j;

      if (cj+ZFILL_OFFSET < zfill_limit) {
        zfill(cj+ZFILL_OFFSET);
      }

      svst1(ptrue, cj,    svld1(ptrue, aj));
      svst1(ptrue, cj+8,  svld1(ptrue, aj+8));
      svst1(ptrue, cj+16, svld1(ptrue, aj+16));
      svst1(ptrue, cj+24, svld1(ptrue, aj+24));
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

    svbool_t ptrue = svptrue_b64();
    svfloat64_t const vscalar = svdup_f64(scalar);

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const bj = b + j;
      stream_t const * restrict const cj = c + j;

      if (bj+ZFILL_OFFSET < zfill_limit) {
        zfill(bj+ZFILL_OFFSET);
      }

      svfloat64_t vc0 = svld1(ptrue, cj);
      svfloat64_t vc1 = svld1(ptrue, cj+8);
      svfloat64_t vc2 = svld1(ptrue, cj+16);
      svfloat64_t vc3 = svld1(ptrue, cj+24);

      svst1(ptrue, bj,    svmul_f64_x(ptrue, vscalar, vc0));
      svst1(ptrue, bj+8,  svmul_f64_x(ptrue, vscalar, vc1));
      svst1(ptrue, bj+16, svmul_f64_x(ptrue, vscalar, vc2));
      svst1(ptrue, bj+24, svmul_f64_x(ptrue, vscalar, vc3));
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

    svbool_t ptrue = svptrue_b64();

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const cj = c + j;
      stream_t const * restrict const aj = a + j;
      stream_t const * restrict const bj = b + j;

      if (cj+ZFILL_OFFSET < zfill_limit) {
        zfill(cj+ZFILL_OFFSET);
      }

      svfloat64_t va0 = svld1(ptrue, aj);
      svfloat64_t va1 = svld1(ptrue, aj+8);
      svfloat64_t va2 = svld1(ptrue, aj+16);
      svfloat64_t va3 = svld1(ptrue, aj+24);

      svfloat64_t vb0 = svld1(ptrue, bj);
      svfloat64_t vb1 = svld1(ptrue, bj+8);
      svfloat64_t vb2 = svld1(ptrue, bj+16);
      svfloat64_t vb3 = svld1(ptrue, bj+24);

      svst1(ptrue, cj,    svadd_f64_x(ptrue, va0, vb0));
      svst1(ptrue, cj+8,  svadd_f64_x(ptrue, va1, vb1));
      svst1(ptrue, cj+16, svadd_f64_x(ptrue, va2, vb2));
      svst1(ptrue, cj+24, svadd_f64_x(ptrue, va3, vb3));
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

    svbool_t ptrue = svptrue_b64();
    svfloat64_t const vscalar = svdup_f64(scalar);

    #pragma omp for schedule(static)
    for (size_t j=0; j<STREAM_ARRAY_SIZE; j+=ELEM_PER_CACHE_LINE) {
      stream_t * restrict const aj = a + j;
      stream_t const * restrict const bj = b + j;
      stream_t const * restrict const cj = c + j;

      if (aj+ZFILL_OFFSET < zfill_limit) {
        zfill(aj+ZFILL_OFFSET);
      }

      svfloat64_t vb0 = svld1(ptrue, bj);
      svfloat64_t vb1 = svld1(ptrue, bj+8);
      svfloat64_t vb2 = svld1(ptrue, bj+16);
      svfloat64_t vb3 = svld1(ptrue, bj+24);

      svfloat64_t vc0 = svld1(ptrue, cj);
      svfloat64_t vc1 = svld1(ptrue, cj+8);
      svfloat64_t vc2 = svld1(ptrue, cj+16);
      svfloat64_t vc3 = svld1(ptrue, cj+24);

      svst1(ptrue, aj,    svmla_f64_x(ptrue, vb0, vscalar, vc0));
      svst1(ptrue, aj+8,  svmla_f64_x(ptrue, vb1, vscalar, vc1));
      svst1(ptrue, aj+16, svmla_f64_x(ptrue, vb2, vscalar, vc2));
      svst1(ptrue, aj+24, svmla_f64_x(ptrue, vb3, vscalar, vc3));
    }

  } // parallel
} 
