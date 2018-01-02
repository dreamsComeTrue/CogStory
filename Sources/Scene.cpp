// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "Scene.h"
#include "ActorFactory.h"
#include "Atlas.h"
#include "AtlasManager.h"
#include "MainLoop.h"
#include "SceneManager.h"
#include "Screen.h"

#include "addons/triangulator/Triangulator.h"

using json = nlohmann::json;

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    /*
     *
     *  {
     *      "name": "Home",
     *      "flag_points" :
     *      [
     *          "SPAWN_1" : "0 0"
     *      ]
     *  }
     *
     */

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    int Entity::GlobalID = 0;

    //--------------------------------------------------------------------------------------------------

    void TriggerArea::UpdatePolygons (Triangulator* triangulator)
    {
        if (Points.size () > 2)
        {
            Polygons.clear ();

            int counter = 0;
            std::vector<std::vector<Point>> result;
            std::vector<Point> pointsCopy = Points;

            int validate = triangulator->Validate (pointsCopy);

            if (validate == 2)
            {
                std::reverse (pointsCopy.begin (), pointsCopy.end ());
            }

            if (triangulator->Validate (pointsCopy) == 0)
            {
                triangulator->ProcessVertices (&pointsCopy, result);

                for (int j = 0; j < result.size (); ++j)
                {
                    Polygons.push_back (Polygon ());
                    Polygons[counter].Points = result[j];
                    Polygons[counter].BuildEdges ();

                    ++counter;
                }
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    auto StringToVectorPoints = [](std::string in) -> std::vector<Point> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<Point> nums;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            float x = atof (pch);

            pch = strtok (NULL, delimiter);

            float y = atof (pch);

            nums.push_back ({ x, y });
        }

        return nums;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToVectorStrings = [](std::string in) -> std::vector<std::string> {
        size_t count = 0;
        const char* delimiter = " ";
        std::vector<std::string> strings;
        char* str = const_cast<char*> (in.c_str ());

        for (char* pch = strtok (str, delimiter); pch != NULL; pch = strtok (NULL, delimiter))
        {
            strings.push_back (pch);
        }

        return strings;
    };

    //--------------------------------------------------------------------------------------------------

    auto StringToPoint = [](std::string in) -> Point {
        std::vector<Point> vec = StringToVectorPoints (in);

        return vec[0];
    };

    //--------------------------------------------------------------------------------------------------

    auto PointToString = [](Point in) -> std::string { return ToString (in.X) + " " + ToString (in.Y); };

    //--------------------------------------------------------------------------------------------------

    auto VectorPointsToString = [](std::vector<Point>& points) -> std::string {
        std::string out;

        for (Point& p : points)
        {
            out += PointToString (p) + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------

    auto VectorStringsToString = [](std::vector<std::string>& strings) -> std::string {
        std::string out;

        for (std::string& s : strings)
        {
            out += s + " ";
        }

        return out;
    };

    //--------------------------------------------------------------------------------------------------

    void Tile::Draw (AtlasManager* atlasManager)
    {
        Atlas* atlas = atlasManager->GetAtlas (Tileset);

        if (atlas)
        {
            atlas->DrawRegion (Name, Bounds.GetPos ().X, Bounds.GetPos ().Y, 1, 1, DegressToRadians (Rotation));
        }
    }

    //--------------------------------------------------------------------------------------------------

    const float boundSize = 10000;

    Scene::Scene (SceneManager* sceneManager)
      : Scriptable (&sceneManager->GetMainLoop ()->GetScriptManager ())
      , m_SceneManager (sceneManager)
      , m_DrawPhysData (true)
      , m_QuadTree (Rect ({ -boundSize, -boundSize }, { boundSize, boundSize }))
    {
    }

    //--------------------------------------------------------------------------------------------------

    Scene::~Scene ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Initialize () { return Lifecycle::Initialize (); }

    //--------------------------------------------------------------------------------------------------

    bool Scene::Destroy ()
    {
        for (std::map<std::string, Actor*>::iterator it = m_Actors.begin (); it != m_Actors.end (); ++it)
        {
            SAFE_DELETE (it->second);
        }

        for (int i = 0; i < m_Tiles.size (); ++i)
        {
            SAFE_DELETE (m_Tiles[i]);
        }

        return Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    Scene* Scene::LoadScene (SceneManager* sceneManager, const std::string& filePath)
    {
        try
        {
            Scene* scene = new Scene (sceneManager);
            std::ifstream file (filePath.c_str ());
            json j;
            file >> j;
            file.close ();

            scene->m_Name = j["name"];
            scene->m_Size = Rect (StringToPoint (j["min_size"]), StringToPoint (j["max_size"]));
            scene->m_Size = Rect (Point (-1000, -1000), Point (1000, 1000));

            scene->m_QuadTree = QuadTreeNode (scene->m_Size);

            auto& scripts = j["scripts"];

            for (auto& j_tile : scripts)
            {
                std::string name = j_tile["name"];
                std::string path = j_tile["path"];

                Script* script = sceneManager->GetMainLoop ()->GetScriptManager ().LoadScriptFromFile (
                  GetDataPath () + "scripts/" + path, name);

                if (script)
                {
                    scene->AttachScript (script, path);
                }
            }

            auto& tiles = j["tiles"];

            for (auto& j_tile : tiles)
            {
                Tile* tile = new Tile (&sceneManager->GetMainLoop ()->GetPhysicsManager ());

                std::string id = j_tile["id"];
                tile->ID = atoi (id.c_str ());
                tile->Tileset = j_tile["tileset"];
                tile->Name = j_tile["name"];
                tile->Bounds.SetPos (StringToPoint (j_tile["pos"]));
                tile->Bounds.SetSize (
                  sceneManager->GetAtlasManager ()->GetAtlas (tile->Tileset)->GetRegion (tile->Name).Bounds.GetSize ());
                std::string zOrder = j_tile["z-order"];
                tile->ZOrder = atoi (zOrder.c_str ());
                std::string rot = j_tile["rot"];
                tile->Rotation = atof (rot.c_str ());

                //  Physics
                if (!j_tile["phys"].is_null ())
                {
                    auto& physTiles = j_tile["phys"];

                    for (auto& physTile : physTiles)
                    {
                        tile->PhysPoints.push_back (StringToVectorPoints (physTile["poly"]));
                    }

                    tile->SetPhysOffset (tile->Bounds.GetPos ());
                }

                scene->AddTile (tile);
            }

            auto& flag_points = j["flag_points"];

            for (auto& flag_point : flag_points)
            {
                std::string name = flag_point["name"];
                Point pos = StringToPoint (flag_point["pos"]);

                scene->AddFlagPoint (name, pos);
            }

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end ();
                 ++it)
            {
                for (auto& flag_point : flag_points)
                {
                    std::string name = flag_point["name"];

                    if (name == it->first)
                    {
                        std::vector<std::string> connections = StringToVectorStrings (flag_point["connections"]);

                        if (!connections.empty ())
                        {
                            for (std::map<std::string, FlagPoint>::iterator it2 = scene->m_FlagPoints.begin ();
                                 it2 != scene->m_FlagPoints.end ();
                                 ++it2)
                            {
                                for (int i = 0; i < connections.size (); ++i)
                                {
                                    if (connections[i] == it2->first)
                                    {
                                        it->second.Connections.push_back (&it2->second);
                                        break;
                                    }
                                }
                            }

                            break;
                        }
                    }
                }
            }

            auto& triggerAreas = j["trigger_areas"];

            for (auto& triggerArea : triggerAreas)
            {
                std::string name = triggerArea["name"];
                std::vector<Point> poly = StringToVectorPoints (triggerArea["poly"]);

                scene->AddTriggerArea (name, poly);
            }

            auto& speeches = j["speeches"];

            for (auto& speech : speeches)
            {
                SpeechData speechData;
                speechData.Name = speech["name"];

                auto& texts = speech["texts"];

                for (auto& text : texts)
                {
                    if (text["langID"] == "EN")
                    {
                        speechData.Text[LANG_EN] = text["data"];
                    }

                    if (text["langID"] == "PL")
                    {
                        speechData.Text[LANG_PL] = text["data"];
                    }
                }

                auto& outcomes = speech["outcomes"];

                for (auto& outcome : outcomes)
                {
                    auto& texts = outcome["texts"];

                    for (auto& text : texts)
                    {
                        SpeechOutcome out;
                        out.Name = text["name"];
                        out.Text = text["data"];
                        out.Action = text["action"];

                        if (outcome["langID"] == "EN")
                        {
                            speechData.Outcomes[LANG_EN].push_back (out);
                        }
                        else if (outcome["langID"] == "PL")
                        {
                            speechData.Outcomes[LANG_PL].push_back (out);
                        }
                    }
                }

                scene->AddSpeech (speechData.Name, speechData);
            }

            auto& actors = j["actors"];

            for (auto& actorIt : actors)
            {
                Actor* newActor = ActorFactory::GetActor (sceneManager, actorIt["type"]);

                if (newActor)
                {
                    scene->AddActor (actorIt["name"], newActor);

                    std::string id = actorIt["id"];
                    newActor->ID = atoi (id.c_str ());
                    newActor->Name = actorIt["name"];
                    newActor->Bounds.Pos = StringToPoint (actorIt["pos"]);
                    std::string zOrder = actorIt["z-order"];
                    newActor->ZOrder = atoi (zOrder.c_str ());
                    std::string rot = actorIt["rot"];
                    newActor->Rotation = atof (rot.c_str ());
                }
            }

            UpdateMaxTileID (scene);

            return scene;
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SaveScene (Scene* scene, const std::string& filePath)
    {
        Point minRect{ std::numeric_limits<float>::max (), std::numeric_limits<float>::max () };
        Point maxRect{ std::numeric_limits<float>::min (), std::numeric_limits<float>::min () };

        try
        {
            json j;

            j["name"] = scene->m_Name;

            j["scripts"] = json::array ({});

            for (std::vector<ScriptMetaData>::iterator it = scene->m_Scripts.begin (); it != scene->m_Scripts.end ();
                 ++it)
            {
                json scriptObj = json::object ({});

                scriptObj["name"] = it->Name;
                scriptObj["path"] = it->Path;

                j["scripts"].push_back (scriptObj);
            }

            j["tiles"] = json::array ({});

            for (Tile* tile : scene->m_Tiles)
            {
                if (tile->Bounds.GetTopLeft ().X < minRect.X)
                {
                    minRect.X = tile->Bounds.GetTopLeft ().X;
                }
                if (tile->Bounds.GetTopLeft ().Y < minRect.Y)
                {
                    minRect.Y = tile->Bounds.GetTopLeft ().Y;
                }
                if (tile->Bounds.GetBottomRight ().X > maxRect.X)
                {
                    maxRect.X = tile->Bounds.GetBottomRight ().X;
                }
                if (tile->Bounds.GetBottomRight ().Y > maxRect.Y)
                {
                    maxRect.Y = tile->Bounds.GetBottomRight ().Y;
                }

                json tileObj = json::object ({});

                tileObj["id"] = ToString (tile->ID);
                tileObj["tileset"] = tile->Tileset;
                tileObj["name"] = tile->Name;
                tileObj["pos"] = PointToString (tile->Bounds.GetPos ());
                tileObj["z-order"] = ToString (tile->ZOrder);
                tileObj["rot"] = ToString (tile->Rotation);

                tileObj["phys"] = json::array ({});

                for (int i = 0; i < tile->PhysPoints.size (); ++i)
                {
                    json physObj = json::object ({});

                    physObj["poly"] = VectorPointsToString (tile->PhysPoints[i]);

                    tileObj["phys"].push_back (physObj);
                }

                j["tiles"].push_back (tileObj);
            }

            j["min_size"] = PointToString (minRect);
            j["max_size"] = PointToString (maxRect);

            j["flag_points"] = json::array ({});

            for (std::map<std::string, FlagPoint>::iterator it = scene->m_FlagPoints.begin ();
                 it != scene->m_FlagPoints.end ();
                 ++it)
            {
                json flagObj = json::object ({});

                flagObj["name"] = it->first;
                flagObj["pos"] = PointToString (it->second.Pos);

                std::vector<std::string> connections;
                for (int i = 0; i < it->second.Connections.size (); ++i)
                {
                    connections.push_back (it->second.Connections[i]->Name);
                }

                flagObj["connections"] = VectorStringsToString (connections);

                j["flag_points"].push_back (flagObj);
            }

            j["trigger_areas"] = json::array ({});

            for (std::map<std::string, TriggerArea>::iterator it = scene->m_TriggerAreas.begin ();
                 it != scene->m_TriggerAreas.end ();
                 ++it)
            {
                json triggerObj = json::object ({});

                triggerObj["name"] = it->second.Name;
                triggerObj["poly"] = VectorPointsToString (it->second.Points);

                j["trigger_areas"].push_back (triggerObj);
            }

            j["speeches"] = json::array ({});

            for (std::map<std::string, SpeechData>::iterator it = scene->m_Speeches.begin ();
                 it != scene->m_Speeches.end ();
                 ++it)
            {
                json speechObj = json::object ({});

                speechObj["name"] = it->first;
                speechObj["texts"] = json::array ({});

                json textObj = json::object ({});
                textObj["langID"] = "EN";
                textObj["data"] = it->second.Text[LANG_EN];

                speechObj["texts"].push_back (textObj);

                textObj["langID"] = "PL";
                textObj["data"] = it->second.Text[LANG_PL];

                speechObj["texts"].push_back (textObj);

                speechObj["outcomes"] = json::array ({});

                //  EN - Outcome
                json outcomeObj = json::object ({});
                outcomeObj["langID"] = "EN";

                outcomeObj["texts"] = json::array ({});

                for (int i = 0; i < it->second.Outcomes[LANG_EN].size (); ++i)
                {
                    json outComeTextObj = json::object ({});
                    outComeTextObj["name"] = it->second.Outcomes[LANG_EN][i].Name;
                    outComeTextObj["data"] = it->second.Outcomes[LANG_EN][i].Text;
                    outComeTextObj["action"] = it->second.Outcomes[LANG_EN][i].Action;

                    outcomeObj["texts"].push_back (outComeTextObj);
                }

                speechObj["outcomes"].push_back (outcomeObj);

                //  PL - Outcome
                outcomeObj = json::object ({});
                outcomeObj["langID"] = "PL";

                outcomeObj["texts"] = json::array ({});

                for (int i = 0; i < it->second.Outcomes[LANG_PL].size (); ++i)
                {
                    json outComeTextObj = json::object ({});
                    outComeTextObj["name"] = it->second.Outcomes[LANG_PL][i].Name;
                    outComeTextObj["data"] = it->second.Outcomes[LANG_PL][i].Text;
                    outComeTextObj["action"] = it->second.Outcomes[LANG_PL][i].Action;

                    outcomeObj["texts"].push_back (outComeTextObj);
                }

                speechObj["outcomes"].push_back (outcomeObj);

                j["speeches"].push_back (speechObj);
            }

            j["actors"] = json::array ({});

            for (auto& actorIt : scene->m_Actors)
            {
                json actorObj = json::object ({});
                Actor* actor = actorIt.second;

                actorObj["id"] = ToString (actor->ID);
                actorObj["type"] = actor->GetTypeName ();
                actorObj["name"] = actor->Name;
                actorObj["pos"] = PointToString (actor->Bounds.GetPos ());
                actorObj["z-order"] = ToString (actor->ZOrder);
                actorObj["rot"] = ToString (actor->Rotation);

                j["actors"].push_back (actorObj);
            }

            // write prettified JSON to another file
            std::ofstream out (filePath);
            out << std::setw (4) << j.dump (4, ' ') << "\n";
        }
        catch (const std::exception&)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::BeforeEnter ()
    {
        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetPlayer ().BeforeEnter ();
            RunAllScripts ("void BeforeEnterScene ()");
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AfterLeave ()
    {
        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetPlayer ().AfterLeave ();
            RunAllScripts ("void AfterLeaveScene ()");
        }
    }

    //--------------------------------------------------------------------------------------------------

    typedef std::map<std::string, Actor*>::iterator ActorIterator;

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    void Scene::Update (float deltaTime)
    {
        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetPlayer ().Update (deltaTime);
            UpdateScripts (deltaTime);

            for (ActorIterator it = m_Actors.begin (); it != m_Actors.end (); ++it)
            {
                it->second->Update (deltaTime);
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Render (float deltaTime)
    {
        Font& font = m_SceneManager->GetMainLoop ()->GetScreen ()->GetFont ();

        m_SceneManager->GetCamera ().Update (deltaTime);

        if (m_DrawPhysData)
        {
            DrawQuadTree (&m_QuadTree);
        }

        bool isPlayerDrawn = false;
        std::vector<Entity*> tiles;

        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () == "EDITOR_STATE")
        {
            for (int i = 0; i < m_Tiles.size (); ++i)
            {
                Tile* tile = m_Tiles[i];

                if (!isPlayerDrawn && tile->ZOrder >= PLAYER_Z_ORDER)
                {
                    m_SceneManager->GetPlayer ().Render (deltaTime);
                    isPlayerDrawn = true;
                }

                tile->RenderID = i;
                tile->Draw (m_SceneManager->GetAtlasManager ());

                if (m_DrawPhysData)
                {
                    tile->DrawPhysVertices ();
                }
            }

            for (ActorIterator it = m_Actors.begin (); it != m_Actors.end (); ++it)
            {
                it->second->Render (deltaTime);

                Point pos = { it->second->Bounds.GetCenter ().X, it->second->Bounds.GetBottomRight ().Y };
                font.DrawText (
                  FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), pos.X, pos.Y, it->first, ALLEGRO_ALIGN_CENTER);
            }
        }
        else
        {
            tiles = GetVisibleEntities ();

            for (int i = 0; i < tiles.size (); ++i)
            {
                Tile* tile = (Tile*)tiles[i];

                if (!isPlayerDrawn && tile->ZOrder >= PLAYER_Z_ORDER)
                {
                    m_SceneManager->GetPlayer ().Render (deltaTime);
                    isPlayerDrawn = true;
                }

                tile->RenderID = i;
                tile->Draw (m_SceneManager->GetAtlasManager ());

                if (m_DrawPhysData)
                {
                    tile->DrawPhysVertices ();
                }
            }

            for (ActorIterator it = m_Actors.begin (); it != m_Actors.end (); ++it)
            {
                it->second->Render (deltaTime);
            }
        }

        if (!isPlayerDrawn)
        {
            m_SceneManager->GetPlayer ().Render (deltaTime);
        }

        if (m_DrawPhysData)
        {
            m_SceneManager->GetPlayer ().DrawPhysVertices ();
        }

        font.DrawText (FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), -100, -50, m_Name, ALLEGRO_ALIGN_LEFT);

        if (m_SceneManager->GetMainLoop ()->GetStateManager ().GetActiveStateName () != "EDITOR_STATE")
        {
            m_SceneManager->GetCamera ().UseIdentityTransform ();

            font.DrawText (
              FONT_NAME_MAIN_SMALL, al_map_rgb (0, 255, 0), 10, 10, ToString (tiles.size ()), ALLEGRO_ALIGN_LEFT);
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::vector<Entity*> Scene::GetVisibleEntities ()
    {
        Point pos = m_SceneManager->GetCamera ().GetTranslate ();
        Point size = m_SceneManager->GetMainLoop ()->GetScreen ()->GetWindowSize ();
        float offsetMultiplier = 0.5f;

        std::vector<Entity*> tiles =
          m_QuadTree.GetEntitiesWithinRect (Rect (pos - size * offsetMultiplier, pos + size + size * offsetMultiplier));

        std::sort (tiles.begin (), tiles.end (), Tile::CompareByZOrder);

        return tiles;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddActor (const std::string& name, Actor* actor)
    {
        if (m_Actors.find (name) == m_Actors.end ())
        {
            m_Actors.insert (make_pair (name, actor));
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveActor (const std::string& name)
    {
        if (m_Actors.find (name) != m_Actors.end ())
        {
            SAFE_DELETE (m_Actors[name]);
            m_Actors.erase (name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    Actor* Scene::GetActor (const std::string& name)
    {
        if (m_Actors.find (name) != m_Actors.end ())
        {
            return m_Actors[name];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, Actor*>& Scene::GetActors () { return m_Actors; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTile (Tile* tile)
    {
        m_Tiles.push_back (tile);
        m_QuadTree.Insert (tile);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTile (Tile* tile)
    {
        m_Tiles.erase (std::remove (m_Tiles.begin (), m_Tiles.end (), tile), m_Tiles.end ());
        m_QuadTree.Remove (tile);
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::SortTiles () { std::sort (m_Tiles.begin (), m_Tiles.end (), Tile::CompareByZOrder); }

    //--------------------------------------------------------------------------------------------------

    std::vector<Tile*>& Scene::GetTiles () { return m_Tiles; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddFlagPoint (const std::string& name, Point point)
    {
        if (m_FlagPoints.find (name) == m_FlagPoints.end ())
        {
            FlagPoint fp;
            fp.Name = name;
            fp.Pos = point;

            m_FlagPoints.insert (make_pair (name, fp));
        }
    }

    //--------------------------------------------------------------------------------------------------

    FlagPoint* Scene::GetFlagPoint (const std::string& name)
    {
        if (!m_FlagPoints.empty ())
        {
            std::map<std::string, FlagPoint>::iterator it = m_FlagPoints.find (name);

            if (it != m_FlagPoints.end ())
            {
                return &(*it).second;
            }
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, FlagPoint>& Scene::GetFlagPoints () { return m_FlagPoints; }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddTriggerArea (const std::string& name, std::vector<Point> points)
    {
        if (m_TriggerAreas.find (name) == m_TriggerAreas.end ())
        {
            TriggerArea area{ name, points };
            area.UpdatePolygons (&m_SceneManager->GetMainLoop ()->GetPhysicsManager ().GetTriangulator ());

            m_TriggerAreas.insert (std::make_pair (name, area));
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, TriggerArea>& Scene::GetTriggerAreas () { return m_TriggerAreas; }

    //--------------------------------------------------------------------------------------------------

    TriggerArea& Scene::GetTriggerArea (const std::string& name) { return m_TriggerAreas[name]; }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveTriggerArea (const std::string& name) { m_TriggerAreas.erase (name); }

    //--------------------------------------------------------------------------------------------------

    bool Scene::AddSpeech (const std::string& name, SpeechData data)
    {
        if (m_Speeches.find (name) == m_Speeches.end ())
        {
            m_Speeches.insert (std::make_pair (name, data));

            return true;
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------

    std::map<std::string, SpeechData>& Scene::GetSpeeches () { return m_Speeches; }

    //--------------------------------------------------------------------------------------------------

    SpeechData* Scene::GetSpeech (const std::string& name)
    {
        if (m_Speeches.find (name) != m_Speeches.end ())
        {
            return &m_Speeches[name];
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::RemoveSpeech (const std::string& name)
    {
        if (m_Speeches.find (name) != m_Speeches.end ())
        {
            m_Speeches.erase (name);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::Reset ()
    {
        RemoveAllScripts ();

        m_SceneManager->GetSpeechFrameManager ().Clear ();
        m_SceneManager->GetMainLoop ()->GetTweenManager ().Clear ();

        m_Tiles.clear ();
        m_FlagPoints.clear ();
        m_TriggerAreas.clear ();

        m_QuadTree = QuadTreeNode (Rect ({ -boundSize, -boundSize }, { boundSize, boundSize }));
    }

    //--------------------------------------------------------------------------------------------------

    std::string Scene::GetName () { return m_Name; }

    //--------------------------------------------------------------------------------------------------

    void Scene::SetDrawPhysData (bool enable) { m_DrawPhysData = enable; }

    //--------------------------------------------------------------------------------------------------

    bool Scene::IsDrawPhysData () { return m_DrawPhysData; }

    //--------------------------------------------------------------------------------------------------

    QuadTreeNode& Scene::GetQuadTree () { return m_QuadTree; }

    //--------------------------------------------------------------------------------------------------

    void Scene::UpdateMaxTileID (Scene* scene)
    {
        std::vector<Tile*>& tiles = scene->GetTiles ();
        int maxTileID = -1;

        for (Tile* t : tiles)
        {
            if (t->ID > maxTileID)
            {
                maxTileID = t->ID + 1;
            }
        }

        Entity::GlobalID = maxTileID;
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::DrawQuadTree (QuadTreeNode* node)
    {
        Rect bounds = node->GetBounds ();
        al_draw_rectangle (bounds.GetTopLeft ().X,
                           bounds.GetTopLeft ().Y,
                           bounds.GetBottomRight ().X,
                           bounds.GetBottomRight ().Y,
                           COLOR_WHITE,
                           1);

        if (node->GetTopLeftTree ())
        {
            DrawQuadTree (node->GetTopLeftTree ());
        }

        if (node->GetTopRightTree ())
        {
            DrawQuadTree (node->GetTopRightTree ());
        }

        if (node->GetBottomLeftTree ())
        {
            DrawQuadTree (node->GetBottomLeftTree ());
        }

        if (node->GetBottomRightTree ())
        {
            DrawQuadTree (node->GetBottomRightTree ());
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddOnEnterCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].OnEnterCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddOnEnterCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].ScriptOnEnterCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddOnLeaveCallback (const std::string& triggerName, std::function<void(float dx, float dy)> func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].OnLeaveCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------

    void Scene::AddOnLeaveCallback (const std::string& triggerName, asIScriptFunction* func)
    {
        if (m_TriggerAreas.find (triggerName) != m_TriggerAreas.end ())
        {
            m_TriggerAreas[triggerName].ScriptOnLeaveCallback = func;
        }
    }

    //--------------------------------------------------------------------------------------------------
}
