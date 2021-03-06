// (c) Copyright 2012 Yves Quemener (quemener.yves@free.fr), Museolab

#include "MirrorApplication.h"
#include "infoviz.h"
#include <vector>
#include <fstream>


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

MirrorApplication::MirrorApplication()
{
    mSelectedBone=0;
    mCurrentUserXn=0;
    mThreadRunning=false;
    mThreadQuit=false;
    mThreadRanOnce=false;
    mThread = new boost::thread(boost::bind(&MirrorApplication::kinectThread, this));
    mKinectScaleX = 0.88;
    mKinectScaleY = 0.86;
    mKinectOffsetX = 22.0;
    mKinectOffsetY = 49;
    mScaleFactor = 1.0;
    mCurrentDisplayed = 0;
    mHideArm = false;
}

void MirrorApplication::destroyScene(void)
{
        // Stops the thread
        assert(mThread);
        mThreadQuit = true;
        mThread->join();

        // Destroy scene
        BaseApplication::destroyScene();
}

void MirrorApplication::createFrameListener(void)
{
    BaseApplication::createFrameListener();
    mKeyboard->setEventCallback(this);
    if(!mCfgDisplayDebug)
    {
      mTrayMgr->hideFrameStats();
      mDetailsPanel->hide();
    }

}

void MirrorApplication::createScene()
{
    Ogre::Entity *ent;
    Ogre::SceneNode *node;
    const float SCALE = 1.0f;

    mRootNode = mSceneMgr->getRootSceneNode();
    /*mModel = mSceneMgr->createEntity("Model", mMeshFilename);
    mModelNode = mRootNode->createChildSceneNode("ModelNode");
    mModelNode->attachObject(mModel);
    mModelNode->setScale(SCALE, SCALE, SCALE);
    mModel->setDisplaySkeleton(false);*/

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
    //mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.5,0.5,0.5));
    mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0,0,0));

    //AxisAlignedBox aabb = AxisAlignedBox(-10e10, -10e10, -10e10,
    //                                          10e10,  10e10,  10e10);
    //mModel->getMesh()->_setBounds(aabb);

    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 1050.0f);
    //mCamera->setPosition(0,200,1400);
    mCamera->lookAt(0,-200,1400);
    mCamera->setPosition(0,-200,0);
    mCamera->setFOVy(Radian(Degree(48.9)));

    mCamPresetPos[0] = Vector3(0,-200,0);
    mCamPresetLookAt[0] = Vector3(200,-200,1400);

    mCamPresetPos[1] = Vector3(0,200,1400);
    mCamPresetLookAt[1] = Vector3(0,0,0);

    mCamPresetPos[2] = Vector3(0,1000,730);
    mCamPresetLookAt[2] = Vector3(0,0,950);

    mCamPresetPos[3] = Vector3(-1200,-10,950);
    mCamPresetLookAt[3] = Vector3(0,0,950);

    bool visible = false;
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
    {
        mKinectNode[i]->setScale(0.1,0.1,0.1);
        mKinectNode[i]->setPosition(70*i,0,0);
    }

    mDebugEnt[0] = mSceneMgr->createEntity("DebugEnt0", "knot.mesh");
    mDebugNode[0] = mRootNode->createChildSceneNode("DebugNode0");
    mDebugNode[0]->attachObject(mDebugEnt[0]);
    mDebugNode[0]->setPosition(640,-480,1024);
    mDebugNode[0]->setScale(0.2,0.2,0.2);
    mDebugNode[0]->setVisible(visible);
    //mDebugNode[0]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);


    mDebugEnt[1] = mSceneMgr->createEntity("DebugEnt1", "knot.mesh");
    mDebugNode[1] = mRootNode->createChildSceneNode("DebugNode1");
    mDebugNode[1]->attachObject(mDebugEnt[1]);
    mDebugNode[1]->setPosition(0,0,1024);
    mDebugNode[1]->setScale(0.2,0.2,0.2);
    mDebugNode[1]->setVisible(visible);
    //mDebugNode[1]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);


    mDebugEnt[2] = mSceneMgr->createEntity("DebugEnt2", "knot.mesh");
    mDebugNode[2] = mRootNode->createChildSceneNode("DebugNode2");
    mDebugNode[2]->attachObject(mDebugEnt[2]);
    mDebugNode[2]->setPosition(0,0,0);
    mDebugNode[2]->setScale(1,1,1);
    mDebugNode[2]->setVisible(visible);
    //mDebugNode[2]->lookAt(mModelNode->getPosition(), Ogre::Node::TS_WORLD);


    /*mProsthesis[0] = new Prosthesis("coeur_scene_3008bras.mesh", 0, 1,1,20,1);
    mProsthesis[1] = new Prosthesis("coeur_scene_3008Hose001.mesh", 0, 1,1,20,2);
    mProsthesis[2] = new Prosthesis("Coeur_I..mesh", 0, 1,1,20,3);
    mCurrentDisplayed=0;

    mProsthesis[0]->load(mSceneMgr);
    mProsthesis[1]->load(mSceneMgr);
    mProsthesis[2]->load(mSceneMgr);*/


    const float ww = mWindow->getWidth();
    const float wh = mWindow->getHeight();

    const int numpoints = ww*wh;
    float *pointlist = new float[numpoints*3];
    float *colorarray = new float[numpoints*3];
    mPointCloud = new PointCloud("KinectCloud", "General",
                                    numpoints, pointlist, colorarray);
    delete[] pointlist;
    delete[] colorarray;

    mPointCloudEnt = mSceneMgr->createEntity("KinectCloudEnt", "KinectCloud");
    mPointCloudEnt->setMaterialName("Pointcloud");
    mPointCloudNode = mRootNode->createChildSceneNode("KinectCouldNode");
    mPointCloudNode->attachObject(mPointCloudEnt);
    mPointCloudNode->setPosition(-320,240,500);
    readScenario("scenario.cfg");

    mThreadRunning = true;

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
        //bone->setInheritOrientation(false);
        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(userId,jointName,joint);

        Ogre::Quaternion quat(joint.orientation.orientation.elements);
        bone->resetToInitialState();
        //bone->resetOrientation();
        //quat = bone->convertWorldToLocalOrientation(quat);
        //bone->setOrientation(quat * bone->getInitialOrientation());
        //bone->setOrientation(quat);
        //Ogre::Vector3 v;
        v = Ogre::Vector3(joint.position.position.X,
                          joint.position.position.Y,
                          joint.position.position.Z);


        v=bone->convertWorldToLocalPosition(v);
        bone->translate(v);

    }
    catch( Ogre::Exception& e )
    {
        cout << "Problem updating bone "<<boneName
             << "with user "<< mCurrentUserXn << endl;
        return false;
    }
    return true;
}

