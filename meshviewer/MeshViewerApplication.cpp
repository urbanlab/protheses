// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab

#include "MeshViewerApplication.h"


void MeshViewerApplication::createScene()
{
    mRootNode = mSceneMgr->getRootSceneNode();
    mModel = mSceneMgr->createEntity("Model", mMeshFilename);


    mModelNode = mRootNode->createChildSceneNode("ModelNode");
    mModelNode->attachObject(mModel);
    mModelNode->setScale(this->scale, this->scale, this->scale);
    mModel->setDisplaySkeleton(true);

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);
    mAnimationState = mModel->getAnimationState("default_skl");
    //mAnimationState = mModel->getAnimationState("Walk");
    mAnimationState->setLoop(true);
    mAnimationState->setEnabled(true);

}

bool MeshViewerApplication::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
    mAnimationState->addTime(evt.timeSinceLastFrame);
    return BaseApplication::frameRenderingQueued(evt);
}

int main(int argc, char *argv[])
{
    MeshViewerApplication app;
    app.scale=1.0;

    if(argc>1)
    {
        app.setMeshFilename(argv[1]);
        if(argc>2)
        {
            app.scale = atof(argv[2]);
        }
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
