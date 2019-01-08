// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __AUDIO_SAMPLE_COMPONENT_H__
#define __AUDIO_SAMPLE_COMPONENT_H__

#include "Component.h"

namespace aga
{
    class AudioSample;

    class AudioSampleComponent : public Component
    {
    public:
        static std::string TypeName;

    public:
        AudioSampleComponent (Actor* owner);
        AudioSampleComponent (const AudioSampleComponent& rhs);
        
        virtual AudioSampleComponent* Clone () const override;
        
        virtual bool Update (float deltaTime) override;

        virtual bool Render (float deltaTime) override;

        void LoadSampleFromFile (const std::string& sampleName, const std::string& path);

        AudioSample* GetAudioSample () { return m_Sample; }

        virtual std::string GetTypeName () override { return TypeName; }

    private:
        AudioSample* m_Sample;
    };
}

#endif //   __AUDIO_SAMPLE_COMPONENT_H__


