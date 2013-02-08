#ifndef PROSTHESIS_H
#define PROSTHESIS_H

#include <string>
#include <OgreSceneManager.h>
#include <OgreNode.h>
#include <OgreEntity.h>
#include <OgreMatrix4.h>
#include <OgreMatrix3.h>
#include <OgreSubEntity.h>
#include <OgreMaterial.h>
#include "fader.h"
#include <XnCppWrapper.h>

class Prosthesis : public Fader
{
private:
  double mStartTime;
  double mFadeinDuration;
  double mFadeoutDuration;
  double mDuration;
  int mType;
  std::string mFilename;

  Ogre::Entity *mEntity;
  Ogre::SceneNode *mNode;

  Ogre::Matrix4 mTransf;
  xn::SkeletonCapability * mSkel;
  xn::DepthGenerator * mDepthGen;

public:
    Prosthesis(std::string name, double start, double fid, double fos,
               double duration ,int  type) :
      mFilename(name), mStartTime(start), mFadeinDuration(fid),
      mFadeoutDuration(fos), mDuration(duration), mType(type)
    {
      //dbgT = Ogre::Vector3(-5, -120, 3);
      //dbgT = Ogre::Vector3(-235,335,2.0);
      dbgT = Ogre::Vector3(0,0,0);
      dbgYaw = 160.0;
      dbgPitch = -12.0;
      dbgRoll = 98.0;
      scaleFactor = 1.0;
      kinectZCorrection = 1.0f;
    }
    void load(Ogre::SceneManager* mgr);
    void updateAllJoints(unsigned long dt, xn::SkeletonCapability* sc,
                xn::DepthGenerator *dg, XnUserID user);
    bool transformBone(std::string boneName, XnSkeletonJoint jointName,
                       XnUserID userId,
                       bool inheritsScale,
                       bool inheritOrientation,
                       bool updatePosition,
                       bool updateOrientation);
    void update(float dT);

    void show();
    void hide();
    Ogre::Vector3 dbgT;
    double dbgYaw;
    double dbgPitch;
    double dbgRoll;
    double scaleFactor;
    double kinectZCorrection;
};

#endif // PROSTHESIS_H
