#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <string>
#include <OgreHardwareVertexBuffer.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreManualObject.h>


class PointCloud : public Ogre::ManualObject
{
public:
    PointCloud(const std::string& name,
               const std::string& resourcegroup,
               const int numpoints,
               float *parray,
               float *carray);

    void updateVertexPositions(int size, float *points);
    void updateVertexColours(int size, float *colours);


private:

   int mSize;
   Ogre::HardwareVertexBufferSharedPtr mVertBuf;
   Ogre::HardwareVertexBufferSharedPtr mColBuf;
   Ogre::MeshPtr mMeshPtr;


};

#endif // POINTCLOUD_H

