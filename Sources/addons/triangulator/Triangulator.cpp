// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Triangulator.h"

namespace aga
{
#define MAX_VALUE 2147483647

    //--------------------------------------------------------------------------------------------------

    void Triangulator::ProcessVertices (std::vector<Point>* vertices, std::vector<std::vector<Point>>& result)
    {
        std::vector<Point> vec;
        int i, n, j, minLen;
        float d, t, dx, dy;
        int i1, i2, i3;
        Point p1, p2, p3;
        int j1, j2;
        Point v1, v2;
        int k = 0, h = 0;
        std::vector<Point>*vec1, *vec2;
        Point *pV, hitV (0, 0);
        bool isConvex;
        std::vector<std::vector<Point>> figsVec;
        std::queue<std::vector<Point>> queue;

        queue.push (*vertices);

        while (!queue.empty ())
        {
            vec = queue.front ();
            n = vec.size ();
            isConvex = true;

            for (i = 0; i < n; ++i)
            {
                i1 = i;
                i2 = (i < n - 1) ? i + 1 : i + 1 - n;
                i3 = (i < n - 2) ? i + 2 : i + 2 - n;

                p1 = vec[i1];
                p2 = vec[i2];
                p3 = vec[i3];

                d = Det (p1.X, p1.Y, p2.X, p2.Y, p3.X, p3.Y);

                if (d < 0)
                {
                    isConvex = false;
                    minLen = MAX_VALUE;

                    for (j = 0; j < n; j++)
                    {
                        if (j != i1 && j != i2)
                        {
                            j1 = j;
                            j2 = (j < n - 1) ? j + 1 : 0;

                            v1 = vec[j1];
                            v2 = vec[j2];

                            pV = HitRay (p1.X, p1.Y, p2.X, p2.Y, v1.X, v1.Y, v2.X, v2.Y);

                            if (pV)
                            {
                                Point v = *pV;
                                dx = p2.X - v.X;
                                dy = p2.Y - v.Y;
                                t = dx * dx + dy * dy;

                                if ((t < minLen))
                                {
                                    h = j1;
                                    k = j2;
                                    hitV = v;
                                    minLen = t;
                                }
                            }
                        }
                    }

                    if (minLen == MAX_VALUE)
                    {
                        // TODO: Throw Error !!!
                    }

                    vec1 = new std::vector<Point> ();
                    vec2 = new std::vector<Point> ();

                    j1 = h;
                    j2 = k;
                    v1 = vec[j1];
                    v2 = vec[j2];

                    if (!PointsMatch (hitV.X, hitV.Y, v2.X, v2.Y))
                    {
                        vec1->push_back (hitV);
                    }
                    if (!PointsMatch (hitV.X, hitV.Y, v1.X, v1.Y))
                    {
                        vec2->push_back (hitV);
                    }

                    h = -1;
                    k = i1;
                    while (true)
                    {
                        if (k != j2)
                        {
                            vec1->push_back (vec[k]);
                        }
                        else
                        {
                            if (h < 0 || h >= n)
                            {
                                // TODO: Throw Error !!!
                            }
                            if (!IsOnSegment (v2.X, v2.Y, vec[h].X, vec[h].Y, p1.X, p1.Y))
                            {
                                vec1->push_back (vec[k]);
                            }
                            break;
                        }

                        h = k;
                        if (k - 1 < 0)
                        {
                            k = n - 1;
                        }
                        else
                        {
                            k--;
                        }
                    }

                    std::reverse (vec1->begin (), vec1->end ());

                    h = -1;
                    k = i2;
                    while (true)
                    {
                        if (k != j1)
                        {
                            vec2->push_back (vec[k]);
                        }
                        else
                        {
                            if (h < 0 || h >= n)
                            {
                                // TODO: Throw Error !!!
                            }
                            if (((k == j1) && !IsOnSegment (v1.X, v1.Y, vec[h].X, vec[h].Y, p2.X, p2.Y)))
                            {
                                vec2->push_back (vec[k]);
                            }
                            break;
                        }

                        h = k;
                        if (((k + 1) > n - 1))
                        {
                            k = 0;
                        }
                        else
                        {
                            k++;
                        }
                    }

                    queue.push (*vec1);
                    queue.push (*vec2);
                    queue.pop ();

                    break;
                }
            }

            if (isConvex)
            {
                figsVec.push_back (queue.front ());
                queue.pop ();
            }
        }

        result = figsVec;
    }

    //--------------------------------------------------------------------------------------------------

