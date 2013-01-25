#include "prosthesis.h"
#include <OgreAxisAlignedBox.h>

using namespace std;

void Prosthesis::load(Ogre::SceneManager * mgr)
{
  Ogre::SceneNode * rootNode;
  rootNode = mgr->getRootSceneNode();
  mEntity = mgr->createEntity("Entity_"+mFilename, mFilename);
  mNode = rootNode->createChildSceneNode("Node_"+mFilename);
  mNode->attachObject(mEntity);
  mEntity->setDisplaySkeleton(false);

  Ogre::AxisAlignedBox aabb = Ogre::AxisAlignedBox(-10e10, -10e10, -10e10,
                                                    10e10,  10e10,  10e10);
  mEntity->getMesh()->_setBounds(aabb);
}

bool Prosthesis::transformBone(std::string boneName,
                               XnSkeletonJoint jointName,
                               XnUserID userId,
                               bool inheritsScale=true,
                               bool inheritOrientation=true,
                               bool updatePosition=true,
                               bool updateOrientation=true)
{
  Ogre::Bone *bone;
  Ogre::Vector3 v;
  XnPoint3D xnv;
  XnSkeletonJointTransformation joint;
  bool retVal=false;

  bone = mEntity->getSkeleton()->getBone(boneName);
  bone->setManuallyControlled(true);
  bone->setInheritOrientation(inheritOrientation);
  bone->setInheritScale(inheritsScale);

  mSkel->GetSkeletonJoint(userId, jointName,joint);
  mDepthGen->ConvertRealWorldToProjective(1,&(joint.position.position), &xnv);
  v = Ogre::Vector3(xnv.X, -xnv.Y, xnv.Z);
  if((updateOrientation)&&(joint.orientation.fConfidence>0.5f))
  {
     bone->resetToInitialState();
      Ogre::Quaternion quat;
      Ogre::Quaternion qI = Ogre::Quaternion::IDENTITY;
      float* matE = joint.orientation.orientation.elements;
      Ogre::Matrix3 matOri = Ogre::Matrix3(  matE[0], matE[1], matE[2],
                                             matE[3], matE[4], matE[5],
                                             matE[6], matE[7], matE[8]);
      quat.FromRotationMatrix(matOri);
      bone->resetOrientation();
      quat = bone->convertWorldToLocalOrientation(quat);
      bone->setOrientation(quat*qI);
      retVal=true;
  }
  if((updatePosition)&&(joint.position.fConfidence>0.5f))
  {
    bone->setPosition(mTransf.inverse() * v);
  }
  mTransf = mNode->_getFullTransform() * bone->_getFullTransform();
  return retVal;
}

void Prosthesis::update(float dT)
{
  Ogre::Material *mat;
  if((dT<mStartTime)||(dT>mStartTime+mDuration+mFadeinDuration+mFadeoutDuration))
  {
      this->hide();
      return;
  }
  this->show();

  float alpha;
  bool transparency=true;
  if(dT-mStartTime<mFadeinDuration)
  {
      alpha = (dT-mStartTime)/mFadeinDuration;
      transparency=true;
  }
  else if((dT-mStartTime-mDuration-mFadeinDuration<mFadeoutDuration)&&
          (dT-mStartTime-mFadeinDuration>=mDuration))
  {
      alpha = 1.0-(dT-mStartTime-mDuration-mFadeinDuration)/mFadeoutDuration;
      transparency=true;
  }
  else
  {
      alpha=1.0;
      transparency=false;
  }

  for(int i=0;i<mEntity->getNumSubEntities();i++)
  {
    if(transparency)
    {
      mat = mEntity->getSubEntity(i)->getMaterial().getPointer();
      mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
      mat->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_CLOCKWISE);
      mat->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    }
    else
    {
      mat = mEntity->getSubEntity(i)->getMaterial().getPointer();
      mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);
      mat->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);
      mat->getTechnique(0)->getPass(0)->setDepthWriteEnabled(true);
    }

    if(mat->getTechnique(0)->getPass(0)->getNumTextureUnitStates()>0)
    {
          mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha);
    }
    else
    {
      Ogre::ColourValue cv=mat->getTechnique(0)->getPass(0)->getDiffuse();
      cv.a = alpha;
      mat->setDiffuse(cv);
    }
  }
}

