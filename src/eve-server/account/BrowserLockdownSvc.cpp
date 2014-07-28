/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Captnoord
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/BrowserLockdownSvc.h"

// crap
PyCallable_Make_InnerDispatcher(BrowserLockdownService)

BrowserLockdownService::BrowserLockdownService( PyServiceMgr *mgr )
: PyService(mgr, "browserLockdownSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesHash);
    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesList);
    PyCallable_REG_CALL(BrowserLockdownService, GetDefaultHomePage);
    PyCallable_REG_CALL(BrowserLockdownService, IsBrowserInLockdown);
}

BrowserLockdownService::~BrowserLockdownService() {
    delete m_dispatch;
}

PyObject* GenerateLockdownCachedObject()
{
/*
PyString:"util.CachedObject"
    PyTuple:3
    itr[0]:PyTuple:3
      itr[0]:"Method Call"
      itr[1]:"server"
      itr[2]:PyTuple:2
        itr[0]:"browserLockdownSvc"
        itr[1]:"GetFlaggedSitesHash"
    itr[1]:0xACECD
    itr[2]:PyTuple:2
      itr[0]:0x1CC26986D4B75A0
      itr[1]:0xC017
        */
    PyTuple * arg_tuple = new PyTuple(3);

    arg_tuple->SetItem(0, new_tuple("Method Call", "server", new_tuple("browserLockdownSvc", "GetFlaggedSitesHash")));
    arg_tuple->SetItem(1, new PyInt(0xACECD));
    arg_tuple->SetItem(2, new_tuple(new PyLong(0x1CC26986D4B75A0LL), new PyInt(0xC017) ) );

    return new PyObject( "util.CachedObject" , arg_tuple );
}

/**
 * CacheOK python class. This class simulates the python class CacheOK which is derived from the StandardError class.
 */
class CacheOK : public PyObjectEx_Type1
{
public:
    CacheOK() : PyObjectEx_Type1( new PyToken("objectCaching.CacheOK"), new_tuple("CacheOK") ) {}
};

//02:40:18 L BrowserLockdownService::Handle_GetFlaggedSitesHash(): size= 0
PyResult BrowserLockdownService::Handle_GetFlaggedSitesHash(PyCallArgs &call)
{
    /* if cache hash is correct send CacheOK */

    /**
      PyTuple:4
        itr[0]:PyInt(0x1)
        itr[1]:"GetFlaggedSitesHash"
        itr[2]:PyTuple:0
        itr[3]:PyDict:1
          dict["machoVersion"]=PyList:2
            itr:PyLong(0x1CC26986D4B75A0) // time
            itr:PyInt(0xC017) // hash?
     */
    return new CacheOK();
}

//02:40:18 L BrowserLockdownService::Handle_GetFlaggedSitesList(): size= 0
PyResult BrowserLockdownService::Handle_GetFlaggedSitesList(PyCallArgs &call)
{
    //PyDict* args = new PyDict;

    //return new PyObject( "objectCaching.CachedMethodCallResult", args );
//}

        /*PyClass
          PyString:"objectCaching.CachedMethodCallResult"
          PyTuple:3
            itr[0]:PyDict:1
              dict["versionCheck"]=PyTuple:3
                itr[0]:"run"
                itr[1]:"run"
                itr[2]:"run"
            itr[1]:PyClass
              PyString:"util.CachedObject"
              PyTuple:3
                itr[0]:PyTuple:3
                  itr[0]:"Method Call"
                  itr[1]:"server"
                  itr[2]:PyTuple:2
                    itr[0]:"browserLockdownSvc"
                    itr[1]:"GetFlaggedSitesHash"
                itr[1]:0xACECD
                itr[2]:PyTuple:2
                  itr[0]:0x1CC26986D4B75A0
                  itr[1]:0xC017
            itr[2]:None
            */
        /**
16:53:03 [SvcCall] Service browserLockdownSvc: calling GetFlaggedSitesList
16:53:03 [SvcCallTrace]   Call Arguments:
16:53:03 [SvcCallTrace]       Tuple: Empty
16:53:03 [SvcCallTrace]   Call Named Arguments:
16:53:03 [SvcCallTrace]     Argument 'machoVersion':
16:53:03 [SvcCallTrace]         Integer field: 1
16:53:03 [SvcCallTrace] Call GetFlaggedSitesList returned:
16:53:03 [SvcCallTrace]       Object:
16:53:03 [SvcCallTrace]         Type: String: 'objectCaching.CachedMethodCallResult'
16:53:03 [SvcCallTrace]         Args: Tuple: 3 elements
16:53:03 [SvcCallTrace]         Args:   [ 0] Dictionary: 1 entries
16:53:03 [SvcCallTrace]         Args:   [ 0]   [ 0] Key: String: 'versionCheck'
16:53:03 [SvcCallTrace]         Args:   [ 0]   [ 0] Value: Tuple: 3 elements
16:53:03 [SvcCallTrace]         Args:   [ 0]   [ 0] Value:   [ 0] String: 'run'
16:53:03 [SvcCallTrace]         Args:   [ 0]   [ 0] Value:   [ 1] String: 'run'
16:53:03 [SvcCallTrace]         Args:   [ 0]   [ 0] Value:   [ 2] String: 'run'
16:53:03 [SvcCallTrace]         Args:   [ 1] Object:
16:53:03 [SvcCallTrace]         Args:   [ 1]   Type: String: 'util.CachedObject'
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args: Tuple: 3 elements
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0] Tuple: 3 elements
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0]   [ 0] String: 'Method Call'
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0]   [ 1] String: 'server'
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0]   [ 2] Tuple: 2 elements
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0]   [ 2]   [ 0] String: 'browserLockdownSvc'
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 0]   [ 2]   [ 1] String: 'GetFlaggedSitesHash'
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 1] Integer field: 708301
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 2] Tuple: 2 elements
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 2]   [ 0] Integer field: 129520925397448096
16:53:03 [SvcCallTrace]         Args:   [ 1]   Args:   [ 2]   [ 1] Integer field: 49175
16:53:03 [SvcCallTrace]         Args:   [ 2] (None)

*/

    PyTuple* arg_tuple = new PyTuple(3);

        PyDict* itr_1 = new PyDict();
        itr_1->SetItem("versionCheck", new_tuple("run", "run", "run"));

    arg_tuple->SetItem(0, itr_1);
    arg_tuple->SetItem(1, GenerateLockdownCachedObject());
    arg_tuple->SetItem(2, new PyNone());

    return new PyObject( "objectCaching.CachedMethodCallResult", arg_tuple );
}

PyResult BrowserLockdownService::Handle_GetDefaultHomePage(PyCallArgs &call) {
  sLog.Log( "BrowserLockdownService::Handle_GetDefaultHomePage()", "size= %u", call.tuple->size() );
    return NULL;
}

//00:37:03 L BrowserLockdownService::Handle_IsBrowserInLockdown(): size= 0
PyResult BrowserLockdownService::Handle_IsBrowserInLockdown(PyCallArgs &call) {
  // make config var to enable/diasble?
    return new PyNone;
}