    int Triangulator::Validate (std::vector<Point>& verticesVec)
    {
        int i, n = verticesVec.size (), ret = 0;
        float j, j2, i2, i3, d;
        bool fl, fl2 = false;

        for (i = 0; i < n; i++)
        {
            i2 = (i < n - 1) ? i + 1 : 0;
            i3 = (i > 0) ? i - 1 : n - 1;

            fl = false;
            for (j = 0; j < n; j++)
            {
                if ((j != i) && (j != i2))
                {
                    if (!fl)
                    {
                        d = Det (
                          verticesVec[i].X, verticesVec[i].Y, verticesVec[i2].X, verticesVec[i2].Y, verticesVec[j].X, verticesVec[j].Y);
                        if ((d > 0))
                        {
                            fl = true;
                        }
                    }

                    if ((j != i3))
                    {
                        j2 = (j < n - 1) ? j + 1 : 0;
                        if (HitSegment (verticesVec[i].X,
                                        verticesVec[i].Y,
                                        verticesVec[i2].X,
                                        verticesVec[i2].Y,
                                        verticesVec[j].X,
                                        verticesVec[j].Y,
                                        verticesVec[j2].X,
                                        verticesVec[j2].Y))
                        {
                            ret = 1; // TODO: This may be wrong!!!
                        }
                    }
                }
            }

            if (!fl)
            {
                fl2 = true;
            }
        }

        if (fl2)
        {
            if (ret == 1)
            {
                ret = 3;
            }
            else
            {
                ret = 2;
            }
        }
        return ret;
    }

    //--------------------------------------------------------------------------------------------------

    Point* Triangulator::HitRay (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
    {
        float t1 = x3 - x1;
        float t2 = y3 - y1;
        float t3 = x2 - x1;
        float t4 = y2 - y1;
        float t5 = x4 - x3;
        float t6 = y4 - y3;
        float t7 = t4 * t5 - t3 * t6;

        // DBZ Error. Undefined hit segment.
        if (t7 == 0)
        {
            return nullptr;
        }

        float a = (((t5 * t2) - t6 * t1) / t7);
        float px = x1 + a * t3;
        float py = y1 + a * t4;
        bool b1 = IsOnSegment (x2, y2, x1, y1, px, py);
        bool b2 = IsOnSegment (px, py, x3, y3, x4, y4);

        if (b1 && b2)
        {
            return new Point (px, py);
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    Point* Triangulator::HitSegment (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
    {
        float t1 = x3 - x1;
        float t2 = y3 - y1;
        float t3 = x2 - x1;
        float t4 = y2 - y1;
        float t5 = x4 - x3;
        float t6 = y4 - y3;
        float t7 = t4 * t5 - t3 * t6;

        // DBZ Error. Undefined hit segment.
        if (t7 == 0)
        {
            return nullptr;
        }

        float a = (((t5 * t2) - t6 * t1) / t7);
        float px = x1 + a * t3;
        float py = y1 + a * t4;
        bool b1 = IsOnSegment (px, py, x1, y1, x2, y2);
        bool b2 = IsOnSegment (px, py, x3, y3, x4, y4);

        if (b1 && b2)
        {
            return new Point (px, py);
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    bool Triangulator::IsOnSegment (float px, float py, float x1, float y1, float x2, float y2)
    {
        bool b1 = ((x1 + 0.1 >= px && px >= x2 - 0.1) || (x1 - 0.1 <= px && px <= x2 + 0.1));
        bool b2 = ((y1 + 0.1 >= py && py >= y2 - 0.1) || (y1 - 0.1 <= py && py <= y2 + 0.1));

        return (b1 && b2 && IsOnLine (px, py, x1, y1, x2, y2));
    }

    //--------------------------------------------------------------------------------------------------

    bool Triangulator::PointsMatch (float x1, float y1, float x2, float y2)
    {
        float dx = (x2 >= x1) ? x2 - x1 : x1 - x2;
        float dy = (y2 >= y1) ? y2 - y1 : y1 - y2;
        return ((dx < 0.1f) && dy < 0.1f);
    }

    //--------------------------------------------------------------------------------------------------

    bool Triangulator::IsOnLine (float px, float py, float x1, float y1, float x2, float y2)
    {
        if (x2 - x1 > 0.1f || x1 - x2 > 0.1f)
        {
            float a = (y2 - y1) / (x2 - x1);
            float possibleY = a * (px - x1) + y1;
            float diff = (possibleY > py ? possibleY - py : py - possibleY);
            return (diff < 0.1f);
        }
        return (px - x1 < 0.1f || x1 - px < 0.1f);
    }

    //--------------------------------------------------------------------------------------------------

    float Triangulator::Det (float x1, float y1, float x2, float y2, float x3, float y3)
    {
        return x1 * y2 + x2 * y3 + x3 * y1 - y1 * x2 - y2 * x3 - y3 * x1;
    }

    //--------------------------------------------------------------------------------------------------
}
