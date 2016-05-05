#ifndef __AMUSE_IBACKENDVOICE_HPP__
#define __AMUSE_IBACKENDVOICE_HPP__

namespace amuse
{

/** Same channel enums from boo, used for matrix coefficient table index */
enum class AudioChannel
{
    FrontLeft,
    FrontRight,
    RearLeft,
    RearRight,
    FrontCenter,
    LFE,
    SideLeft,
    SideRight,
    Unknown = 0xff
};

/**
 * @brief Client-implemented voice instance
 */
class IBackendVoice
{
public:
    virtual ~IBackendVoice() = default;

    /** Set channel-gains for audio source (AudioChannel enum for array index) */
    virtual void setMatrixCoefficients(const float coefs[8])=0;

    /** Called by client to dynamically adjust the pitch of voices with dynamic pitch enabled */
    virtual void setPitchRatio(double ratio)=0;

    /** Instructs platform to begin consuming sample data; invoking callback as needed */
    virtual void start()=0;

    /** Instructs platform to stop consuming sample data */
    virtual void stop()=0;
};

}

#endif // __AMUSE_IBACKENDVOICE_HPP__