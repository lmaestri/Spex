#ifndef TrayGroup_h__
#define TrayGroup_h__

#include <vector>
#include <string>
//#include "SdkTrays.h"


namespace OgreBites
{
    class Widget;
    class SdkTrayManager;
    class Label;
    enum TrayLocation;
}

class TrayGroup 
{
    friend class TrayGroupManager;

private:
    std::vector<OgreBites::Widget *> m_widgets;
    bool    m_bShowLabelAlways;
    OgreBites::Label    *m_pLabel;
    bool    m_bIsUnfold;
    bool    m_bIsVisible;
    OgreBites::TrayLocation m_trayLocation;
    OgreBites::SdkTrayManager *m_pTrayMgr;
    std::string m_name;
    int     m_trayWidth;

    void updateTray( void );
    void removeAllWidgets( );

public:
    TrayGroup( OgreBites::SdkTrayManager * pTrayMgr, const OgreBites::TrayLocation trayLocation, const std::string& name, bool bShowLabel = false );
    virtual ~TrayGroup( void );
    void addWidget( OgreBites::Widget * pWidget );
    bool toggleTrayGroup( );
    OgreBites::Label* getLabel( void );
    void setUnfold( bool isUnfold );
    bool isUnfold();
    void setVisible( bool isVisible );
};

#endif // TrayGroup_h__
