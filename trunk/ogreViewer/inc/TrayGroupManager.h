#ifndef TrayGroupManager_h__
#define TrayGroupManager_h__
#include <map>
#include <vector>

//predeclarations
namespace OgreBites
{
    class SdkTrayManager;
    enum TrayLocation;
}
class TrayGroup;

class TrayGroupManager
{
private:
    typedef std::vector<TrayGroup *> TrayGroupVector;
    typedef std::map<OgreBites::TrayLocation, TrayGroupVector > TrayLocationTrayGroupsMap;
    typedef std::pair<OgreBites::TrayLocation, TrayGroupVector > TrayLocationTrayGroupsPair;
    TrayLocationTrayGroupsMap m_trayLocationTrayGroups;
    OgreBites::SdkTrayManager *m_pTrayMgr;

    void addTrayGroup( const OgreBites::TrayLocation& location, TrayGroup * pTray );

public:
    TrayGroupManager( OgreBites::SdkTrayManager *pTrayMgr );
    ~TrayGroupManager();
    TrayGroup * createTrayGroup( const OgreBites::TrayLocation trayLocation, const std::string& name, bool bShowLabelAllways = false );
    void update( );
};

#endif // TrayGroupManager_h__
