#ifndef trayicon_h
#define trayicon_h


// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.

template <class T>
class CTrayIconImpl
{
public:	
    CTrayIconImpl()
        : m_bInstalled(false), menuDefaultItem(-1), currentIcon(0), menuId(-1)
    {
        WM_TRAYICON = ::RegisterWindowMessage(_T("WM_TRAYICON"));
    }
    
    ~CTrayIconImpl()
    {
        hideIcon();
        
        IconList::const_iterator it = icons.begin();
        while(it != icons.end())
        {
            unloadIcon(it->first);
            it = icons.begin();
        }
    }

    // Load an icon from the resources and adds it to internal icon list
    // id is icon resource ID
    bool loadIcon(WORD id)
    {
        HICON icon = NULL;

        if ( ! getIcon(id, &icon))
        {
            icon = (HICON) ::LoadImage( WTL::ModuleHelper::GetResourceInstance()
                                      , MAKEINTRESOURCE(id)
                                      , IMAGE_ICON
                                      , ::GetSystemMetrics(SM_CXSMICON)
                                      , ::GetSystemMetrics(SM_CYSMICON)
                                      , LR_DEFAULTCOLOR);
            if (icon != NULL)
            {
                icons[id] = icon;
            }
        }

        return icon != NULL;
    }

    // Unloads previously loaded icon with given Id
    void unloadIcon(WORD id)
    {
        HICON icon = NULL;

        if (getIcon(id, &icon))
        {
            if (m_bInstalled && currentIcon == id)
            {
                hideIcon();
            }

            DestroyIcon(icon);
            icons.erase(id);
        }
    }


    // Shows previously loaded icon in the system tray
    // id is icon resource ID
    bool showIcon(WORD id, LPCTSTR lpszToolTip = NULL)
    {
        HICON icon;

        if ( ! getIcon(id, &icon))
        {
            return false;
        }

        T* pT = static_cast<T*>(this);

        // Fill in the data		
        m_notifyData.hWnd = pT->m_hWnd;
        m_notifyData.uID = 0;
        m_notifyData.hIcon = icon;
        m_notifyData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        m_notifyData.uCallbackMessage = WM_TRAYICON;
        if (lpszToolTip != NULL)
        {
            _tcscpy(m_notifyData.szTip, lpszToolTip);
        }

        currentIcon = id;

        // Install
        bool result = Shell_NotifyIcon(m_bInstalled ? NIM_MODIFY : NIM_ADD, &m_notifyData) ? true : false;
        if (result)
        {
            m_bInstalled = true;
        }
        return result;
    }

    // Remove taskbar icon
    // returns true on success
    bool hideIcon()
    {
        if (!m_bInstalled)
        {
            return false;
        }

        m_bInstalled = false;
        currentIcon = 0;

        m_notifyData.uFlags = 0;
        return Shell_NotifyIcon(NIM_DELETE, &m_notifyData) ? true : false;
    }

    // Set the icon tooltip text
    // returns true on success
    bool setTooltipText(LPCTSTR pszTooltipText)
    {
        if (pszTooltipText == NULL)
        {
            return false;
        }

        // Fill the structure
        m_notifyData.uFlags = NIF_TIP;
        _tcscpy(m_notifyData.szTip, pszTooltipText);

        return Shell_NotifyIcon(NIM_MODIFY, &m_notifyData) ? true : false;
    }

    bool setBalloonText(LPCTSTR balloonText, LPCTSTR titletext = NULL, DWORD icon = NIIF_NONE, UINT timeout = 8000)
    {
        if (balloonText == NULL)
        {
            return false;
        }

        m_notifyData.uFlags = NIF_INFO;
        m_notifyData.uTimeout = timeout;
        m_notifyData.dwInfoFlags = icon;

        _tcscpy(m_notifyData.szInfo, balloonText);
        if (titletext != NULL)
        {
            _tcscpy(m_notifyData.szInfoTitle, titletext);
        }
        else
        {
            m_notifyData.szInfoTitle[0] = _T('\0');
        }

        return Shell_NotifyIcon(NIM_MODIFY, &m_notifyData) ? true : false;
    }

    // Set the menu and default menu item ID for icon popup menu
    void setTrayMenu(int menuId, int defaultItem)
    {
        this->menuId = menuId;
        menuDefaultItem = defaultItem;
    }

    // Allow the menu items to be enabled/checked/etc.
    virtual void PrepareMenu(WTL::CMenuHandle & /*hMenu*/)
    {
        // Stub
    }


private:

    BEGIN_MSG_MAP(CTrayIcon)
        MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
    END_MSG_MAP()

    LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        // Is this the ID we want?
        if (wParam != m_notifyData.uID)
        {
            return 0;
        }

        T* pT = static_cast<T*>(this);

        if (LOWORD(lParam) == WM_RBUTTONUP)
        {
            SetForegroundWindow(pT->m_hWnd);            // Make app the foreground

            // Load the menu
            WTL::CMenu oMenu;
            if (menuId == -1
                || ! oMenu.LoadMenu(menuId))
            {
                // no menu - default action to show window and remove icon
                hideIcon();
                ShowWindow(pT->m_hWnd, SW_SHOW);
                return 0;
            }

            WTL::CMenuHandle oPopup(oMenu.GetSubMenu(0));    // Get the sub-menu
            pT->PrepareMenu(oPopup);

            // Set the default menu item
            if (menuDefaultItem == -1)
            {
                oPopup.SetMenuDefaultItem(0, TRUE);
            }
            else
            {
                oPopup.SetMenuDefaultItem(menuDefaultItem);
            }
            
            CPoint pos; 
            GetCursorPos(&pos);                         // Get the menu position
            oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);

            // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
            pT->PostMessage(WM_NULL);

            oMenu.DestroyMenu();
        }
        else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
        {
            SetForegroundWindow(pT->m_hWnd);            // Make app the foreground

            // Load the menu
            WTL::CMenu oMenu;
            if (menuId == -1
                || ! oMenu.LoadMenu(menuId))
            {
                return 0;
            }
            
            WTL::CMenuHandle oPopup(oMenu.GetSubMenu(0));	// Get the sub-menu
            // Get the item
            if (menuDefaultItem != -1)
            {
                // Send
                pT->SendMessage(WM_COMMAND, menuDefaultItem, 0);
            }
            else
            {
                UINT nItem = oPopup.GetMenuItemID(0);
                pT->SendMessage(WM_COMMAND, nItem, 0);
            }

            oMenu.DestroyMenu();
        }

        return 0;
    }

    // Wrapper class for the Win32 NOTIFYICONDATA structure
    class CNotifyIconData : public NOTIFYICONDATA
    {
    public:	
        CNotifyIconData()
        {
            memset(this, 0, sizeof(NOTIFYICONDATA));
            cbSize = sizeof(NOTIFYICONDATA);
        }
    };

    bool getIcon(WORD id, HICON *icon)
    {
        IconList::const_iterator it = icons.find(id);
        if (it != icons.end())
        {
            *icon = it->second;
            return true;
        }
        return false;
    }


    typedef std::map<WORD, HICON>   IconList;

    IconList        icons;
    WORD            currentIcon;

    UINT            WM_TRAYICON;
    CNotifyIconData m_notifyData;
    bool            m_bInstalled;
    int             menuId;
    int             menuDefaultItem;

};

#endif