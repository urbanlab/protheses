// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab


#ifndef __MeshViewerApplication_h_
#define __MeshViewerApplication_h_

#include "BaseApplication.h"

class MeshViewerApplication : public BaseApplication
{
public:
    void setMeshFilename(std::string filename) {mMeshFilename=filename;}
    float scale;

protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    Ogre::AnimationState *mAnimationState;
    std::string mMeshFilename;
};

#endif