void MirrorApplication::kinectThread()
{
    while(!mThreadQuit)
    {
        if(mThreadRunning)
        {
            g_Context.WaitOneUpdateAll(g_UserGenerator);
            xn::ImageGenerator imgene;
            g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, imgene);
            imgene.GetMetaData(mKinectVideo);
            mDepthGenerator.GetMetaData(mKinectDepth);
            g_UserGenerator.GetUserPixels(0, mKinectLabels);
            mThreadRanOnce=true;
        }
        else
        {
            boost::this_thread::sleep(boost::posix_time::millisec(500));
        }
    }
}

void MirrorApplication::updateKinectCloud()
{
    const XnRGB24Pixel* videoImage = mKinectVideo.RGB24Data();
    // actually an uint16*
    const XnDepthPixel* videoDepth = mKinectDepth.Data();
    const XnLabel* labels = mKinectLabels.Data();
    const float ww = mWindow->getWidth();
    float horf = 640.0f/ww;
    const float wh = mWindow->getHeight();
    float verf = 480.0f/wh;
    int numpoints = ww*wh;
    static float * pointlist=0;
    static float * colorarray=0;
    int vindex=0;
    int cindex=0;
    int kindex=0;
    static float dT = 0.0;
    dT+=0.04;

    if(!mThreadRanOnce) return;

    if(pointlist==0) pointlist = new float[numpoints*3];
    if(colorarray==0) colorarray = new float[numpoints*3];

/*
    int pointwalker=0;
    for (XnUInt y = 0; y < g_imageMD.YRes(); ++y){

    for (XnUInt x = 0; x < g_imageMD.XRes(); ++x){

    imRGB[y][x].red = pImageRow[y*g_imageMD.XRes() +x].nRed;
    imRGB[y][x].green = pImageRow[y*g_imageMD.XRes() +x].nGreen;
    imRGB[y][x].blue = pImageRow[y*g_imageMD.XRes() +x].nBlue;
    }
    }*/


    XnPoint3D ptShoulderR, ptShoulderL, ptHip;
    ptShoulderR = mRightShoulderJoint.position.position;
    ptShoulderL = mLeftShoulderJoint.position.position;
    ptHip = mRightHipJoint.position.position;
    mDepthGenerator.ConvertRealWorldToProjective(1, &ptShoulderR, &ptShoulderR);
    mDepthGenerator.ConvertRealWorldToProjective(1, &ptHip, &ptHip);

    float dx = ptShoulderR.X - ptHip.X;
    float dy = ptShoulderR.Y - ptHip.Y;
    float sFactor = sqrt(dx*dx+dy*dy);

    for(int jj=0;jj<wh;jj++)
    {
        for(int ii=0;ii<ww;ii++)
        {
          int i=ii*horf;
          int j=jj*verf;
          kindex = i+j*640;
          bool visible = true;
          if((mHideArm)&&(i>ptShoulderR.X+sFactor*0.1)
                       &&(fabs(ptShoulderL.X-i)>sFactor*3)
                       &&(j<ptHip.Y+sFactor*0.8))
            visible = false;

          unsigned int newIndex = mKinectOffsetX + i*mKinectScaleX +
              640*(int)(mKinectOffsetY + j*mKinectScaleY);
          newIndex = newIndex % (640*480);

          if((labels[kindex]!=0)&&(visible))
          {
            colorarray[cindex++]=(videoImage[newIndex].nBlue)/256.0;
            colorarray[cindex++]=(videoImage[newIndex].nGreen)/256.0;
            colorarray[cindex++]=(videoImage[newIndex].nRed)/256.0;
            pointlist[vindex++]=(float)(ii)*horf;
            pointlist[vindex++]=-(float)(jj)*verf;
            pointlist[vindex++]=videoDepth[kindex];
          }
          else
          {
            colorarray[cindex++]=0.0;
            colorarray[cindex++]=0.0;
            colorarray[cindex++]=0.0;
            pointlist[vindex++]=0;
            pointlist[vindex++]=0;
            pointlist[vindex++]=0;
          }
        }
    }

    cindex=0;
    if(mHighlightContour)
    {
        for(int jj=0;jj<wh;jj++)
        {
            for(int ii=0;ii<ww;ii++)
            {
                int i=ii*horf;
                int j=jj*verf;
                kindex = i+j*640;

                if(labels[kindex]!=0)
                {
                   if((labels[kindex-1]==0)||(labels[kindex+1]==0)
                     ||(labels[kindex+640]==0)||(labels[kindex-640]==0))
                    {
                      colorarray[cindex++]=1.0;
                      colorarray[cindex++]=0.6;
                      colorarray[cindex++]=0.6;
                    }
                   else
                   {
                     colorarray[cindex++]=0.3;
                     colorarray[cindex++]=0.1;
                     colorarray[cindex++]=0.1;
                   }
                }
                else
                {
                  colorarray[cindex++]=0.0;
                  colorarray[cindex++]=0.0;
                  colorarray[cindex++]=0.0;
                }
                kindex++;
            }
        }
    }
    mPointCloud->updateVertexColours(numpoints, colorarray);
    mPointCloud->updateVertexPositions(numpoints, pointlist);
}

