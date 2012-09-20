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

void Prosthesis::transformBone(std::string boneName,
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
  bone = mEntity->getSkeleton()->getBone(boneName);
  bone->setManuallyControlled(true);
  bone->setInheritOrientation(inheritOrientation);
  bone->setInheritScale(inheritsScale);
  bone->resetToInitialState();

  mSkel->GetSkeletonJoint(userId, jointName,joint);
  mDepthGen->ConvertRealWorldToProjective(1,&(joint.position.position), &xnv);
  v = Ogre::Vector3(xnv.X, -xnv.Y, xnv.Z);
  if((updateOrientation)&&(joint.orientation.fConfidence>0.5f))
  {
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
  }
  if(updatePosition)
  {
    bone->setPosition(mTransf.inverse() * v);
  }
  mTransf = mNode->_getFullTransform() * bone->_getFullTransform();
}

void Prosthesis::updateAllJoints(unsigned long dt,
                        xn::SkeletonCapability *sc,
                        xn::DepthGenerator *dg,
                        XnUserID user)
{
  mSkel = sc;
  mDepthGen = dg;
  mTransf = mNode->_getFullTransform();
  if(mType==1)
  {
    Ogre::Bone * bone = mEntity->getSkeleton()->getBone("epaule_");
    transformBone("epaule_", XN_SKEL_NECK, user, false, false,true, true);
    bone->setScale(500,500,500);
    static int dbgi=0;
    dbgi++;
    bone->yaw(Ogre::Radian(Ogre::Degree(90+dbgi)));
    bone->pitch(Ogre::Radian(Ogre::Degree(-51)));
    bone->roll(Ogre::Radian(Ogre::Degree(-90)));


    transformBone("bras_", XN_SKEL_RIGHT_SHOULDER,user,true,true,false,true);
    transformBone("avant_bras_", XN_SKEL_RIGHT_ELBOW,user,true,true,false,true);
    transformBone("poignet_", XN_SKEL_RIGHT_HAND,user,true,true,false,true);
  }
}
