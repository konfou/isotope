/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include <stdio.h>
#include "TestBigNumber.h"
#include "../../../GClasses/GBigNumber.h"
#include "../../../GClasses/GKeyPair.h"
#include "../../../GClasses/GRand.h"
#include "../ClassTests.h"
#include <time.h>

bool TestRSA()
{
	time_t t;
	unsigned int seed[2];
	seed[0] = time(&t);
	seed[1] = time(&t);
	GRand rand((unsigned char*)seed, 2 * sizeof(unsigned int));
	GKeyPair kp;
	kp.GenerateKeyPair(&rand);

	// Make up a message
	GBigNumber message;
	message.SetUInt(0, 0x6a54);

	// Encrypt it
	GBigNumber cypher;
	cypher.PowerMod(&message, kp.GetPrivateKey(), kp.GetN());

	// Decrypt it
	GBigNumber final;
	final.PowerMod(&cypher, kp.GetPublicKey(), kp.GetN());

	// Check the final value
	if(final.CompareTo(&message) != 0)
		return false;
	return true;
}

bool TestGBigNumber(ClassTests* pThis)
{
	if(!TestRSA())
		return false;
	return true;
}