void Prosthesis::updateAllJoints(unsigned long dt,
                        xn::SkeletonCapability *sc,
                        xn::DepthGenerator *dg,
                        XnUserID user)
{
  float seconds = dt / 1000.0;
  // Manage visibility
  /*if((dt<mStartTime)||(dt>mStartTime+mDuration+mFadeinSpeed+mFadeoutSpeed))
  {
    mEntity->setVisible(false);
    mNode->setVisible(false);
    return;
  }
  else
  {
    mEntity->setVisible(true);
    mNode->setVisible(true);
  }*/
  //if(dt<mStartTime+mFadeinSpeed)
  {
    //double alpha = (dt-mStartTime)/mFadeinSpeed;
    /*double alpha = fabs(cos(dt));
    Ogre::Material * mat = mEntity->getSubEntity(0)->getMaterial().get();
    Ogre::Pass *myPass = mat->getTechnique(0)->getPass(0);
    myPass->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL, Ogre::LBS_CURRENT, 1.0, 1.0, alpha);*/
  }

  // Manage position and pose
  Ogre::Bone * bone;

  mNode->setPosition(0,0,0);;

  mSkel = sc;
  mDepthGen = dg;
  mTransf = mNode->_getFullTransform();
  if(mType==1)    // Realistic arm
  {
    bone = mEntity->getSkeleton()->getBone("epaule_");
    if(transformBone("epaule_", XN_SKEL_NECK, user, false, false,true, true))
    {
      bone->setScale(500,500,500);
      bone->translate(Ogre::Vector3(-5, 50, 33), Ogre::Node::TS_LOCAL);
      //bone->translate(dbgT, Ogre::Node::TS_LOCAL);

      bone->yaw(Ogre::Radian(Ogre::Degree(90)));
      bone->pitch(Ogre::Radian(Ogre::Degree(-51)));
      bone->roll(Ogre::Radian(Ogre::Degree(-90)));

    }
    transformBone("bras_", XN_SKEL_RIGHT_SHOULDER,user,true,true,false,true);
    transformBone("avant_bras_", XN_SKEL_RIGHT_ELBOW,user,true,true,false,true);
    transformBone("poignet_", XN_SKEL_RIGHT_HAND,user,true,true,false,true);
  }
  else if(mType==2) //Mechanical arm
  {
    bone = mEntity->getSkeleton()->getBone("epaule_");
    if(transformBone("epaule_", XN_SKEL_NECK, user, false, false,true, true))
    {
      bone->setScale(500,500,500);
      bone->translate(Ogre::Vector3(55, 10, 23), Ogre::Node::TS_LOCAL);
      bone->yaw(Ogre::Radian(Ogre::Degree(90)));
      bone->pitch(Ogre::Radian(Ogre::Degree(-51)));
      bone->roll(Ogre::Radian(Ogre::Degree(-90)));
    }
    transformBone("bras", XN_SKEL_RIGHT_SHOULDER,user,true,true,false,true);
    transformBone("avant_bras", XN_SKEL_RIGHT_ELBOW,user,true,true,false,true);
    transformBone("main", XN_SKEL_RIGHT_HAND,user,true,true,false,true);
  }
  else if(mType==3)   // Heart
  {
    if(transformBone("Bone001", XN_SKEL_NECK, user, false, false,true, true))
    {
      bone = mEntity->getSkeleton()->getBone("Bone001");
      bone->translate(Ogre::Vector3(-5, -120, 3), Ogre::Node::TS_LOCAL);
      bone->yaw(Ogre::Radian(Ogre::Degree(160)));
      bone->pitch(Ogre::Radian(Ogre::Degree(-12)));
      bone->roll(Ogre::Radian(Ogre::Degree(98)));
    }
    bone = mEntity->getSkeleton()->getBone("Bone007");
    double s = cos(dt/500.0)*0.2+1.1;
    bone->setScale(s,s,s);
    bone->setManuallyControlled(true);
    bone->setInheritScale(false);
  }
  mNode->setPosition(dbgT);
}

void Prosthesis::hide()
{
  mEntity->setVisible(false);
  mNode->setVisible(false);
}

void Prosthesis::show()
{
  mEntity->setVisible(true);
  mNode->setVisible(true);
}
