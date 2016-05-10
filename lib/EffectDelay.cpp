#include "amuse/EffectDelay.hpp"
#include "amuse/Common.hpp"
#include "amuse/IBackendVoice.hpp"
#include <string.h>

namespace amuse
{

template <typename T>
EffectDelay<T>::EffectDelay(uint32_t initDelay, uint32_t initFeedback,
                            uint32_t initOutput, double sampleRate)
: m_sampsPerMs(std::ceil(sampleRate / 1000.0)),
  m_blockSamples(m_sampsPerMs * 5)
{
    initDelay = clamp(10u, initDelay, 5000u);
    initFeedback = clamp(0u, initFeedback, 100u);
    initOutput = clamp(0u, initOutput, 100u);

    for (int i=0 ; i<8 ; ++i)
    {
        x3c_delay[i] = initDelay;
        x48_feedback[i] = initFeedback;
        x54_output[i] = initOutput;
    }

    _update();
}

template <typename T>
void EffectDelay<T>::_update()
{
    for (int i=0 ; i<8 ; ++i)
    {
        x0_currentSize[i] = ((x3c_delay[i] - 5) * m_sampsPerMs + 159) / 160;
        xc_currentPos[i] = 0;
        x18_currentFeedback[i] = x48_feedback[i] * 128 / 100;
        x24_currentOutput[i] = x54_output[i] * 128 / 100;

        x30_chanLines[i].reset(new T[m_blockSamples * x0_currentSize[i]]);
        memset(x30_chanLines[i], 0, m_blockSamples * x0_currentSize[i] * sizeof(T));
    }

    m_dirty = false;
}

template <typename T>
void EffectDelay<T>::applyEffect(T* audio, size_t frameCount, const ChannelMap& chanMap)
{
    if (m_dirty)
        _update();

    for (size_t f=0 ; f<frameCount ;)
    {
        for (int c=0 ; c<chanMap.m_channelCount ; ++c)
        {
            T* chanAud = audio + c;
            for (int i=0 ; i<m_blockSamples && f<frameCount ; ++i, ++f)
            {
                T& liveSamp = chanAud[chanMap.m_channelCount * i];
                T& samp = x30_chanLines[c][xc_currentPos[c] * m_blockSamples + i];
                samp = ClampFull<T>(samp * x18_currentFeedback[c] / 128 + liveSamp);
                liveSamp = samp * x24_currentOutput[c] / 128;
            }
            xc_currentPos = (xc_currentPos[c] + 1) % x0_currentSize[c];
        }
        audio += chanMap.m_channelCount * m_blockSamples;
    }
}

}