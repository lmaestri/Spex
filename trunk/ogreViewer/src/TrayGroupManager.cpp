#include "TrayGroupManager.h"
#include "OgreRender.h"
#include "SdkTrays.h"
#include "TrayGroup.h"

TrayGroupManager::TrayGroupManager( OgreBites::SdkTrayManager *pTrayMgr )
    : m_pTrayMgr( pTrayMgr )
{

}
TrayGroupManager::~TrayGroupManager()
{
    TrayLocationTrayGroupsMap::iterator iter = m_trayLocationTrayGroups.begin();
    while( iter != m_trayLocationTrayGroups.end() )
    {
        m_pTrayMgr->clearTray( iter->first );
        TrayGroupVector::iterator trayIter = iter->second.begin();
        while( trayIter != iter->second.end() )
        {
            // dunno should we delete all TrayGroups??
            if( (*trayIter) != 0 )
            {
                delete (*trayIter);
                (*trayIter) = 0;
            }

            ++ trayIter;
        }
        ++ iter;
    }
}


void TrayGroupManager::addTrayGroup( const OgreBites::TrayLocation& location, TrayGroup * pTray )
{
     TrayLocationTrayGroupsMap::iterator iter = m_trayLocationTrayGroups.find( location );
     if( iter == m_trayLocationTrayGroups.end() )
     {
        std::pair<TrayLocationTrayGroupsMap::iterator, bool> insertResult = m_trayLocationTrayGroups.insert( TrayLocationTrayGroupsPair( location, TrayGroupVector() ) ); 
        iter = insertResult.first;
     }

     iter->second.push_back( pTray );
}


void TrayGroupManager::update( )
{
    TrayLocationTrayGroupsMap::iterator iter = m_trayLocationTrayGroups.begin();
    while( iter != m_trayLocationTrayGroups.end() )
    {
        m_pTrayMgr->clearTray( iter->first );
        TrayGroupVector::iterator trayIter = iter->second.begin();
        while( trayIter != iter->second.end() )
        {
            if( (*trayIter) != 0 )
            {
                (*trayIter)->updateTray();
            }

            ++ trayIter;
        }
        ++ iter;
    }
}


TrayGroup *TrayGroupManager::createTrayGroup ( const OgreBites::TrayLocation trayLocation, const std::string& name, bool bShowLabel )
{
    TrayGroup* pTrayGroup = new TrayGroup( m_pTrayMgr, trayLocation, name, bShowLabel );
    addTrayGroup( trayLocation, pTrayGroup );
    return pTrayGroup;
}