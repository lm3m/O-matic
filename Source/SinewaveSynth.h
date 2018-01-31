/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/** A demo synth sound that's just a basic sine wave.. */
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};

int sgn(double value)
{
    return (value >= 0.0) ? 1 : -1;
};

double triangle(double value)
{
    double res = 0.0;
    
    if (value < 0.25) {
        res = value * 4;
    }
    else if (value < 0.75) {
        res = 2.0 - (value * 4.0);
    }
    else {
        res = value * 4 - 4.0;
    }
    
    return res;
};

//==============================================================================
/** A simple demo synth voice that just plays a sine wave.. */
class SineWaveVoice   : public SynthesiserVoice
{
private:
    double currentAngle;
    double angleDelta;
    double level;
    double tailOff;
    double frequency;
    double m_time;
    double m_deltaTime;
    double m_float;
    
public:
    SineWaveVoice()
       : currentAngle (0), angleDelta (0), level (0), tailOff (0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound* /*sound*/,
                    int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();
        frequency = cyclesPerSecond;
        angleDelta = cyclesPerSample * 2.0 * double_Pi;
        
        m_time = 0.0;
        m_deltaTime = 1.0 / getSampleRate();
        m_float = 0.0;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..

            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override
    {
        // not implemented for the purposes of this demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not implemented for the purposes of this demo!
    }
    
    double sampleByTime(double t)
    {
        double fullPeriodTime = 1.0 / frequency;
        double halfPeriodTime = fullPeriodTime / 2.0;
        double localTime = fmod(t, fullPeriodTime);
        
        if (localTime < halfPeriodTime)
        {
            return 1.0;
        }
        else{
            return -1.0;
        }
    }

    int sign(double value)
    {
        return (value >= 0.0) ? 1 : -1;
    }

    double sampleBySinSign(double t)
    {
        return sign(sin(2 * double_Pi * frequency * t));
    }
    
    double saw(double t)
    {
        double fullPeriodTime = 1.0 / frequency;
        double localTime = fmod(t, fullPeriodTime);
        
        return level * ((localTime / fullPeriodTime) * 2 - 1.0);
    }
    
    double triangle(double t)
    {
        double res = 0.0;
        double fullPeriodTime = 1.0 / frequency;
        double localTime = fmod(t, fullPeriodTime);
        
        double value = localTime / fullPeriodTime;
        
        if (value < 0.25) {
            res = value * 4;
        }
        else if (value < 0.75) {
            res = 2.0 - (value * 4.0);
        }
        else {
            res = value * 4 - 4.0;
        }
        
        return level * res;
    };
    
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    /*
                    const float currentSample4 = (float) (sgn(sin (currentAngle) * level * tailOff));
                    const float currentSample2 = (float) (sin (currentAngle) * level * tailOff);
                    const float currentSample3 = (float) ((fmod (currentAngle, 200)/100 - 1) * level * tailOff);
                    const float currentSample1 = (float) (triangle(sin(currentAngle)) * level * tailOff);
                    
                    const float currentSample = currentSample1; //(currentSample1 + currentSample2 + currentSample3 + currentSample4) / 4;
                    */

                    // square wave
                    m_float += .0000001;
                    const float currentSample = tailOff * level * sampleByTime(m_time) + m_float;

                    //const float currentSample = tailOff * level * sampleBySinSign(m_time);
                    
                    
                    // saw wave
                    //const float currentSample = tailOff * level * saw(m_time);
                    
                    // sine wave
                    //const float currentSample = tailOff * level * sin(2 * double_Pi * frequency * m_time);

                    // saw wave
                    //const float currentSample = tailOff * level * triangle(m_time);

                    
                    m_time += m_deltaTime;
                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        // tells the synth that this voice has stopped
                        clearCurrentNote();
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    //const float currentSample4 = (float) sgn(sin (currentAngle) * level);
                    //const float currentSample2 = (float) (sin (currentAngle) * level);
                    //const float currentSample3 = (float) ((fmod (currentAngle, 200)/100 - 1) * level);
                    //const float currentSample1 = (float) (triangle(sin(currentAngle)) * level);

                    //const float currentSample = currentSample1; //(currentSample1 + currentSample2 + currentSample3 + currentSample4) / 4;
                    
                    
                    // square wave
                    m_float += .0000001;
                    const float currentSample = level * sampleByTime(m_time) + m_float;

                    // const float currentSample = level * sampleBySinSign(m_time);
                    
                    // saw wave
                    //const float currentSample = level * saw(m_time);
                    
                    // sine wave
                    //const float currentSample = level * sin(2 * double_Pi * frequency * m_time);

                    // saw wave
                    //const float currentSample = level * triangle(m_time);

                    m_time += m_deltaTime;
                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;
                }
            }
        }
    }
};
