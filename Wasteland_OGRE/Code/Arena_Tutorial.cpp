#include "Arena_Tutorial.h"
#include "GameManager.h"

#include <Ogre.h>

ArenaTutorial::ArenaTutorial()
{
	_camera = 0;
	_scene = 0;
	_rootNode = 0;
}

void ArenaTutorial::Setup()
{
	_scene = OgreManager::getSingleton().getRoot()->createSceneManager(Ogre::ST_INTERIOR,"arenaTut");

	_camera = _scene->createCamera("arenaTutCam");
	_view = OgreManager::getSingleton().getRenderWindow()->addViewport(_camera);
	_view->setBackgroundColour(Ogre::ColourValue(0,0,0));

	_rootNode = _scene->getRootSceneNode();

	//some special physics setup
	BulletManager::getSingleton().setGravity(btVector3(0,-9.8f,0));
	
	//Loading the level through the GameManager(takes care of Bullet initialization,etc)
	std::map<std::string,std::string> opt; 
	opt.insert(std::make_pair("name","arenaLevel"));
	opt.insert(std::make_pair("filename","test_level.mesh"));
	opt.insert(std::make_pair("resgroup","Models"));
	Ogre::SceneNode* level = GameManager::getSingleton().createLevel(_scene,opt);

	//test node;
	Ogre::Entity* sphereEnt = _scene->createEntity("sphere","test_sphere.mesh","Models");
	Ogre::SceneNode* sphereNode = _rootNode->createChildSceneNode("sphere");
	sphereNode->attachObject(sphereEnt);
	btCollisionShape* sphereCol = new btSphereShape(5.0f);
	btScalar mass = 10.0f;
	btTransform sphereT; sphereT.setIdentity(); sphereT.setOrigin(btVector3(150,150,145));
	OgreBulletPair spherePair = GameManager::getSingleton().createObject(sphereNode,sphereCol,mass,sphereT);

	//Let's position the camera so we can see it.
	_camera->setPosition(Ogre::Vector3(0,500,500));
	_camera->lookAt(0,0,0);
	//set the camera aspect ratio
	_camera->setAspectRatio(4.0f/3.0f);

}

int ArenaTutorial::Run()
{
	_stateShutdown=false;

	float time,oldTime,dTime;
	time = OgreManager::getSingleton().getTimer()->getMilliseconds();
	oldTime = OgreManager::getSingleton().getTimer()->getMilliseconds();
	dTime = (time - oldTime)/1000;

	//while the escape key isn't pressed and the state isn't told to shutdown.
	while(!OISManager::getSingleton().escapePressed() && !_stateShutdown)
	{
		//Capture input.
		OISManager::getSingleton().capture();
		
		//Run the message pump
		Ogre::WindowEventUtilities::messagePump();
		
		oldTime = time;
		time = OgreManager::getSingleton().getTimer()->getMilliseconds();
		dTime = (time-oldTime)/1000.0f;

		BulletManager::getSingleton().Update(dTime);

		//Have Ogre render a frame.
		if(!OgreManager::getSingleton().Render())
		{
			//exception occurred, need to exit as cleanly as possible.
			_stateShutdown=true; 
		}
	}

	//no matter what, end the program after this state.
	return END;
}

void ArenaTutorial::Shutdown()
{
	//clean up what you initialized in the Setup() function.
	OgreManager::getSingleton().getRenderWindow()->removeAllViewports();
	
	//Destroy the camera, since we've taken care of the viewport.
	_scene->destroyCamera(_camera);
	//Destroy all SceneNodes, Entites, etc.
	_scene->clearScene();

	//Set to null.
	_rootNode=0;

	//Clear the vectors
	_nodes.clear();
	_entities.clear();

	//Destroy the scene manager.
	OgreManager::getSingleton().getRoot()->destroySceneManager(_scene);
}