bool MirrorApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    Bone* bone;
    static float dT=0.0;

    /*mModelNode->setPosition(0,0,0);
    bone = mModel->getSkeleton()->getBone("epaule_");
    bone->setManuallyControlled(true);
    bone->setScale(1000,1000,1000);*/
    updateKinectCloud();


#if 0
    g_Context.WaitOneUpdateAll(g_UserGenerator);
    //g_Context.WaitAndUpdateAll();
    xn::ImageGenerator imgene;
    g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, imgene);
    imgene.GetMetaData(mKinectVideo);


    //g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, mDepthGenerator);
    mDepthGenerator.GetMetaData(mKinectDepth);
#endif

    // We suppose a max of 15 memorized users
    nUsers=15;
    XnUserID aUsers[15];
    XnSkeletonJointTransformation joint;
    g_UserGenerator.GetUsers(aUsers, nUsers);

    bool found=false;
    bool static previousFound=false;

    for(XnUserID i=0; (i<nUsers)&&(!found); i++)
    {
        if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==TRUE)
        {
          if(mCurrentUserXn!=i)
          {
            mTimerSinceDetection.reset();
          }
          mCurrentUserXn=i;
          //cout << "Found = "<<mCurrentUserXn<<endl;
          //cout << mTimerSinceDetection.getMilliseconds()<<endl;
          if(!previousFound) dT=0.0f;
          found=true;
        }
    }
    previousFound=found;

    if(found)
    {
      xn::SkeletonCapability sc = g_UserGenerator.GetSkeletonCap();

      dT = dT + evt.timeSinceLastFrame;
      if((dT>mScenarioLoopTime)&&(mScenarioLoopTime>0.0f)) dT=0.0;
      for(int i=0;i<mScenario.size();i++)
      {
          mScenario[i].fader->update(dT);
          if((mScenario[i].startTime<dT)&&
             (mScenario[i].startTime+mScenario[0].playDuration
              +mScenario[0].fadeoutDuration+mScenario[0].fadeInDuration>dT))
          {
              if(mScenario[i].type>0)
              {
                ((Prosthesis*)(mScenario[i].fader))->updateAllJoints(
                      mTimerSinceDetection.getMilliseconds(),
                      &sc, &mDepthGenerator, aUsers[mCurrentUserXn]);
              }
              mHideArm = mScenario[i].hideArm!=0;
          }
      }


        XnSkeletonJointTransformation joint;
        Vector3 v;
        XnPoint3D xnv;

        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_RIGHT_SHOULDER,joint);
        mRightShoulderJoint = joint;
        mDepthGenerator.ConvertRealWorldToProjective
                (1,&(joint.position.position), &xnv);
        v = Ogre::Vector3(xnv.X, -xnv.Y, xnv.Z);
        mKinectNode[1]->setPosition(v);
        //cout << "shoulder=" << v << endl;


        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_RIGHT_ELBOW,joint);
        mDepthGenerator.ConvertRealWorldToProjective
                (1,&(joint.position.position), &xnv);
        v = Ogre::Vector3(xnv.X, -xnv.Y, xnv.Z);
        mKinectNode[2]->setPosition(v);
        //cout << "elbow=" << v << endl;


        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_RIGHT_HAND,joint);
        mDepthGenerator.ConvertRealWorldToProjective
                (1,&(joint.position.position), &xnv);
        v = Ogre::Vector3(xnv.X, -xnv.Y, xnv.Z);
        mKinectNode[3]->setPosition(v);
        //cout << "hand=" << v << endl;


        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_RIGHT_HIP,joint);
        mRightHipJoint = joint;

        g_UserGenerator.GetSkeletonCap().
                GetSkeletonJoint(aUsers[mCurrentUserXn],XN_SKEL_LEFT_SHOULDER,joint);
        mLeftShoulderJoint = joint;
    }

    else
    {
        if(mSelectedBone)
        {
            mSelectedBone->setManuallyControlled(true);
            mSelectedBone->setInheritOrientation(false);
            Quaternion quat;
            quat.FromAngleAxis(Degree(90*cos(dT)), Vector3(0,0,1));
            mSelectedBone->setOrientation(quat);
        }
    }


    return BaseApplication::frameRenderingQueued(evt);
}

