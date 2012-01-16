#include "StdAfx.h"
#include "GameManager.h"
#include "debug\print.h"

//=======================================
/*
(!!!)NOTE(!!!)
	Bullet units are 1 unit = 1 meter.
	So that means that OGRE units are now 1 unit = 1 meter.
*/
//=======================================

template<> GameManager* Ogre::Singleton<GameManager>::ms_Singleton = 0;

GameManager::GameManager()
{
	_config = NULL;
	_charCamera = NULL;
	_charNode = NULL;
	_charGhost = NULL;
	_charController = NULL;
	_dummyNode = NULL;
	oldTime = 0;
	deltaTime = 0;
	Time = (float)OgreManager::getSingleton().getTimer()->getMilliseconds();
}

GameManager::~GameManager()
{
	//destroy whatever I initialized(if anything) in the constructor.
	if(_config)
	{
		delete _config;
	}
}

bool GameManager::UpdateManagers()
{
	bool retVal = true;
	//Update OIS
	OISManager::getSingleton().capture();
	retVal = !OISManager::getSingleton().escapePressed();

	//Update Bullet
	oldTime = Time;
	Time = (float)OgreManager::getSingleton().getTimer()->getMilliseconds();
	deltaTime = (Time - oldTime)/1000.0f;
	//update character controller if _charCamera is set
	if(_charCamera)
	{
		updateCharacterController(deltaTime,0);
	}
	BulletManager::getSingleton().Update(deltaTime);

	//if the debug printer is active, update it
	if(DebugPrint::getSingleton().isActive())
	{
		DebugPrint::getSingleton().Update();
	}

	//Update Ogre
	Ogre::WindowEventUtilities::messagePump();
	//If an error occurred...
	if(!OgreManager::getSingleton().Render())
	{
		//state needs to end
		retVal=false;
	}

	return retVal;
}

void GameManager::loadConfiguration(std::string& file)
{
	_config = configuration(file).release();
	OISManager::getSingleton().setConfiguration(_config);
}

configuration_t* GameManager::getConfiguration()
{
	return _config;
}

void GameManager::useDebugDrawer(Ogre::SceneManager* scene)
{
	CDebugDraw* draw = new CDebugDraw(scene,BulletManager::getSingleton().getWorld());
	BulletManager::getSingleton().setDebugDrawer(draw);
}

