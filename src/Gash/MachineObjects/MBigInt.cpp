/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "MBigInt.h"
#include "MachineObjects.h"
#include "../../GClasses/GHashTable.h"
#include "../Include/GashLib.h"

void RegisterMBigInt(GConstStringHashTable* pTable)
{
	pTable->Add("method &add(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::add));
	pTable->Add("method !new()", new EMethodPointerHolder((MachineMethod0)&MBigInt::allocate));
	pTable->Add("method &and(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::And));
	pTable->Add("method compareTo(&Bool, BigInt)", new EMethodPointerHolder((MachineMethod2)&MBigInt::compareTo));
	pTable->Add("method &copy(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::copy));
	pTable->Add("method &decrement()", new EMethodPointerHolder((MachineMethod0)&MBigInt::decrement));
	pTable->Add("method &divide(BigInt, BigInt, &BigInt)", new EMethodPointerHolder((MachineMethod3)&MBigInt::divide));
	pTable->Add("method &euclid(BigInt, BigInt, &BigInt, &BigInt)", new EMethodPointerHolder((MachineMethod4)&MBigInt::euclid));
	pTable->Add("method &fromHex(String)", new EMethodPointerHolder((MachineMethod1)&MBigInt::fromHex));
	pTable->Add("method getBit(&Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MBigInt::getBit));
	pTable->Add("method getBitCount(&Integer)", new EMethodPointerHolder((MachineMethod1)&MBigInt::getBitCount));
	pTable->Add("method getSign(&Bool)", new EMethodPointerHolder((MachineMethod1)&MBigInt::getSign));
	pTable->Add("method getUInt(&Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MBigInt::getUInt));
	pTable->Add("method getUIntCount(&Integer)", new EMethodPointerHolder((MachineMethod1)&MBigInt::getUIntCount));
	pTable->Add("method &increment()", new EMethodPointerHolder((MachineMethod0)&MBigInt::increment));
	pTable->Add("method &invert()", new EMethodPointerHolder((MachineMethod0)&MBigInt::invert));
	pTable->Add("method isPrime(&Bool)", new EMethodPointerHolder((MachineMethod1)&MBigInt::isPrime));
	pTable->Add("method isZero(&Bool)", new EMethodPointerHolder((MachineMethod1)&MBigInt::isZero));
	pTable->Add("method millerRabin(&Bool, BigInt)", new EMethodPointerHolder((MachineMethod2)&MBigInt::millerRabin));
	pTable->Add("method &multiply(Integer)", new EMethodPointerHolder((MachineMethod1)&MBigInt::multiply));
	pTable->Add("method &multiply(BigInt, BigInt)", new EMethodPointerHolder((MachineMethod2)&MBigInt::multiply2));
	pTable->Add("method &or(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::Or));
	pTable->Add("method &powerMod(BigInt, BigInt, BigInt)", new EMethodPointerHolder((MachineMethod3)&MBigInt::powerMod));
	pTable->Add("method &setBit(Integer, Bool)", new EMethodPointerHolder((MachineMethod2)&MBigInt::setBit));
	pTable->Add("method &setSign(Bool)", new EMethodPointerHolder((MachineMethod1)&MBigInt::setSign));
	pTable->Add("method &setToZero()", new EMethodPointerHolder((MachineMethod0)&MBigInt::setToZero));
	pTable->Add("method &setUInt(Integer, Integer)", new EMethodPointerHolder((MachineMethod2)&MBigInt::setUInt));
	pTable->Add("method &shiftLeft(Integer)", new EMethodPointerHolder((MachineMethod1)&MBigInt::shiftLeft));
	pTable->Add("method &shiftRight(Integer)", new EMethodPointerHolder((MachineMethod1)&MBigInt::shiftRight));
	pTable->Add("method &subtract(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::subtract));
	pTable->Add("method toHex(&String)", new EMethodPointerHolder((MachineMethod1)&MBigInt::toHex));
	pTable->Add("method &xor(BigInt)", new EMethodPointerHolder((MachineMethod1)&MBigInt::Xor));
	pTable->Add("method toStream(&Stream, &Stream)", new EMethodPointerHolder((MachineMethod2)&MBigInt::toStream));
	pTable->Add("method !fromStream(&Stream)", new EMethodPointerHolder((MachineMethod1)&MBigInt::fromStream));
	pTable->Add("method &setRefs(&Stream)", new EMethodPointerHolder((MachineMethod1)&MBigInt::setRefs));
}

