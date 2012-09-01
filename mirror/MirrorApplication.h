// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab


#ifndef __MirrorApplication_h_
#define __MirrorApplication_h_

#include <XnCppWrapper.h>
#include "BaseApplication.h"

class MirrorApplication : public BaseApplication
{
public:
    void setMeshFilename(std::string filename) {mMeshFilename=filename;}

protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool updateBone(std::string boneName,
                            XnUserID userId,
                            XnSkeletonJoint jointName);

private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    Ogre::AnimationState *mAnimationState;
    std::string mMeshFilename;

    Ogre::SceneNode * mKinectNode[10];

    XnUserID mCurrentUserXn;
    XnUInt16 nUsers;


    Ogre::Entity * mDebugEnt[10];
    Ogre::SceneNode * mDebugNode[10];

};

#endif
