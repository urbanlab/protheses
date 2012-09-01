// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab

#include "MirrorApplication.h"

using namespace std;
using namespace Ogre;

//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
xn::Context g_Context;
xn::ScriptNode g_scriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::UserGenerator g_UserGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

XnBool fileExists(const char *fn)
{
        XnBool exists;
        xnOSDoesFileExist(fn, &exists);
        return exists;
}

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    if (g_bNeedPose)
    {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    }
    else
    {
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    }
    else
    {
        // Calibration failed
        printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose)
        {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}

#define CHECK_RC(nRetVal, what)					    \
    if (nRetVal != XN_STATUS_OK)				    \
{								    \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
    return nRetVal;						    \
}


void MirrorApplication::createScene()
{
    Ogre::Entity *ent;
    Ogre::SceneNode *node;
    const float SCALE = 1.0f;

    mRootNode = mSceneMgr->getRootSceneNode();
    mModel = mSceneMgr->createEntity("Model", mMeshFilename);
    mModelNode = mRootNode->createChildSceneNode("ModelNode");
    mModelNode->attachObject(mModel);
    mModelNode->setScale(SCALE, SCALE, SCALE);
    mModel->setDisplaySkeleton(true);

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
    mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.2,0.2,0.2w));

    AxisAlignedBox aabb = AxisAlignedBox(-10e10, -10e10, -10e10,
                                          10e10,  10e10,  10e10);
    mModel->getMesh()->_setBounds(aabb);

    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 1050.0f);
    mCamera->setPosition(0,200,800);
    //mCamera->lookAt(0,200,1400);
//    /mCamera->setPosition(0,200,0);

    bool visible = true;
    mKinectNode[0] = mRootNode->createChildSceneNode("K1");
    mKinectNode[1] = mRootNode->createChildSceneNode("K2");
    mKinectNode[2] = mRootNode->createChildSceneNode("K3");
    mKinectNode[3] = mRootNode->createChildSceneNode("K4");

    if(visible)
    {
        mKinectNode[0]->attachObject(mSceneMgr->createEntity("Knot1", "knot.mesh"));
        mKinectNode[1]->attachObject(mSceneMgr->createEntity("Knot2", "knot.mesh"));
        mKinectNode[2]->attachObject(mSceneMgr->createEntity("Knot3", "knot.mesh"));
        mKinectNode[3]->attachObject(mSceneMgr->createEntity("Knot4", "knot.mesh"));
    }
    for(int i=0;i<4;i++)
        mKinectNode[i]->setScale(0.1,0.1,0.1);

    mDebugEnt[0] = mSceneMgr->createEntity("DebugEnt0", "knot.mesh");
    mDebugNode[0] = mRootNode->createChildSceneNode("DebugNode0");
    mDebugNode[0]->attachObject(mDebugEnt[0]);
    mDebugNode[0]->setPosition(500,-100,0);
    mDebugNode[0]->setScale(1,1,1);
    mDebugNode[0]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);


    mDebugEnt[1] = mSceneMgr->createEntity("DebugEnt1", "knot.mesh");
    mDebugNode[1] = mRootNode->createChildSceneNode("DebugNode1");
    mDebugNode[1]->attachObject(mDebugEnt[1]);
    mDebugNode[1]->setPosition(0,-400,0);
    mDebugNode[1]->setScale(1,1,1);
    mDebugNode[1]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);

}

bool MirrorApplication::updateBone(std::string boneName,
                                   XnUserID userId,
                                   XnSkeletonJoint jointName)
{
    try
    {
        XnSkeletonJointTransformation joint;
        Ogre::Vector3 v;

        Ogre::Bone* bone = mModel->getSkeleton()->getBone(jointName);
        bone->setManuallyControlled(true);
        bone->setInheritOrientation(false);
        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(userId,jointName,joint);

        Ogre::Quaternion quat(joint.orientation.orientation.elements);
        //bone->resetOrientation();
        //quat = bone->convertWorldToLocalOrientation(quat);
        //bone->setOrientation(quat * bone->getInitialOrientation());
        //bone->setOrientation(quat);
        //Ogre::Vector3 v;
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);


        //bone->setPosition(bone->_getFullTransform().inverse()*v);
        bone->resetToInitialState();
        cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
        cout << v << endl;
        cout << bone->convertWorldToLocalPosition(v) << endl << endl;

        v=bone->convertWorldToLocalPosition(v);
        bone->translate(v);
        cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
        cout << Ogre::Vector3(joint.position.position.X,
                                  joint.position.position.Y,
                                  joint.position.position.Z) << endl;

    }
    catch( Ogre::Exception& e )
    {
        cout << "Problem updating bone "<<boneName
             << "with user "<< mCurrentUserXn << endl;
        return false;
    }
    return true;
}

