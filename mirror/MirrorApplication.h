// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab


#ifndef __MirrorApplication_h_
#define __MirrorApplication_h_

#include <XnCppWrapper.h>
#include <XnTypes.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "pointcloud.h"
#include "BaseApplication.h"

#include <OgreTimer.h>
#include "prosthesis.h"


struct ScenarioElement{
    // mesh filename for 3d obects, material name for infoviz
    std::string name;
    // type : 0 : infoviz 1-3 : prosthesis of the corresponding type
    int type;
    // Used only for infoviz
    Ogre::Vector3 pos;
    float width,height;
    // time in seconds before appearing after user detection
    float startTime;
    float fadeInDuration;
    // How long will it be displayed
    float playDuration;
    float fadeoutDuration;
};

class MirrorApplication : public BaseApplication
{
public:
    MirrorApplication();
    void setMeshFilename(std::string filename) {mMeshFilename=filename;}
    void readScenario(std::string filename);

    // Should be private and grouped with all the dirty globals in main
    xn::ImageMetaData mKinectVideo;
    xn::DepthMetaData mKinectDepth;
    xn::SceneMetaData mKinectLabels;
    xn::DepthGenerator mDepthGenerator;


    bool mThreadRunning;
    bool mThreadQuit;
    bool mThreadRanOnce;
    boost::thread * mThread;
    boost::mutex m_Mutex;

protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool updateBone(std::string boneName,
                            XnUserID userId,
                            XnSkeletonJoint jointName);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual void createFrameListener(void);
    virtual void destroyScene(void);
    void updateKinectCloud();
    void kinectThread();




private:
    Prosthesis *mProsthesis[3];
    int mCurrentDisplayed;

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

    Ogre::Timer mTimerSinceDetection;

    double mKinectScaleX;
    double mKinectScaleY;
    double mKinectOffsetX;
    double mKinectOffsetY;

    std::vector<ScenarioElement> mScenario;

    // Hackity stuff
    XnSkeletonJointTransformation mRightShoulderJoint;

};

#endif
