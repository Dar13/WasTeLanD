#include "StdAfx.h"

#include "CrowdManager.h"

#ifndef _CROWD_AGENT_H_
#define _CROWD_AGENT_H_

class Character
{
public:
	Character(const std::string& name, Ogre::SceneManager* scene, CrowdManager* crowd, const Ogre::Vector3& position);
	Character(Ogre::SceneNode* node,CrowdManager* crowd,const Ogre::Vector3& position);

	Ogre::SceneNode* getNode() { return _node; }

	Ogre::Entity* getEntity() { return _entity; }

	void update(float deltaTime);

private:
	float _destRadius;

	std::string name;
	Ogre::SceneNode* _node;
	Ogre::Entity* _entity;

	CrowdManager* _crowd;
	
	dtCrowdAgent* _agent;
	int _agentID;

	Ogre::Vector3 _destination;

	Ogre::Vector3 _manualVelocity;

	bool _isStopped;

	bool _isAgentControlled;
	
	//Not sure how to integrate Bullet into all this. Ghost collision similar to the player character?
	//btRigidBody* _rigidBody;
};

#endif