bool MirrorApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{

    /*XnUserID aUsers[15];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation joint;

    g_Context.WaitOneUpdateAll(g_UserGenerator);
    // print the torso information for the first user already tracking
    nUsers=15;
    g_UserGenerator.GetUsers(aUsers, nUsers);

    for(XnUInt16 i=0; i<nUsers; i++)
    {
        if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==TRUE)
        {
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_TORSO,joint);
                printf("user %d: head at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
                                                                joint.position.position.X,
                                                                joint.position.position.Y,
                                                                joint.position.position.Z);
                break;
        }
    }*/




    g_Context.WaitOneUpdateAll(g_UserGenerator);

    // We suppose a max of 15 memorized users
    nUsers=15;
    XnUserID aUsers[15];
    XnSkeletonJointTransformation joint;
    g_UserGenerator.GetUsers(aUsers, nUsers);

    bool found=false;

    for(XnUserID i=0; (i<nUsers)&&(!found); i++)
    {
        if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==TRUE)
        {
            mCurrentUserXn=i;
            found=true;
            cout << "found ("<<mCurrentUserXn<<") " << endl;
        }
    }

    Ogre::Vector3 v;
    v = Ogre::Vector3(500, 0, 0);
    Ogre::Bone* bone = mModel->getSkeleton()->getBone("epaule_");
    bone->setManuallyControlled(true);
    bone->setInheritOrientation(false);
    bone->setInheritScale(false);

    //bone->setPosition(bone->_getFullTransform().inverse()*v);
    cout << "**********************" << endl;
    //mModelNode->setPosition(0,0,0);
    bone->resetToInitialState();
    //bone->setPosition(0,0,0);
    //bone->setOrientation(1,0,0,0);
    bone->setScale(1000,1000,1000);



    cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
    cout << v << endl;
    cout << bone->convertWorldToLocalPosition(v) << endl << endl;

    bone->setPosition(mModelNode->_getDerivedOrientation().Inverse()*
                      (v - mModelNode->_getDerivedPosition()));
    v=bone->convertWorldToLocalPosition(v);
    bone->translate(v);
    cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
    cout << bone->getPosition() << endl;
    cout << v << endl;



    v = Vector3(0, -400, 0);
    bone = mModel->getSkeleton()->getBone("bras_");
    bone->setManuallyControlled(true);
    //bone->setInheritOrientation(false);
    //bone->setInheritScale(false);

    //bone->setPosition(bone->_getFullTransform().inverse()*v);
    //mModelNode->setPosition(0,0,0);
    bone->resetToInitialState();
    //bone->setPosition(0,0,0);
    //bone->setOrientation(1,0,0,0);
    //bone->setScale(1000,1000,1000);


    cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
    cout << v << endl;
    cout << bone->convertWorldToLocalPosition(v) << endl << endl;

    bone->setPosition(mModelNode->_getDerivedOrientation().Inverse()*
                      (v - mModelNode->_getDerivedPosition()));
    v=bone->convertWorldToLocalPosition(v);
    bone->translate(v);
    cout << bone->convertLocalToWorldPosition(bone->getPosition()) << endl;
    cout << bone->getPosition() << endl;
    cout << v << endl;


    //g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_LEFT_ELBOW,elbowLeftJoint);
    if(found)
    {



        XnSkeletonJointTransformation joint;
        Ogre::Vector3 v;


        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_LEFT_SHOULDER,joint);
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);
        mModelNode->setPosition(v);

        /*updateBone("epaule_", aUsers[mCurrentUserXn], XN_SKEL_LEFT_SHOULDER);
        updateBone("avant_bras_", aUsers[mCurrentUserXn], XN_SKEL_LEFT_ELBOW);
        updateBone("poignet_", aUsers[mCurrentUserXn], XN_SKEL_LEFT_WRIST);*/
        //updateBone("bras_", aUsers[mCurrentUserXn], XN_SKEL_LEFT_ELBOW);




        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_LEFT_SHOULDER,joint);
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);
        mKinectNode[1]->setPosition(v);

        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_LEFT_ELBOW,joint);
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);
        mKinectNode[2]->setPosition(v);
        cout << mKinectNode[2]->getPosition() << endl;

        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_LEFT_HAND,joint);
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);
        mKinectNode[3]->setPosition(v);

        mDebugNode[0]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);
    }


    return BaseApplication::frameRenderingQueued(evt);
}

int main(int argc, char *argv[])
{
    MirrorApplication app;

    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    nRetVal = g_Context.InitFromXmlFile("../Data/SamplesConfig.xml", g_scriptNode, &errors);

    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (nRetVal);
    }
    else if (nRetVal != XN_STATUS_OK)
    {
        printf("Open failed: %s\n", xnGetStatusString(nRetVal));
        return (nRetVal);
    }

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
    CHECK_RC(nRetVal,"No depth");

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = g_UserGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return 1;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");


    printf("Starting to run\n");
    if(g_bNeedPose)
    {
        printf("Assume calibration pose\n");
    }
    XnUInt32 epochTime = 0;


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

    g_scriptNode.Release();
    g_DepthGenerator.Release();
    g_UserGenerator.Release();
    g_Context.Release();

    return 0;
}
