// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __TRIANGULATOR_H__
#define __TRIANGULATOR_H__

#include <algorithm>
#include <queue>
#include <vector>

#include "Point.h"

namespace aga
{
    int GetQuadrant (const Point& p);
    float GetClockwiseAngle (const Point& p);
    bool ComparePoints (const Point& a, const Point& b);

    class Triangulator
    {
    public:
        Triangulator () {}

        /**
         * Separates a non-convex polygon into convex polygons and adds them as fixtures to the <code>body</code>
         *parameter.<br/> There are some rules you should follow (otherwise you might get unexpected results) : <ul>
         * <li>The vertices must be in clockwise order.</li>
         * <li>No three neighbouring points should lie on the same line segment.</li>
         * <li>There must be no overlapping segments and no "holes".</li>
         * </ul> <p/>
         * @param body The b2Body, in which the new fixtures will be stored.
         * @param fixtureDef A b2FixtureDef, containing all the properties (friction, density, etc.) which the new
         *fixtures will inherit.
         * @param verticesVec The vertices of the non-convex polygon, in clockwise order.
         * @see b2PolygonShape
         * @see b2PolygonShape.SetAsArray()
         * @see b2PolygonShape.SetAsVector()
         **/

        void ProcessVertices (std::vector<Point>* vertices, std::vector<std::vector<Point>>& out);

        /**
         * Checks whether the vertices in <code>verticesVec</code> can be properly distributed into the new fixtures
         * (more specifically, it makes sure there are no overlapping segments and the vertices are in clockwise order).
         * It is recommended that you use this method for debugging only, because it may cost more CPU usage.
         * <p/>
         * @param verticesVec The vertices to be validated.
         * @return An integer which can have the following values:
         * <ul>
         * <li>0 if the vertices can be properly processed.</li>
         * <li>1 If there are overlapping lines.</li>
         * <li>2 if the points are <b>not</b> in clockwise order.</li>
         * <li>3 if there are overlapping lines <b>and</b> the points are <b>not</b> in clockwise order.</li>
         * </ul>
         * */

        int Validate (std::vector<Point>& verticesVec);

    private:
        Point* HitRay (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
        Point* HitSegment (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
        bool IsOnSegment (float px, float py, float x1, float y1, float x2, float y2);
        bool PointsMatch (float x1, float y1, float x2, float y2);
        bool IsOnLine (float px, float py, float x1, float y1, float x2, float y2);
        float Det (float x1, float y1, float x2, float y2, float x3, float y3);
    };
}

#endif //   __TRIANGULATOR_H__
