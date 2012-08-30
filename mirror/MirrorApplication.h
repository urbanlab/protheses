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

private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    std::string mMeshFilename;

    XnUserID aUsers[15];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation torsoJoint;

};

#endif
