// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TILE_ACTOR_H__
#define __TILE_ACTOR_H__

#include "Actor.h"

namespace aga
{
    class TileActor : public Actor
    {
    public:
        static std::string TypeName;

    public:
        TileActor (SceneManager* sceneManager);

        virtual bool Update (float deltaTime) override;
        virtual void Render (float deltaTime) override;

        virtual std::string GetTypeName () override;

        std::string Tileset;

    private:
        bool operator== (const TileActor& rhs) const
        {
            return Tileset == rhs.Tileset && Name == rhs.Name && Bounds == rhs.Bounds && Rotation == rhs.Rotation;
        }
    };
}

#endif //   __TILE_ACTOR_H__
