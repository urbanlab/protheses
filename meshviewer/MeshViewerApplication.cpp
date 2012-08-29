// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab

#include "MeshViewerApplication.h"


void MeshViewerApplication::createScene()
{
    mRootNode = mSceneMgr->getRootSceneNode();
    mModel = mSceneMgr->createEntity("Model", mMeshFilename);
    mModelNode = mRootNode->createChildSceneNode("ModelNode");
    mModelNode ->attachObject(mModel);
    mModel->setDisplaySkeleton(true);

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);
}


int main(int argc, char *argv[])
{
    MeshViewerApplication app;


    if(argc>1)
    {
        app.setMeshFilename(argv[1]);
    }
    else
    {
        app.setMeshFilename("bras_sans_smooth_III_2308..mesh");
    }

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
    }

    return 0;
}
