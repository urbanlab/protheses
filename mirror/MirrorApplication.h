// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab


#ifndef __MirrorApplication_h_
#define __MirrorApplication_h_

#include <XnCppWrapper.h>
#include <XnTypes.h>
#include "pointcloud.h"
#include "BaseApplication.h"

class MirrorApplication : public BaseApplication
{
public:
    void setMeshFilename(std::string filename) {mMeshFilename=filename;}

    // Should private and grouped with all the dirty globals in main
    xn::ImageMetaData mKinectVideo;
    xn::DepthMetaData mKinectDepth;
    xn::DepthGenerator mDepthGenerator;


protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool updateBone(std::string boneName,
                            XnUserID userId,
                            XnSkeletonJoint jointName);
    void updateKinectCloud();




private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    Ogre::AnimationState *mAnimationState;
    std::string mMeshFilename;

    Ogre::SceneNode * mKinectNode[10];

    XnUserID mCurrentUserXn;
    XnUInt16 nUsers;

    Ogre::Entity * mPointCloudEnt;
    PointCloud * mPointCloud;
    Ogre::Entity * mDebugEnt[10];
    Ogre::SceneNode * mDebugNode[10];



};

#endif
