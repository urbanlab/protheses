#include "infoviz.h"

void Infoviz::load(Ogre::SceneManager *mgr)
{
    mRectangle  = new Ogre::Rectangle2D(true);
    mRectangle->setCorners(this->x, this->y,
                           this->x+this->width, this->y+this->height);
    mRectangle->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
    mRectangle->setMaterial("infoviz_1_material");

    mNode = mgr->getRootSceneNode()->createChildSceneNode("Infoviz"+mFilename);
    mNode->attachObject(mRectangle);
    this->hide();
}

void Infoviz::update(float dT)
{
    Ogre::Material *mat;
    if((dT<mStartTime)||(dT>mStartTime+mDuration+mFadeinDuration+mFadeoutDuration))
    {
        this->hide();
        return;
    }
    this->show();

    float alpha;
    if(dT-mStartTime<mFadeinDuration)
    {
        alpha = (dT-mStartTime)/mFadeinDuration;
    }
    else if(dT-mStartTime-mDuration-mFadeinDuration<mFadeoutDuration)
    {
        alpha = 1.0-(dT-mStartTime-mDuration-mFadeinDuration)/mFadeoutDuration;
    }
    else
    {
        alpha=1.0;
    }

    mat = mRectangle->getMaterial().getPointer();
    mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mat->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha);
}

void Infoviz::hide()
{
  mRectangle->setVisible(false);
  mNode->setVisible(false);
}

void Infoviz::show()
{
  mRectangle->setVisible(true);
  mNode->setVisible(true);
}
