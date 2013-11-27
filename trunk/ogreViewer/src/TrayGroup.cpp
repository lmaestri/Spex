#include "TrayGroup.h"
#include "OgreRender.h"
#include "SdkTrays.h"


TrayGroup::TrayGroup( OgreBites::SdkTrayManager* pMgr, const OgreBites::TrayLocation location,const std::string& name, bool bShowLabel )
    : m_pTrayMgr( pMgr ), m_trayLocation( location ), m_bShowLabelAlways( bShowLabel ), m_name( name ), m_bIsUnfold( false ), m_trayWidth( 200 ), m_bIsVisible( true )
{
    m_pLabel = m_pTrayMgr->createLabel( m_trayLocation, m_name.append("Label"), name, m_trayWidth );
    updateTray();
}

TrayGroup::~TrayGroup( void )
{

}

void TrayGroup::addWidget( OgreBites::Widget * pWidget )
{
    m_widgets.push_back( pWidget );
    updateTray();
}

void TrayGroup::updateTray( void )
{
    // first we remove all widgets
    removeAllWidgets( );

    if( m_bIsVisible )
    {
        // tray closed
        if( !m_bIsUnfold ) 
        {   
            //m_pTrayMgr->removeWidgetFromTray( m_pLabel );

            {
                m_pTrayMgr->moveWidgetToTray( m_pLabel, m_trayLocation );
                m_pLabel->show();
            }

        }
        // tray is open
        else
        {
            m_pTrayMgr->moveWidgetToTray( m_pLabel, m_trayLocation );
            m_pLabel->show();

            std::vector<OgreBites::Widget *>::iterator iter = m_widgets.begin();
            while( iter != m_widgets.end() )
            {
                m_pTrayMgr->moveWidgetToTray( (*iter), m_trayLocation );
                (*iter)->show();
                ++iter;
            }

            if( !m_bShowLabelAlways )
            {
                m_pTrayMgr->removeWidgetFromTray( m_pLabel );
                m_pLabel->hide();
            }
        }
    }
}

bool TrayGroup::toggleTrayGroup( )
{
    m_bIsUnfold = !m_bIsUnfold;

    //updateTray( );

    return m_bIsUnfold;
}

OgreBites::Label* TrayGroup::getLabel( void )
{
    return m_pLabel;
}

void TrayGroup::setUnfold( bool isUnfold )
{
    if( m_bIsUnfold != isUnfold )
    {
      m_bIsUnfold = isUnfold;
      //updateTray();
    }
}

bool TrayGroup::isUnfold()
{
    return m_bIsUnfold;
}

void TrayGroup::setVisible( bool isVisible )
{
    m_bIsVisible = isVisible;
}

void TrayGroup::removeAllWidgets( )
{
    // remove all widgets, except label
    std::vector<OgreBites::Widget *>::iterator iter = m_widgets.begin();
    while( iter != m_widgets.end() )
    {
        m_pTrayMgr->removeWidgetFromTray( (*iter) );
        (*iter)->hide();
        ++iter;
    }
    
    // remove label
    m_pTrayMgr->removeWidgetFromTray( m_pLabel );
    m_pLabel->hide();
}