void GameManager::createCharacterController(Ogre::Camera* camera,Ogre::Vector3 initPosition)
{
	//sets up a variable that holds the character controller's initial position.
	btTransform initial;
	initial.setIdentity();
	initial.setOrigin(btVector3(initPosition.x,initPosition.y,initPosition.z));

	//this object detects the collisions for the character controller
	_charGhost = new btPairCachingGhostObject();
	_charGhost->setWorldTransform(initial);

	//create collision shape
	BulletManager::getSingleton().getWorld()->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	btScalar charHeight = .9f;
	btScalar charWidth = .3f;
	btConvexShape* capsule = new btCapsuleShape(charWidth,charHeight);
	_charGhost->setCollisionShape(capsule);
	_charGhost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	btScalar stepHeight = 0.35f;
	_charController = new btKinematicCharacterController(_charGhost,capsule,stepHeight);

	//adding the ghost and character controller to the physics world
	BulletManager::getSingleton().getWorld()->addCollisionObject(_charGhost,btBroadphaseProxy::CharacterFilter,btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	BulletManager::getSingleton().getWorld()->addAction(_charController);

	//Stores the current camera that the character controller is controlling.
	_charCamera = camera;
	//also attaches it to a scenenode, so that other objects can be oriented around the camera.
	_charNode = camera->getSceneManager()->getRootSceneNode()->createChildSceneNode("charCameraNode");
	_charNode->setPosition(_charCamera->getPosition());
	_charNode->setOrientation(_charCamera->getOrientation());
	_charNode->attachObject(_charCamera);

	_dummyNode = _charNode->createChildSceneNode("dummyNode",Ogre::Vector3::UNIT_X);
	_dummyNode->setPosition(0.5f,1.5f,0.5f);

	//_charCamera->setAutoTracking(true,_dummyNode);

	_charController->setGravity(btScalar(9.8f));
	//doesn't do anything...
	//_charController->setMaxJumpHeight(btScalar(.5f));
	_charController->setJumpSpeed(btScalar(6.0f));

	return;
}

void GameManager::updateCharacterController(float phyTime,Ogre::Camera* camera)
{
	//update bullet character controller and apply transformations to ogre camera
	//if no camera is available, then don't apply to a camera
	if(!camera && !_charCamera)
	{
		//for now return
		return;
	}
	//if a camera is passed in, but there isn't one set already then set the internal camera to the passed-in one.
	if(camera && !_charCamera)
	{
		_charCamera = camera;
	}

	//update the bullet character controller
	btTransform camTrans = _charGhost->getWorldTransform();
	btVector3 forDir = camTrans.getBasis()[0]; forDir.normalize();
	btVector3 upDir = camTrans.getBasis()[1]; upDir.normalize();
	//let's halve the upDir
	upDir /= 2;
	btVector3 strafeDir = camTrans.getBasis()[2]; strafeDir.normalize();
	btVector3 walkDir = btVector3(0,0,0);
	btScalar walkVel = btScalar(2.0f) * 4.0f;
	btScalar walkSpd = walkVel * phyTime;

	Ogre::Vector3 dir;
	Ogre::Quaternion rot;
	//This is creates a vector that moves along the quat.
	//Refer to Ogre::Node::translate for formula.
	//basically is:
	// direction vector = orientation * (orientation * unit vector)
	if(OISManager::getSingleton().isCFGKeyPressed(FORWARD))
	{
		walkDir += forDir;
	}

	if(OISManager::getSingleton().isCFGKeyPressed(BACKWARD))
	{
		walkDir -= forDir;
	}

	if(OISManager::getSingleton().isCFGKeyPressed(RIGHT))
	{
		walkDir += strafeDir;
	}

	if(OISManager::getSingleton().isCFGKeyPressed(LEFT))
	{
		walkDir -= strafeDir;
	}

	if(OISManager::getSingleton().isCFGKeyPressed(JUMP))
	{
		//seems like it is implemented. At least partially.
		_charController->jump();
	}

	dir = convertBulletVector3(walkDir);
	rot = _charNode->getOrientation();
	rot.z = 0.0f; //this is needed to prevent the z-component interfering with the movement.
	dir = rot * (rot * dir);
	walkDir = convertOgreVector3(dir);

	//mouse-look code.
	int mmx,mmy;
	mmx = OISManager::getSingleton().getMMX();
	mmy = OISManager::getSingleton().getMMY();
	//going to try only y-axis for now(mmx only)
	//this code works.
	btMatrix3x3 yorn = camTrans.getBasis();
	yorn *= btMatrix3x3(btQuaternion(btVector3(0,1,0),(-mmx * 0.007f)));
	_charGhost->getWorldTransform().setBasis(yorn);

	//x-axis?
	//will only apply to scene node, not bullet physics.
	//have to get current x-rotation from node, not bullet
	//this works.
	Ogre::Quaternion quat;
	Ogre::Radian angle;

	angle = (-mmy) * 0.007f;
	quat.FromAngleAxis(angle,Ogre::Vector3::UNIT_Z);

	Ogre::Quaternion zrestrict(Ogre::Radian(Ogre::Math::PI),Ogre::Vector3::UNIT_Z);
	//Ogre::Quaternion znegrestrict(Ogre::Radian(Ogre::Math::PI),Ogre::Vector3::UNIT_Z);
	
	//this works.
	_charController->setWalkDirection(walkDir * walkSpd);

	//convert bullet position/rotation to ogre position rotation.
	Ogre::Quaternion oRot; btQuaternion btRot;
	//need to multiply by xRot to get x-axis mouse-look rotations.
	btRot = _charGhost->getWorldTransform().getRotation();
	oRot = convertBulletQuat(btRot);
	Ogre::Vector3 oPos = convertBulletVector3(_charGhost->getWorldTransform().getOrigin());

	//update camera position/rotation
	//uses scene node created in createCharacterController().
	_charNode->setPosition(oPos);
	Ogre::Quaternion zRot = _charNode->getOrientation();
	zRot.x=0;
	zRot.y=0;
	//attempt at limiting mouselook up/down
	Ogre::Quaternion testQ = zRot * zrestrict;
	DebugPrint::getSingleton().printVar(testQ.w);
	DebugPrint::getSingleton().printVar(testQ.x);
	DebugPrint::getSingleton().printVar(testQ.y);
	DebugPrint::getSingleton().printVar(zRot.z);

	bool extraRotate = true;
	if(zRot.z > .5f)
	{
		extraRotate = false;
		//just in case it jumps around
		zRot.z = .5f;
	}
	if(zRot.z < -.5f)
	{
		extraRotate = false;
		//just in case it jumps around.
		zRot.z = -.5f;
	}


	_charNode->setOrientation(oRot);
	_charNode->rotate(zRot); //original z-rotation
	if(extraRotate)
	{
		_charNode->rotate(quat); //then the new z-rotation.
	}

}

void loadWeapons(std::string file)
{
	list_t* wepList = list(file).release();
	for(unsigned int i=0; i<wepList->file().size(); ++i)
	{

	}
}

OgreBulletPair GameManager::createObject(Ogre::SceneManager* scene,object_t* objectInfo)
{
	//return variable
	OgreBulletPair retVal;

	//Ogre part of this arrangement.
	Ogre::SceneNode* node = OgreManager::getSingleton().createSceneNode(scene,objectInfo);
	retVal.ogreNode = node;

	btCollisionShape* shape = NULL;

	//Only do this if it's an object.
	if(objectInfo->type() == "entity")
	{
		if(objectInfo->mass()!=0.0f)
		{
			//get the collision shape.
			shape = BulletManager::getSingleton().generateCollisionShape(objectInfo);
		}
		else
		{
			//assumes that all non-mass objects will be static triangle meshes.
			//unless otherwise told
			if(objectInfo->collisionShape() == "TriangleMesh")
			{
				shape = buildTriangleCollisionShape(node);
			}
			else
			{
				shape = BulletManager::getSingleton().generateCollisionShape(objectInfo);
			}
		}
		
		btTransform init;
		init.setIdentity();
		init.setOrigin(btVector3(objectInfo->positionX(),objectInfo->positionY(),objectInfo->positionZ()));

		//Easy function call.
		retVal.btBody=BulletManager::getSingleton().addRigidBody(shape,node,objectInfo->mass(),init);
	}


	//return by-value, not reference.
	return retVal;
}

OgreBulletPair GameManager::createObject(Ogre::SceneNode* node,object_t* objectInfo)
{
	//Here's the variable that'll be passed back(by-value, not reference!).
	OgreBulletPair retVal;

	//That's dead simple, it's already passed in!
	retVal.ogreNode=node;

	//Easy(ish) function call.
	btCollisionShape* shape = NULL;
	if(objectInfo->mass()!=0.0f)
	{
		shape = BulletManager::getSingleton().generateCollisionShape(objectInfo);
	}
	else
	{
		shape = buildTriangleCollisionShape(node);
	}
	btTransform init; init.setIdentity();
	btVector3 pos;
	pos.setX(objectInfo->positionX());
	pos.setY(objectInfo->positionY());
	pos.setZ(objectInfo->positionZ());
	retVal.btBody = BulletManager::getSingleton().addRigidBody(shape,node,objectInfo->mass(),init);

	return retVal;
}

//----------------------------------------
//Private, utility functions start here..
//----------------------------------------

//converts bullet vector3 to ogre vector3
Ogre::Vector3 GameManager::convertBulletVector3(const btVector3 &v)
{
	return Ogre::Vector3(v.x(),v.y(),v.z());
}

//converts ogre vector3 to bullet vector3
btVector3 GameManager::convertOgreVector3(const Ogre::Vector3 &v)
{
	return btVector3(v.x,v.y,v.z);
}

Ogre::Quaternion GameManager::convertBulletQuat(const btQuaternion &q)
{
	return Ogre::Quaternion(q.w(),q.x(),q.y(),q.z());
}

btQuaternion GameManager::convertOgreQuat(const Ogre::Quaternion &q)
{
	return btQuaternion(q.x,q.y,q.z,q.w);
}

Ogre::Matrix3 GameManager::convertBulletMatrix3(const btMatrix3x3 &m)
{
	Ogre::Matrix3 mat;
	mat.SetColumn(0,convertBulletVector3(m[0]));
	mat.SetColumn(1,convertBulletVector3(m[1]));
	mat.SetColumn(2,convertBulletVector3(m[2]));
	return mat;
}

btMatrix3x3 GameManager::convertOgreMatrix3(const Ogre::Matrix3 &m)
{
	btMatrix3x3 mat;
	Ogre::Quaternion quat;
	quat.FromRotationMatrix(m);
	mat.setRotation(convertOgreQuat(quat));
	return mat;
}

//manually builds triangle mesh collision shape.
btBvhTriangleMeshShape* GameManager::buildTriangleCollisionShape(Ogre::SceneNode* node)
{
	btBvhTriangleMeshShape* shape;
	btTriangleMesh* bmesh = new btTriangleMesh();

	Ogre::MeshPtr mesh = ((Ogre::Entity*)node->getAttachedObject(0))->getMesh();
	size_t vertCnt,inCnt;
	Ogre::Vector3* vertices;
	unsigned long* indices;

	OgreManager::getSingleton().getMeshInformation(&mesh,vertCnt,vertices,inCnt,indices);

	btVector3 vert1,vert2,vert3;

	for(unsigned int i=0; i<inCnt; i+=3)
	{
		vert1.setValue(vertices[indices[i]].x,vertices[indices[i]].y,vertices[indices[i]].z);
		vert2.setValue(vertices[indices[i+1]].x,vertices[indices[i+1]].y,vertices[indices[i+1]].z);
		vert3.setValue(vertices[indices[i+2]].x,vertices[indices[i+2]].y,vertices[indices[i+2]].z);

		bmesh->addTriangle(vert1,vert2,vert3);
	}

	shape = new btBvhTriangleMeshShape(bmesh,true,true);

	delete[] vertices;
	delete[] indices;

	return shape;
}