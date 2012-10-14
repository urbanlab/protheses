#ifndef INFOVIZ_H
#define INFOVIZ_H

#include <string>
#include <OgreSceneManager.h>
#include <OgreNode.h>
#include <OgreEntity.h>

class Infoviz
{
private:
    std::string mFilename;
    Ogre::Rectangle2D *mRectangle;
    Ogre::SceneNode *mNode;
    float x,y,width,height;


public:
    Infoviz(std::string name, float _x, float _y, float _width, float _height) :
        mFilename(name), x(_x), y(_y), width(_width), height(_height) {}

    void load(Ogre::SceneManager* mgr);
    void show();
    void hide();
};

#endif // INFOVIZ_H
