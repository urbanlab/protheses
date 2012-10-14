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
}
