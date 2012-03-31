#include "StdAfx.h"
#include "Utility.h"

Ogre::Vector3 Utility::convert_btVector3(const btVector3& v)
{
	return Ogre::Vector3(v.x(),v.y(),v.z());
}
btVector3 Utility::convert_OgreVector3(const Ogre::Vector3& v)
{
	return btVector3(v.x,v.y,v.z);
}
Ogre::Quaternion Utility::convert_btQuaternion(const btQuaternion& q)
{
	return Ogre::Quaternion(q.w(),q.x(),q.y(),q.z());
}
btQuaternion Utility::convert_OgreQuaternion(const Ogre::Quaternion& q)
{
	return btQuaternion(q.x,q.y,q.z,q.w);
}

Ogre::Matrix3 Utility::convert_btMatrix3(const btMatrix3x3 &m)
{
	Ogre::Matrix3 mat;
	mat.SetColumn(0,Utility::convert_btVector3(m[0]));
	mat.SetColumn(1,Utility::convert_btVector3(m[1]));
	mat.SetColumn(2,Utility::convert_btVector3(m[2]));
	return mat;
}

btMatrix3x3 Utility::convert_OgreMatrix3(const Ogre::Matrix3 &m)
{
	btMatrix3x3 mat;
	Ogre::Quaternion quat;
	quat.FromRotationMatrix(m);
	mat.setRotation(Utility::convert_OgreQuaternion(quat));
	return mat;
}