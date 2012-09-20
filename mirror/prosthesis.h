#ifndef PROSTHESIS_H
#define PROSTHESIS_H

#include <string>
#include <OgreSceneManager.h>
#include <OgreNode.h>
#include <OgreEntity.h>
#include <OgreMatrix4.h>
#include <OgreMatrix3.h>

#include <XnCppWrapper.h>

class Prosthesis
{
private:
  double mStartTime;
  double mFadeinSpeed;
  double mFadeoutSpeed;
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
      mFilename(name), mStartTime(start), mFadeinSpeed(fid),
      mFadeoutSpeed(fos), mDuration(duration), mType(type) {}
    void load(Ogre::SceneManager* mgr);
    void updateAllJoints(unsigned long dt, xn::SkeletonCapability* sc,
                xn::DepthGenerator *dg, XnUserID user);
    void transformBone(std::string boneName, XnSkeletonJoint jointName,
                       XnUserID userId,
                       bool inheritsScale,
                       bool inheritOrientation,
                       bool updatePosition,
                       bool updateOrientation);
};

#endif // PROSTHESIS_H
