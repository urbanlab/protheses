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
    MirrorApplication();
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
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual void createFrameListener(void);
    void updateKinectCloud();




private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    Ogre::AnimationState *mAnimationState;
    std::string mMeshFilename;
    Ogre::Bone * mSelectedBone;

    Ogre::Vector3 mCamPresetPos[10];
    Ogre::Vector3 mCamPresetLookAt[10];

    Ogre::SceneNode * mKinectNode[10];

    XnUserID mCurrentUserXn;
    XnUInt16 nUsers;

    Ogre::Entity * mPointCloudEnt;
    PointCloud * mPointCloud;
    Ogre::Entity * mDebugEnt[10];
    Ogre::SceneNode * mDebugNode[10];

    double mDebugYaw, mDebugPitch, mDebugRoll;


};

#endif
