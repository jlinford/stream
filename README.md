# Stream

This is a refactored version of the STREAM benchmark that works well across multi-core CPUs, especially for Arm.
Out of the box, it should demonstrate good bandwidth on most CPUs based on the Arm Neoverse cores.

STREAM is a defacto standard for measuring memory bandwidth.
The benchmark includes four kernels that operate on 1D arrays `a`, `b`, and `c`, with scalar `x`: 
 * **COPY**: `c = a`
 * **SCALE**: `b = x*a`
 * **ADD**: `c = a + b`
 * **TRIAD**: `a = b + x*c`

The kernels are executed in sequence in a loop.  Two parameters configure STREAM:
 * `STREAM_ARRAY_SIZE`: The number of double-precision elements in each array.
   It is critical to select a sufficiently large array size when measuring 
   bandwidth to/from main memory.
 * `NTIMES`: The number of iterations of the test loop.

See *[README.original](README.original)* for more details on STREAM.

This implementation is restructured to demonstrate the performance benefits of OpenMP,
effective use of NUMA, and features of the Arm architecture.  It uses a version of
stream that has been modified to enable dynamic memory allocation and to separate 
the kernel implementation from the benchmark driver.  This makes it the code easier
to read and faciliatates the use of external tools to measure the performance in 
each kernel.

## stream_vanilla

This is a basic, untuned, out-of-box, "vanilla" implementation of STREAM. 
Performance will most likely be very poor since it uses only a single core and 
does not consider NUMA or any architectural features. 

gcc 11 on the Fujitsu A64FX:
```
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:           40859.3     0.003931     0.003916     0.003981
Scale:          40796.5     0.003931     0.003922     0.003949
Add:            47235.1     0.005109     0.005081     0.005188
Triad:          47253.3     0.005096     0.005079     0.005114
-------------------------------------------------------------
```

## stream_openmp

A moderately tuned version of STREAM that uses OpenMP to span multiple threads 
and numactl to improve memory/thread locality.  On many systems, this implementation 
of STREAM will be close to 80% of the theoretical peak bandwidth.

gcc 11 on the Fujitsu A64FX:
```
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:          537948.0     0.032011     0.031936     0.032123
Scale:         537695.1     0.032026     0.031951     0.032179
Add:           597172.3     0.043259     0.043153     0.043500
Triad:         597324.1     0.043282     0.043142     0.044186
-------------------------------------------------------------
```

## stream_zfill

An optimized version of STREAM that uses Arm's `DC ZVA` instruction to zero-fill
cache lines.  `DC ZVA` isn't a prefetch instruction, but rather a way to map cache 
lines to virtual memory without having to read main memory.  Many architectures 
(not just Arm's) load a whole cache line from main memory when any address in the 
line is written to.  This is so that when the cache line is flushed the addresses 
adjacent to the written address retain their values.  On Arm, if you know for certain 
that the entire cache line will be written before it is flushed (or you you want to 
write zeros to main memory in any case), you can use the "DC ZVA" instruction to map 
the cache line to the appropriate virtual address without first reading main memory.  
All values in the cache line will be set to zero.  

This optimization can dramatically improve the performance of systems with wide L2 
cache lines and/or very little available L3 cache.  The Fujitsu A64FX has no L3, and 
each L2 cache is 256 bytes, so a "useless" load of the destination array from main 
memory has a much higher penalty on A64FX.

gcc 11 on the Fujitsu A64FX:
```
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:          780579.1     0.022083     0.022009     0.022202
Scale:         780689.0     0.022146     0.022006     0.022576
Add:           788330.3     0.032902     0.032689     0.033698
Triad:         787974.0     0.032808     0.032704     0.033263
-------------------------------------------------------------
```

## stream_zfill_acle

Further optimization of the stream_zfill vesion that uses SVE intrinsics via the 
Arm C Language Extensions (ACLE).  The memory bandwidth performance of this version 
does not improve, but the number of front-end bound cyles is reduced due to more 
effective vectorization of the inner loop.  Since STREAM is backend-bound on main
memory, there is no gain.

Arm Compiler for Linux 20.3 on Fujitsu A64FX:
```
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:          729261.4     0.023643     0.023558     0.023709
Scale:         707515.2     0.024440     0.024282     0.025103
Add:           768215.5     0.033778     0.033545     0.034639
Triad:         781942.8     0.033090     0.032956     0.033276
-------------------------------------------------------------
```

Contact: John Linford <john@redhpc.com>
