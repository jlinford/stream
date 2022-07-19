# Stream

This is a refactored version of the STREAM benchmark that works well across multi-core CPUs, especially for Arm.
Out of the box, it should demonstrate good bandwidth on most CPUs based on the Arm Neoverse cores.
The [Makefile](Makefile) includes optimal compiler options for GNU, ACfL, and Fujitsu compilers.
It also includes some tuned kernels for the Fujitsu A64FX that demonstrate how to use the `DC ZVA` instructions
for cache line management and how to use Arm C Language Extensions.

See [README.original](README.original) for the original STREAM readme.

