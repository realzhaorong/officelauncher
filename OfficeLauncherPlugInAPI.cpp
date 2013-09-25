/*
 * Copyright (C) 2012-2013 Alfresco Software Limited.
 *
 * This file is part of Alfresco
 *
 * Alfresco is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alfresco is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Alfresco. If not, see <http://www.gnu.org/licenses/>.
 */

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"
#include "global/config.h"
#include "APITypes.h"
#include "BrowserHost.h"
#include "DOM/Window.h"
#ifdef FB_WIN
#include "AXDOM/Window.h"
#endif

#include "OfficeLauncherPlugInAPI.h"
#include "OfficeLauncherPlugInErrorCodes.h"

#ifdef FB_WIN
#include "windows.h"
#endif

///////////////////////////////////////////////////////////////////////////////
/// @fn long OfficeLauncherPlugInAPI::viewDocument(const std::string& url_utf8)
///
/// @brief  Launches the associated application and opens the document with
///         the given URL Read-Only.
///         Delegates control to the platform dependant code.
///////////////////////////////////////////////////////////////////////////////
long OfficeLauncherPlugInAPI::viewDocument(const std::string& url_utf8)
{
    if(url_utf8.size() > MAX_URL_LENGTH)
    {
        return OLP_ERROR_URL_TOO_LONG;
    }
    if(!( m_platformOfficeLauncher.suppressOpenWarning(url_utf8) || confirmOpen(url_utf8)))
    {
        return OLP_ERROR_USER_REJECTED;
    }
    return m_platformOfficeLauncher.viewDocument(url_utf8);
}

///////////////////////////////////////////////////////////////////////////////
/// @fn long OfficeLauncherPlugInAPI::editDocument(const std::string& url_utf8)
///
/// @brief  Launches the associated application and opens the document with
///         the given URL for editing.
///         Delegates control to the platform dependant code.
///////////////////////////////////////////////////////////////////////////////
long OfficeLauncherPlugInAPI::editDocument(const std::string& url_utf8)
{
    if(url_utf8.size() > MAX_URL_LENGTH)
    {
        return OLP_ERROR_URL_TOO_LONG;
    }
    if(!( m_platformOfficeLauncher.suppressOpenWarning(url_utf8) || confirmOpen(url_utf8)))
    {
        return OLP_ERROR_USER_REJECTED;
    }
    return m_platformOfficeLauncher.editDocument(url_utf8);
}

bool confirmOpenActiveX(const std::string& url_utf8)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @fn bool OfficeLauncherPlugInAPI::confirmOpen(const std::string& url_utf8)
///
/// @brief  Asks the user for  permission to open the given URL.
///         Invokes the JavaScript confirm method for user interaction.
///////////////////////////////////////////////////////////////////////////////
bool OfficeLauncherPlugInAPI::confirmOpen(const std::string& url_utf8)
{
    FB::DOM::WindowPtr domWindow = m_host->getDOMWindow();
    std::wstring decodedURL = domWindow->callMethod<std::wstring>("decodeURIComponent", FB::variant_list_of(url_utf8));
    std::wstring msg = L"Do you want to open this file?\n\n";
    msg.append(decodedURL);
    msg.append(L"\n\nSome files can harm your computer. If you do not fully trust the source, do not open the file.");
    const std::string msg_utf8 = FB::wstring_to_utf8(msg);
#ifdef FB_WIN
    if(typeid(*domWindow) == typeid(FB::ActiveX::AXDOM::Window))
    {
        // invoking the confirm method does not work with ActiveX for some reason.
        // So we have to use a normal Windows MessageBox for that.
        int result = MessageBox(NULL, msg.c_str(), L"Confirm", MB_OKCANCEL | MB_ICONINFORMATION);
        return (result == IDOK);
    }
    else
    {
#endif
        return domWindow->callMethod<bool>("confirm", FB::variant_list_of(msg_utf8));
#ifdef FB_WIN
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// @fn OfficeLauncherPlugInPtr OfficeLauncherPlugInAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
OfficeLauncherPlugInPtr OfficeLauncherPlugInAPI::getPlugin()
{
    OfficeLauncherPlugInPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read-only property version
std::string OfficeLauncherPlugInAPI::get_version()
{
    return FBSTRING_PLUGIN_VERSION;
}