// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab


#ifndef __MeshViewerApplication_h_
#define __MeshViewerApplication_h_

#include "BaseApplication.h"

class MeshViewerApplication : public BaseApplication
{
public:
    void setMeshFilename(std::string filename) {mMeshFilename=filename;}

protected:
    virtual void createScene(void);

private:
    Ogre::Entity * mModel;
    Ogre::SceneNode * mRootNode;
    Ogre::SceneNode * mModelNode;
    std::string mMeshFilename;
};

#endif
