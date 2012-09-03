#include "pointcloud.h"
#include <OgreAxisAlignedBox.h>

using namespace std;

PointCloud::PointCloud(const std::string& name,
                       const std::string& resourcegroup,
                       const int numpoints,
                       float *parray,
                       float *carray) : ManualObject(name)

{
   mMeshPtr = Ogre::MeshManager::getSingleton().createManual(name, resourcegroup);
   Ogre::SubMesh* sub = mMeshPtr->createSubMesh();
   mMeshPtr->sharedVertexData = new Ogre::VertexData();
   mMeshPtr->sharedVertexData->vertexCount = numpoints;
   Ogre::VertexDeclaration* decl = mMeshPtr->sharedVertexData->vertexDeclaration;
   decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
   mVertBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
               decl->getVertexSize(0),
               mMeshPtr->sharedVertexData->vertexCount,
               Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

   /// Upload the vertex data to the card
   mVertBuf->writeData(0, mVertBuf->getSizeInBytes(), parray, true);

   if(carray != NULL)

   {
      // Create 2nd buffer for colors

      decl->addElement(1, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
      mColBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                  Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR),
                  mMeshPtr->sharedVertexData->vertexCount,
                  Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


      Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
      Ogre::RGBA *colours = new Ogre::RGBA[numpoints];
      for(int i=0, k=0; i<numpoints*3, k<numpoints; i+=3, k++)
      {
         // Use render system to convert colour value since colour packing varies
         rs->convertColourValue(Ogre::ColourValue(carray[i],carray[i+1],carray[i+2]), &colours[k]);
      }
      // Upload colour data
      mColBuf->writeData(0, mColBuf->getSizeInBytes(), colours, true);
      delete[] colours;
   }

   /// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
   Ogre::VertexBufferBinding* bind = mMeshPtr->sharedVertexData->vertexBufferBinding;
   bind->setBinding(0, mVertBuf);


   if(carray != NULL)
   {
      // Set colour binding so buffer 1 is bound to colour buffer
      bind->setBinding(1, mColBuf);
   }

   sub->useSharedVertices = true;
   sub->operationType = Ogre::RenderOperation::OT_LINE_STRIP;

   mMeshPtr->load();
   sub->_getRenderOperation(rend);
}



void PointCloud::updateVertexPositions(int size, float *points)
{
   float *pPArray = static_cast<float*>(mVertBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

   for(int i=0; i<size*3; i+=3)
   {
      pPArray[i] = points[i];
      pPArray[i+1] = points[i+1];
      pPArray[i+2] = points[i+2];
   }
   mVertBuf->unlock();

   mMeshPtr->_setBoundingSphereRadius(10000);
   /*mMeshPtr->_setBounds(Ogre::AxisAlignedBox(-10e10, -10e10, -10e10,
                                         10e10,  10e10,  10e10));*/

}



void PointCloud::updateVertexColours(int size, float *colours)
{
   float *pCArray = static_cast<float*>(mColBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

   for(int i=0; i<size*3; i+=3)
   {
      pCArray[i] = colours[i];
      pCArray[i+1] = colours[i+1];
      pCArray[i+2] = colours[i+2];
   }
   mColBuf->unlock();
}

