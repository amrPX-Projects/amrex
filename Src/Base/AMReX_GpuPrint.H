#ifndef AMREX_GPU_PRINT_H_
#define AMREX_GPU_PRINT_H_
#include <AMReX_Config.H>

#if !defined(__APPLE__)

#include <cstdio>

#ifdef AMREX_USE_DPCPP
#  include <sycl/sycl.hpp>
#endif

#  define AMREX_DEVICE_PRINTF(...) std::printf(__VA_ARGS__);

#endif  // !defined(__APPLE__)
#endif  // AMREX_GPU_PRINT_H_
