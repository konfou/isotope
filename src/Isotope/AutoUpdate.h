#ifndef __AUTOUPDATE_H__
#define __AUTOUPDATE_H__

// Defines the build number for this app
#define BUILD_NUMBER 20050706

#ifdef WIN32
#define UPDATE_DESCRIPTOR "http://localhost/update.xml"
//#define UPDATE_DESCRIPTOR "http://neutron.edumetrics.org/updates/windows/update.xml"
#else // WIN32
#define UPDATE_DESCRIPTOR "http://neutron.edumetrics.org/updates/linux/update.xml"
#endif // !WIN32


// Returns a string containing an XML-serialization of the trusted public key
const char* GetTrustedPublicKey();

// If a newer version was previously downloaded, swaps it out with this app,
// otherwise checks for updates, and if a newer version exists, downloads it
bool DoAutoUpdate();

// Produce an update.xml file containing the hash of this application and
// signed with the trusted private key
void BlessThisApplication(const char* szPrivateKeyFilename, const char* szUrl);

#endif // __AUTOUPDATE_H__