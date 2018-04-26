#ifndef TORUS_UTILS_BIG_H
#define TORUS_UTILS_BIG_H

#include "big_int.h"
#include "big_torus.h"

template<>
class TorusUtils<BigTorus> {
protected:
    using TORUS = BigTorus;

public:
    typedef BigInt INT_TYPE;

    /**
     * @brief Converts real number to torus (mainly for printing)
     * @param d real to convert
     * @return torus value
     */
    static void from_double(TORUS *res, const double d, const ZModuleParams<TORUS> *params) {
        tfhe_backend::from_double(res, d, params);
    }

    /**
     * @brief Converts torus to real number (mainly for printing)
     * @param x torus element to convert
     * @return real number
    */
    static double to_double(const TORUS *x, const ZModuleParams<TORUS> *params) {
        return tfhe_backend::to_double(x, params);
    }

    /**
     * @brief Rounds the torus value to the nearest multiple of 1/Msize
     * @param phase torus value
     * @param Msize discrete space size
     * @return approximated torus value
     */
    static void approxPhase(TORUS *res, const TORUS *phase, const uint64_t Msize, const ZModuleParams<TORUS> *params,
                            Allocator alloc) {
        tfhe_backend::approxPhase(res, phase, Msize, params, std::move(alloc));
    }

    /**
     * @brief Mod-Rescale from the torus to Z/Msize.Z
     *  i.e. computes roundToNearestInteger(Msize*phase)
     * @param phase torus value to convert
     * @param Msize discrete space size
     * @return discrete space value in [0, MSize[
     */
    static uint64_t
    modSwitchFromTorus(const TORUS *phase, const uint64_t Msize, const ZModuleParams<TORUS> *params, Allocator alloc) {
        return tfhe_backend::modSwitchFromTorus(phase, Msize, params, std::move(alloc));
    }

    /**
     * @brief Converts discrete message space to torus
     *  i.e. value mu/Msize to a torus for mu
     * @param mu discrete space value (from [0,Msize[) to convert
     * @param Msize discrete space size
     * @return torus value
     */
    static void
    modSwitchToTorus(TORUS *res, const uint64_t mu, const uint64_t Msize, const ZModuleParams<TORUS> *params,
                     Allocator alloc) {
        tfhe_backend::modSwitchToTorus(res, mu, Msize, params, std::move(alloc));
    }

    /**
     * @brief Return absolute distance between 2 torus elements
     *
     * @param t1 first torus element
     * @param t2 second torus element
     * @return double value of the infinity norm
     */
    static double distance(const TORUS *t1, const TORUS *t2, const ZModuleParams<TORUS> *params, Allocator alloc) {
        TORUS *t = alloc.newObject<TORUS>(params, &alloc);
        tfhe_backend::sub(t, t1, t2, params);
        double diff = std::abs(TorusUtils<TORUS>::to_double(t, params));
        alloc.deleteObject(t, params, &alloc);
        return diff;
    }
};

#endif // TORUS_UTILS_BIG_H
