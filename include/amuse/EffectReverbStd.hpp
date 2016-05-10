#ifndef __AMUSE_EFFECTREVERBSTD_HPP__
#define __AMUSE_EFFECTREVERBSTD_HPP__

#include "EffectBase.hpp"

namespace amuse
{

/** Reverb effect with configurable reflection filtering */
template <typename T>
class EffectReverbStd : public EffectBase<T>
{
    float m_coloration; /**< [0.0, 1.0] influences filter coefficients to define surface characteristics of a room */
    float m_mix; /**< [0.0, 1.0] dry/wet mix factor of reverb effect */
    float m_time; /**< [0.01, 10.0] time in seconds for reflection decay */
    float m_damping; /**< [0.0, 1.0] damping factor influencing low-pass filter of reflections */
    float m_preDelay; /**< [0.0, 0.1] time in seconds before initial reflection heard */
public:
    EffectReverbStd(float coloration, float mix, float time,
                    float damping, float preDelay);
    void applyEffect(int16_t* audio, size_t frameCount,
                     const ChannelMap& chanMap, double sampleRate);
    void applyEffect(int32_t* audio, size_t frameCount,
                     const ChannelMap& chanMap, double sampleRate);
    void applyEffect(float* audio, size_t frameCount,
                     const ChannelMap& chanMap, double sampleRate);
};

}

#endif // __AMUSE_EFFECTREVERBSTD_HPP__