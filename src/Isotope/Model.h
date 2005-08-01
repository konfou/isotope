/*
	Copyright (C) 2006, Edumetrics Institute

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/gpl.html
*/

#ifndef __MODEL_H__
#define __MODEL_H__

class MRealm;
class GXMLTag;
class MAvatar;
class NRealmServerConnection;
class NRealmClientConnection;
class NRealmPacket;
class NSetPathPacket;
class NSendMeUpdatesPacket;
class NUpdateObjectPacket;
class MObject;
class Controller;
class View;

// This is the base class for the client and server models.  Anything common between them should
// go in this class.
class Model
{
public:
	enum ModelType
	{
		Server,
		Client,
		KeyGenerator,
	};

	Model();
	virtual ~Model();

	virtual void Update(double time) = 0;

	// Whenever the MRealm replaces an object, it will call this method
	// to inform the model about the change.  If the model is holding any
	// pointers to old objects then it needs to update them to point to
	// the new objects.  The server uses this hook as an opportunity to
	// verify that the new object doesn't violate any laws of physics.  If
	// it returns false, then the replacement is abandoned.
	virtual bool OnReplaceObject(int nConnection, MObject* pOld, MObject* pNew) = 0;

	virtual ModelType GetType() = 0;

protected:

};


#endif // __MODEL_H__