bool MirrorApplication::mouseMoved( const OIS::MouseEvent &arg )
{
  if(mCfgMouseControl) return BaseApplication::mouseMoved(arg);
}

bool MirrorApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if(mCfgMouseControl) return BaseApplication::mousePressed(arg, id);
}

bool MirrorApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if(mCfgMouseControl) return BaseApplication::mouseReleased(arg, id);
}


bool MirrorApplication::keyPressed( const OIS::KeyEvent &arg )
{
  if(!mCfgKeyboardControl)
  {
    if (arg.key == OIS::KC_ESCAPE)
    {
       mShutDown = true;
    }
      return false;
  }
  bool kinectCalibChanged=false;
  bool anyChange=true;

  if (arg.key == OIS::KC_NUMPAD0)
  {
      mCamera->setPosition(mCamPresetPos[0]);
      mCamera->lookAt(mCamPresetLookAt[0]);
  }
  else if (arg.key == OIS::KC_NUMPAD1)
  {
      mCamera->setPosition(mCamPresetPos[1]);
      mCamera->lookAt(mCamPresetLookAt[1]);
  }
  else if (arg.key == OIS::KC_NUMPAD2)
  {
      mCamera->setPosition(mCamPresetPos[2]);
      mCamera->lookAt(mCamPresetLookAt[2]);
  }
  else if (arg.key == OIS::KC_NUMPAD3)
  {
      mCamera->setPosition(mCamPresetPos[3]);
      mCamera->lookAt(mCamPresetLookAt[3]);
  }
  else if (arg.key == OIS::KC_I)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.x += 5;
  }
  else if (arg.key == OIS::KC_K)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.x -= 5;
  }
  else if (arg.key == OIS::KC_O)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.y += 5;
  }
  else if (arg.key == OIS::KC_L)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.y -= 5;
  }
  else if (arg.key == OIS::KC_P)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.z += 5;
  }
  else if (arg.key == OIS::KC_M)
  {
    mProsthesis[mCurrentDisplayed]->dbgT.z -= 5;
  }
  else if (arg.key == OIS::KC_A)
  {
    /*Radian r = mCamera->getFOVy();
    r*=1.05;
    mCamera->setFOVy(r);*/
    mProsthesis[mCurrentDisplayed]->kinectZCorrection *= 1.05;
  }
  else if (arg.key == OIS::KC_Q)
  {
    /*Radian r = mCamera->getFOVy();
    r*=0.95;
    mCamera->setFOVy(r);*/
    mProsthesis[mCurrentDisplayed]->kinectZCorrection *= 0.95;
  }
  /*else if (arg.key == OIS::KC_Z)
  {
    Vector3 v = mPointCloudNode->getPosition();
    v.z*=1.05;
    mPointCloudNode->setPosition(v);
  }
  else if (arg.key == OIS::KC_S)
  {
    Vector3 v = mPointCloudNode->getPosition();
    v.z*=0.95;
    mPointCloudNode->setPosition(v);
  }
 else if (arg.key == OIS::KC_E)
  {
    mProsthesis[mCurrentDisplayed]->scaleFactor *= 1.05;
  }
  else if (arg.key == OIS::KC_D)
  {
    mProsthesis[mCurrentDisplayed]->scaleFactor *= 0.95;
  }
  else if (arg.key == OIS::KC_R)
  {
    Radian r = mCamera->getFOVy();
    r*=1.05;
    mCamera->setFOVy(r);
  }
  else if (arg.key == OIS::KC_F)
  {
    Radian r = mCamera->getFOVy();
    r*=0.95;
    mCamera->setFOVy(r);
  }*/
  /*else if (arg.key == OIS::KC_T)
  {
      mProsthesis[mCurrentDisplayed]->dbgYaw += 1;
  }
  else if (arg.key == OIS::KC_G)
  {
      mProsthesis[mCurrentDisplayed]->dbgYaw -= 1;
  }
  else if (arg.key == OIS::KC_Y)
  {
      mProsthesis[mCurrentDisplayed]->dbgPitch += 1;
  }
  else if (arg.key == OIS::KC_H)
  {
      mProsthesis[mCurrentDisplayed]->dbgPitch -= 1;
  }
  else if (arg.key == OIS::KC_U)
  {
      mProsthesis[mCurrentDisplayed]->dbgRoll += 1;
  }
  else if (arg.key == OIS::KC_J)
  {
      mProsthesis[mCurrentDisplayed]->dbgRoll -= 1;
  }*/
  else if (arg.key == OIS::KC_B)
  {
      mKinectOffsetY += 1;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_N)
  {
      mKinectOffsetY -= 1;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_C)
  {
      mKinectScaleY +=0.01;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_V)
  {
      mKinectScaleY -=0.01;
      kinectCalibChanged=true;
  }

  else if (arg.key == OIS::KC_H)
  {
      mKinectOffsetX += 1;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_J)
  {
      mKinectOffsetX -= 1;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_F)
  {
      mKinectScaleX +=0.01;
      kinectCalibChanged=true;
  }
  else if (arg.key == OIS::KC_G)
  {
      mKinectScaleX -=0.01;
      kinectCalibChanged=true;
  }


  else if (arg.key == OIS::KC_SPACE)
  {
      mCurrentDisplayed = (mCurrentDisplayed+1)%mProsthesis.size();
  }
  else
  {
    anyChange=false;
    return BaseApplication::keyPressed(arg);
  }
  /*for(int i=0;i<3;i++)
  {
    mProsthesis[i]->hide();
  }
  mProsthesis[mCurrentDisplayed]->show();*/
  if(anyChange)
  {
    cout << "Kinect calib : ";
    cout << "OffX: " << mKinectOffsetX << "\t";
    cout << "OffY: " << mKinectOffsetY << "\t";
    cout << "ScaleX: " << mKinectScaleX << "\t";
    cout << "ScaleY: " << mKinectScaleY << endl;

    cout << mProsthesis[mCurrentDisplayed]->dbgT << endl;
    /*cout << "Fov Y = " << Degree(mCamera->getFOVy());
    cout << "\t Point cloud distance = " << mPointCloudNode->getPosition().z;
    cout << "\t scale = " << mProsthesis[mCurrentDisplayed]->scaleFactor;
    cout << "\t kinectZ correction :"<<mProsthesis[mCurrentDisplayed]->kinectZCorrection << endl;*/
  }
  return true;
}

