#ifndef INFOVIZ_H
#define INFOVIZ_H

#include "fader.h"
#include <string>
#include <OgreSceneManager.h>
#include <OgreNode.h>
#include <OgreEntity.h>

class Infoviz : public Fader
{
private:
    std::string mFilename;
    Ogre::Rectangle2D *mRectangle;
    Ogre::SceneNode *mNode;
    float x,y,width,height;
    double mStartTime;
    double mFadeinDuration;
    double mFadeoutDuration;
    double mDuration;

public:
    Infoviz(std::string name, float _x, float _y, float _width, float _height,
            double start, double fid, double fos, double duration ) :
        mFilename(name), x(_x), y(_y), width(_width), height(_height),
        mStartTime(start), mFadeinDuration(fid), mFadeoutDuration(fos),
        mDuration(duration){}

    void load(Ogre::SceneManager* mgr);
    void show();
    void hide();
    void update(float dT);
};

#endif // INFOVIZ_H
