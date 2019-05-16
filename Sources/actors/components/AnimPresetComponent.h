// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __ANIMPRESET_COMPONENT_H__
#define __ANIMPRESET_COMPONENT_H__

#include "Component.h"

namespace aga
{
	enum AnimPresetType
	{
		NoAnim,
		JumpInPlace
	};

	class AnimPresetComponent : public Component
	{
	public:
		static std::string TypeName;

	public:
		AnimPresetComponent (Actor* owner);
		AnimPresetComponent (const AnimPresetComponent& rhs);

		virtual AnimPresetComponent* Clone () const override;

		virtual bool Update (float deltaTime) override;

		virtual bool Render (float deltaTime) override;

		void SetAnimPresetType (AnimPresetType type);
		AnimPresetType GetAnimPresetType () const;
        
        void SetJumpInPlaceMaxHeight (int heightInPixels);
        void SetJumpInPlaceSpeed (float speed);
		void SetMaxJumpTimes (int times);
        
        void Play ();

		virtual std::string GetTypeName () override;

	private:
	private:
		AnimPresetType m_AnimPresetType;
        
        //  JumpInPlace        
		Point m_StartPos;
        Point m_EndPos;
        int m_MaxHeight;
        float m_JumpSpeed;
        bool m_GoingUp;
		int m_MaxJumpTimes;
		int m_CurrentJumpTimes;
	};
}

#endif //   __ANIMPRESET_COMPONENT_H__