void inline readElement(std::ifstream& is, std::string& s)
{
    do
        is >> s;
    while(s[0]=='#');
}

void inline readElement(std::ifstream& is, int& i)
{
    std::stringstream ss;
    do
        ss << is;
    while(ss.str()[0]=='#');
    ss >> i;
}

void inline readElement(std::ifstream& is, float& f)
{
    std::stringstream ss;
    do
        ss << is;
    while(ss.str()[0]=='#');
    ss >> f;
}

void MirrorApplication::readScenario(string filename)
{
    std::ifstream input;
    input.open(filename.c_str(), std::ios_base::in);
    ScenarioElement se;
    mScenario.clear();

    // Remove comments
    char buf[1001];
    std::stringstream ss;
    while(!input.eof())
    {
        input.getline(buf,1000);
        if(buf[0]=='#') continue;
        ss << buf << '\n';
    }

    ss >> mScenarioLoopTime;
    float px,py,pz;
    ss >> px >> py >> pz;
    mCamera->setPosition(px,py,pz);
    ss >> px >> py >> pz;
    mCamera->lookAt(px,py,pz);
    ss >> mKinectOffsetX >> mKinectOffsetY >> mKinectScaleX >> mKinectScaleY;
    int fakebool;
    ss >> fakebool;
    mHighlightContour = (fakebool!=0);
    ss >> fakebool;
    mCfgDisplayDebug=(fakebool!=0);
    ss >> fakebool;
    mCfgKeyboardControl=(fakebool!=0);
    ss >> fakebool;
    mCfgMouseControl=(fakebool!=0);

    while(ss.rdbuf()->in_avail())
    {
        ss >> se.name;
        ss >> se.type;
        ss >> se.hideArm;
        ss >> se.pos.x >> se.pos.y >> se.pos.z;
        ss >> se.width >> se.height;
        ss >> se.startTime >> se.fadeInDuration;
        ss >> se.playDuration;
        ss >> se.fadeoutDuration;
        if(!ss.rdbuf()->in_avail()) break;
        /*char dump[2];
        ss >> dump;
*/
        if(se.type==0)
        {
            Infoviz* iz = new Infoviz(se.name, se.pos.x, se.pos.y,
                                      se.width, se.height,
                                      se.startTime,
                                      se.fadeInDuration,
                                      se.fadeoutDuration,
                                      se.playDuration);
            iz->load(this->mSceneMgr);
            se.fader=iz;
        }
        else
        {
            Prosthesis* p=new Prosthesis(se.name, se.startTime,
                                        se.fadeInDuration,
                                        se.fadeoutDuration,
                                        se.playDuration,
                                        se.type);
            p->load(this->mSceneMgr);
            mProsthesis.push_back(p);;
            se.fader=p;
        }

        mScenario.push_back(se);
    }
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

    xn::ImageGenerator imgene;
    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, imgene);
    CHECK_RC(nRetVal, "Init Video input");
    imgene.GetMetaData(app.mKinectVideo);

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, app.mDepthGenerator);
    CHECK_RC(nRetVal, "Init Depth video input");
    app.mDepthGenerator.GetMetaData(app.mKinectDepth);

    printf("Starting to run\n");
    if(g_bNeedPose)
    {
        printf("Assume calibration pose\n");
    }
    XnUInt32 epochTime = 0;


    if(argc>1)
    {
        //app.setMeshFilename(argv[1]);
    }
    else
    {
        //app.setMeshFilename("bras_sans_smooth_III_2308..mesh");
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
