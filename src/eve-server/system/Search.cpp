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
    Author:        Allan
*/


#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/Search.h"

PyCallable_Make_InnerDispatcher(Search)

Search::Search(PyServiceMgr *mgr)
: PyService(mgr, "search"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(Search, Query);
    PyCallable_REG_CALL(Search, QuickQuery);
}

Search::~Search() {
    delete m_dispatch;
}

PyResult Search::Handle_Query( PyCallArgs& call )   //called from people/places window and some other things...
{
    std::string str = call.tuple->GetItem( 0 )->AsWString()->content();

	Replace(str);

    //  if search type 'all' is selected, use this query to search everything....
    if ( call.tuple->GetItem( 1 )->AsList()->size() > 1 )
        return(m_db.QueryAll( str, call.client->GetCharacterID() ));

    //  else specific search type is selected....passed as single int arg.
    Call_SearchQuery args;      // this decodes a single int arg passed in a PyList
    if(!args.Decode(&call.tuple)) {
        _log(CLIENT__ERROR, "Failed to decode args.");
        call.client->SendErrorMsg("Search Failed.  Try using a different search string.");
        return NULL;
    }

    return(m_db.Query( str, args.int1, call.client->GetCharacterID() ));
}

//  hideNPC is true for bounty search
PyResult Search::Handle_QuickQuery( PyCallArgs& call )  //called from starmap  and contracts......and the fuckin mail search...
{/*         QuickQuery(query, groupIDList, hideNPC=hideNPC, onlyAltName=onlyAltName)
05:56:52 L search::Handle_QuickQuery(): size=2, 0=WString(rens*), 1=List()
05:56:52 [SvcCall]   Call Arguments:
05:56:52 [SvcCall]       Tuple: 2 elements
05:56:52 [SvcCall]         [ 0] WString: 'rens*'            // search string
05:56:52 [SvcCall]         [ 1] List: 3 elements            // invGroup   groupID 6=sun, 7=planet, 8=moon
05:56:52 [SvcCall]         [ 1]   [ 0] Integer field: 6
05:56:52 [SvcCall]         [ 1]   [ 1] Integer field: 7
05:56:52 [SvcCall]         [ 1]   [ 2] Integer field: 8
05:56:52 [SvcCall]   Call Named Arguments:
05:56:52 [SvcCall]     Argument 'hideNPC':
05:56:52 [SvcCall]         Integer field: 0
05:56:52 [SvcCall]     Argument 'machoVersion':
05:56:52 [SvcCall]         Integer field: 1
05:56:52 [SvcCall]     Argument 'onlyAltName':
05:56:52 [SvcCall]         Integer field: 0

00:41:50 L Search: Handle_QuickQuery
00:41:50 [SvcCall]   Call Arguments:
00:41:50 [SvcCall]       Tuple: 2 elements
00:41:50 [SvcCall]         [ 0] WString: 'lee domani'
00:41:50 [SvcCall]         [ 1] List: 1 elements
00:41:50 [SvcCall]         [ 1]   [ 0] Integer field: 2             //  shit...this is a char item...called from the damn mail window....
*/
    sLog.Log( "Search", "Handle_QuickQuery" );
  call.Dump(SERVICE__CALLS);

    std::string str = call.tuple->GetItem( 0 )->AsWString()->content();
	Replace(str);

  /*  may not need this....using same python xml decode as above searchquery
    Call_SearchQuickQuery args;    //  args.ints given as map now when using Call_SearchQuickQuery.
*/
    //return(m_db.QuickQuery( args.string, args.int1, args.int2, args.int3, call.byname.find("hideNPC"), call.byname.find("onlyAltName"), call.client->GetCharacterID() ));

    if ( call.tuple->GetItem( 1 )->AsList()->size() > 1 )
        return(m_db.QuickQuery( str, call.client->GetCharacterID() ));

    Call_SearchQuery args;      // this decodes a single int arg passed in a PyList
    if(!args.Decode(&call.tuple)) {
        _log(CLIENT__ERROR, "Failed to decode args.");
        call.client->SendErrorMsg("Search Failed.  Try using a different search string.");
        return NULL;
    }

    return(m_db.Query( str, args.int1, call.client->GetCharacterID() ));
}

void Search::Replace(std::string &s) {
    int i;
    for (i = 0; i < s.length(); ++i) {
        switch (s[i]) {
        case '*':
            s[i] = '%';
        }
    }
}

/**
18:21:22 [SvcCall] Service search: calling QuickQuery
18:21:22 [SvcCallTrace]   Call Arguments:
18:21:22 [SvcCallTrace]       Tuple: 2 elements
18:21:22 [SvcCallTrace]         [ 0] WString: 'zyen* domani*'
18:21:22 [SvcCallTrace]         [ 1] List: 1 elements
18:21:22 [SvcCallTrace]         [ 1]   [ 0] Integer field: 2
18:21:22 [SvcCallTrace]   Call Named Arguments:
18:21:22 [SvcCallTrace]     Argument 'hideNPC':
18:21:22 [SvcCallTrace]         Integer field: 0
18:21:22 [SvcCallTrace]     Argument 'machoVersion':
18:21:22 [SvcCallTrace]         Integer field: 1
18:21:22 [SvcCallTrace]     Argument 'onlyAltName':
18:21:22 [SvcCallTrace]         Integer field: 0
18:21:22 [SvcCall]   Call Arguments:
18:21:22 [SvcCall]       Tuple: 2 elements
18:21:22 [SvcCall]         [ 0] WString: 'zyen* domani*'
18:21:22 [SvcCall]         [ 1] List: 1 elements
18:21:22 [SvcCall]         [ 1]   [ 0] Integer field: 2
18:21:22 [SvcCall]   Call Named Arguments:
18:21:22 [SvcCall]     Argument 'hideNPC':
18:21:22 [SvcCall]         Integer field: 0
18:21:22 [SvcCall]     Argument 'machoVersion':
18:21:22 [SvcCall]         Integer field: 1
18:21:22 [SvcCall]     Argument 'onlyAltName':
18:21:22 [SvcCall]         Integer field: 0
18:21:22 [SvcCallTrace] Call QuickQuery returned:
18:21:22 [SvcCallTrace]       (None)
*/
