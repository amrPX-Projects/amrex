#ifndef AMREX_GPU_RANGE_H_
#define AMREX_GPU_RANGE_H_
#include <AMReX_Config.H>

#include <AMReX_GpuQualifiers.H>
#include <AMReX_GpuControl.H>
#include <AMReX_GpuError.H>
#include <AMReX_Box.H>
#include <AMReX_TypeTraits.H>

#include <nv/target>

namespace amrex {

template <typename T, typename std::enable_if<std::is_integral<T>::value,int>::type = 0>
bool isEmpty (T n) noexcept { return n <= 0; }

AMREX_FORCE_INLINE bool isEmpty (Box const& b) noexcept { return b.isEmpty(); }

namespace Gpu {

namespace range_detail {

//! integer version
template <typename T, typename std::enable_if<std::is_integral<T>::value,int>::type = 0>
AMREX_GPU_HOST_DEVICE
Long size (T const& b) noexcept { return static_cast<Long>(b); }

template <typename T, typename std::enable_if<std::is_integral<T>::value,int>::type = 0>
AMREX_GPU_HOST_DEVICE
Long at (T const& /*b*/, Long offset) noexcept { return offset; }

//! Box version
AMREX_GPU_HOST_DEVICE
AMREX_FORCE_INLINE Long size (Box const& b) noexcept
{
if target (nv::target::is_device) {
    return b.numPts();
}
if target (nv::target::is_host) {
    amrex::ignore_unused(b);
    return 1;
}
}

AMREX_GPU_HOST_DEVICE
AMREX_FORCE_INLINE Box at (Box const& b, Long offset) noexcept
{
if target (nv::target::is_device) {
    auto len = b.length3d();
    Long k = offset / (len[0]*len[1]);
    Long j = (offset - k*(len[0]*len[1])) / len[0];
    Long i = (offset - k*(len[0]*len[1])) - j*len[0];
    IntVect iv{AMREX_D_DECL(static_cast<int>(i),
                            static_cast<int>(j),
                            static_cast<int>(k))};
    iv += b.smallEnd();
    return Box(iv,iv,b.type());
}
if target (nv::target::is_host) {
    amrex::ignore_unused(offset);
    return b;
}
}

template <typename T>
struct range_impl
{
    AMREX_GPU_HOST_DEVICE
    explicit range_impl (T const& b) noexcept : m_b(b), m_n(range_detail::size(b)) {}

#ifdef AMREX_USE_DPCPP
    range_impl (T const& b, Long gid, Long grange) noexcept
        : m_b(b), m_n(range_detail::size(b)), m_gid(gid), m_grange(grange) {}
#endif

    struct iterator
    {
        AMREX_GPU_HOST_DEVICE
        iterator (T const& b, Long i, Long s) noexcept : mi_b(b), mi_i(i), mi_s(s)  {}

        AMREX_GPU_HOST_DEVICE
        void operator++ () noexcept { mi_i += mi_s; }

        AMREX_GPU_HOST_DEVICE
        bool operator!= (iterator const& rhs) const noexcept { return mi_i < rhs.mi_i; }

        AMREX_GPU_HOST_DEVICE
        T operator* () const noexcept { return range_detail::at(mi_b,mi_i); }

    private:
        T const& mi_b;
        Long mi_i;
        Long mi_s;
    };

    AMREX_GPU_HOST_DEVICE
    iterator begin () const noexcept {
if target (nv::target::is_device) {
        return iterator(m_b, m_gid, m_grange);
}
if target (nv::target::is_host) {
        return iterator(m_b,0,1);
}
    }

    AMREX_GPU_HOST_DEVICE
    iterator end () const noexcept { return iterator(m_b,m_n,0); }

private:
    T m_b;
    Long m_n;
#ifdef AMREX_USE_DPCPP
    Long m_gid;
    Long m_grange;
#endif
};
}

#ifdef AMREX_USE_DPCPP
template <typename T>
range_detail::range_impl<T> Range (T const& b, Long gid, Long grange) noexcept {
    return range_detail::range_impl<T>(b,gid,grange);
}
#endif

template <typename T>
AMREX_GPU_HOST_DEVICE
range_detail::range_impl<T> Range (T const& b) noexcept { return range_detail::range_impl<T>(b); }

}}

#endif
