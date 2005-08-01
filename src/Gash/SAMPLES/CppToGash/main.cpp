#include <stdio.h>
#include "../../Source/Include/Gash.h"
#include "../../Source/GnuSDK/GnuSDK.h"
#include "ShowString.h"

void DoNothing(CallBackData* pCallBackData)
{

}





// The VM will call this function when it needs to link to a C++ function.
// It will pass in the name of the function it wants to link to and you
// should return a pointer to the C++ function that it is looking for.
CallBackFunc GetCallBack(GXMLTag* pCallBackTag)
{
	return DoNothing;
}





void main()
{
	GVM vm(g_xlib_string_ShowString, strlen(g_xlib_string_ShowString) + 1, true, GetCallBack, NULL, NULL);

	GObjectParam myString;
	int nStringClassID = vm.GetLibrary()->FindClassID("String");
	int nStringNewMethodID = vm.GetLibrary()->FindMethodID(nStringClassID, "New");
	vm.CallOneParam(nStringNewMethodID, &myString);
}
