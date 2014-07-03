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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "PyBoundObject.h"
#include "system/BookmarkService.h"

// Set the maximum number for any user-created bookmark and folder.
const uint32 BookmarkService::MAX_BOOKMARK_ID = 0xFFFFFFFF;
const uint32 BookmarkService::MAX_BM_FOLDER_ID = 0xFFFFFFFF;

PyCallable_Make_InnerDispatcher(BookmarkService)

BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BookmarkService, GetBookmarks)
    PyCallable_REG_CALL(BookmarkService, BookmarkLocation)
    PyCallable_REG_CALL(BookmarkService, UpdateBookmark)
    PyCallable_REG_CALL(BookmarkService, CreateFolder)
    PyCallable_REG_CALL(BookmarkService, UpdateFolder)
    PyCallable_REG_CALL(BookmarkService, DeleteFolder)

    /*  NOT WORKING YET  */
    PyCallable_REG_CALL(BookmarkService, DeleteBookmarks)
    PyCallable_REG_CALL(BookmarkService, MoveBookmarksToFolder)
}

BookmarkService::~BookmarkService() {
    delete m_dispatch;
}

uint32 BookmarkService::GetNextAvailableBookmarkID()
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID "
        " FROM bookmarks "
        " WHERE bookmarkID > %u ", 0))
    {
        sLog.Error( "BookmarkDB::GetNextAvailableBookmarkID()", "Error in query: %s", res.error.c_str() );
        return 0;
    }

    uint32 currentBookmarkID = 1;

    // Traverse through the rows in the query result until the first gap is found
    // and return the value that would be first (or only one) in the gap as the next
    // free bookmark ID:
    DBResultRow row;
    while( res.GetRow(row) )
    {
        const uint32 bookmarkID = row.GetUInt( 0 );

        if( currentBookmarkID < bookmarkID )
            return currentBookmarkID;

        ++currentBookmarkID;
    }

        // Check to make sure that the next available bookmarkID is not equal to the Maximum bookmarkID value
    if( currentBookmarkID <= BookmarkService::MAX_BOOKMARK_ID )
        return currentBookmarkID;
    else
        return 0;    // No free bookmarkIDs found (this should never happen as there are way too many IDs to exhaust)
}

uint32 BookmarkService::GetNextAvailableFolderID()
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  folderID "
        " FROM bookmarkFolders "
        " WHERE folderID > %u ", 0))
    {
        sLog.Error( "BookmarkDB::GetNextAvailableFolderID()", "Error in query: %s", res.error.c_str() );
        return 0;
    }

    uint32 currentFolderID = 1;

    DBResultRow row;
    while( res.GetRow(row) )
    {
        const uint32 folderID = row.GetUInt( 0 );

        if( currentFolderID < folderID )
            return currentFolderID;

        ++currentFolderID;
    }

    if( currentFolderID <= BookmarkService::MAX_BM_FOLDER_ID )
        return currentFolderID;
    else
        return 0;
}

bool BookmarkService::LookupBookmark(uint32 characterID, uint32 bookmarkID, uint32 &itemID, uint32 &typeID, double &x, double &y, double &z)
{
    // Retrieve bookmark information for external use:
    uint32 ownerID;
    uint32 flag;
    std::string memo;
    uint64 created;
    uint32 locationID;
    uint32 creatorID;
    uint32 folderID;
    std::string note;

    return m_db.GetBookmarkInformation(bookmarkID,ownerID,itemID,typeID,flag,memo,created,x,y,z,locationID,note,creatorID,folderID);
}


PyResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
    PyTuple* result = new PyTuple(2);

    result->items[0] = m_db.GetBookmarks(call.client->GetCharacterID());
    result->items[1] = m_db.GetFolders(call.client->GetCharacterID());

    return result;
}


PyResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call)
{
    std::string note;
    std::string memo;
    uint32 bookmarkID = GetNextAvailableBookmarkID();
    uint32 ownerID = 0;
    uint32 itemID = 0;
    uint32 typeID = 0;
    uint32 flag = 0;
    uint64 created = 0;
    uint32 locationID = 0;
    uint32 typeCheck = 0;
    uint32 creatorID = call.client->GetCharacterID();       //  will need more research when trade is implemented for sharing bm's between chars....corp/friend/etc.  this should be the original characterID that made the bm.
    uint32 folderID = 0;
    GPoint point;

    ////////////////////////////////////////
    // Verify expected packet structure:            updated 20Jan14   -allan
    //
    // call.tuple  size=4
    //       |
    //       |--> [0] PyInt:      sends id of entity calling char is in...ship/station/system/pos/etc
    //       |--> [1] PyInt:       ownerID = charID of char making the bm
    //       |--> [2] PyWString:  label (called memo in db) for the bookmark
    //       \--> [3] PyString:  text for the "note" field in the bookmark
    //
    ////////////////////////////////////////

    typeCheck = call.tuple->GetItem( 0 )->AsInt()->value();  //current shipID/stationID/POS_ID/systemID
    typeID = m_db.FindBookmarkTypeID(typeCheck);    // get bm typeID

    if ( typeCheck >= 140000000 ) {      // entity #'s above 140m are player-owned
        point = call.client->GetPosition();       // Get x,y,z location.  bm type is coordinate as "spot in xxx system"
        locationID = call.client->GetLocationID();       // locationID of bm is current sol system
        itemID = locationID;      //  locationID = itemID for coord bm.  shows jumps, s/c/r in bm window, green in system
    }else if ( typeID == 2502 ){  // not player-owned, check for station.
        point.x, point.y, point.z = 0, 0, 0;      // no x,y,z location.  bm type is station
        itemID =  call.tuple->GetItem( 0 )->AsInt()->value();  // this is stationID
        locationID = call.client->GetSystemID();       // get sol system of current station
    }else{      // char is passing systemID from map.  char is marking a solar systemID for bm
        point.x, point.y, point.z = 0, 0, 0;      // no x,y,z location.  bm type is solar system
        locationID = call.tuple->GetItem( 0 )->AsInt()->value();  // this is systemID from map
        itemID = locationID;      //  locationID = itemID for coord bm.  shows jumps, s/c/r in bm window, green in system
    }
    ownerID = call.tuple->GetItem( 1 )->AsInt()->value();

    if ( call.tuple->GetItem( 2 )->IsString() )
        memo = call.tuple->GetItem( 2 )->AsString()->content();
    else if ( call.tuple->GetItem( 2 )->IsWString() )
        memo = call.tuple->GetItem( 2 )->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(2) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(2)->TypeString() );
        return NULL;
    }

    if ( call.tuple->GetItem( 3 )->IsString() )
        note = call.tuple->GetItem( 3 )->AsString()->content();
    else if ( call.tuple->GetItem( 3 )->IsWString() )
        note = call.tuple->GetItem( 3 )->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(3) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(3)->TypeString() );
        return NULL;
    }

      sLog.Debug( "BookmarkService::Handle_BookmarkLocation()", "itemID = %u, typeID = %u", itemID, typeID );
    ////////////////////////////////////////
    // Save bookmark info to database:
    ////////////////////////////////////////
    flag = 0;                                           // Don't know what to do with this value
    created = Win32TimeNow();
    m_db.SaveNewBookmarkToDatabase
    (
        bookmarkID,
        ownerID,
        itemID,
        typeID,
        flag,
        memo,
        created,
        point.x,
        point.y,
        point.z,
        locationID,
        note,
        creatorID,
        folderID

    );

    ////////////////////////////////////////
    // Build return packet:
    //
    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    ////////////////////////////////////////

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyInt( itemID );               // itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invTypes
    tuple0->items[ 3 ] = new PyInt( (uint32)(point.x) );    // X coordinate
    tuple0->items[ 4 ] = new PyInt( (uint32)(point.y) );    // Y coordinate
    tuple0->items[ 5 ] = new PyInt( (uint32)(point.z) );    // Z coordinate
    tuple0->items[ 6 ] = new PyInt( locationID );           // systemID

    res = tuple0;

    return res;
}

PyResult BookmarkService::Handle_UpdateBookmark(PyCallArgs &call)
{
    uint32 bookmarkID;
    uint32 ownerID;
    uint32 itemID;
    uint32 typeID;
    uint32 flag;
    std::string memo;
    std::string newMemo;
    uint64 created;
    double x;
    double y;
    double z;
    uint32 locationID;
    uint32 creatorID;
    uint32 folderID;
    std::string note;
    std::string newNote;

    if ( !(call.tuple->GetItem( 0 )->IsInt()) ) {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 0 ) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem( 0 )->TypeString() );
        return NULL;
    } else
        bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();

    if ( !(call.tuple->GetItem( 3 )->IsWString()) ) {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 3 ) is of the wrong type: '%s'.  Expected PyWString type.", call.client->GetName(), call.tuple->GetItem( 3 )->TypeString() );
        return NULL;
    } else
        newMemo = call.tuple->GetItem( 3 )->AsWString()->content();

    if( call.byname.find("note") == call.byname.end() ) {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\") could not be found.", call.client->GetName() );
        return NULL;
    } else {
        if( call.byname.find("note")->second->IsWString() )
            newNote = call.byname.find("note")->second->AsWString()->content();
        else if( call.byname.find("note")->second->IsString() )
            newNote = call.byname.find("note")->second->AsString()->content();
        else {
            sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\")->second is of the wrong type: '%s'.  Expected PyWString or PyString type.", call.client->GetName(), call.byname.find("note")->second->TypeString() );
            return NULL;
        }
    }

    m_db.GetBookmarkInformation
    (
        bookmarkID,
        ownerID,
        itemID,
        typeID,
        flag,
        memo,
        created,
        x,
        y,
        z,
        locationID,
        note,
        creatorID,
        folderID
    );

    m_db.UpdateBookmarkInDatabase(bookmarkID,call.client->GetCharacterID(),newMemo,newNote);

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyInt( itemID);                // itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invTypes
    tuple0->items[ 3 ] = new PyInt( (uint32)x );            // X
    tuple0->items[ 4 ] = new PyInt( (uint32)y );            // Y
    tuple0->items[ 5 ] = new PyInt( (uint32)z );            // Z
    tuple0->items[ 6 ] = new PyInt( locationID );           // systemID

    res = tuple0;

    return res;
}

PyResult BookmarkService::Handle_CreateFolder(PyCallArgs &call) {
    uint32 folderID = GetNextAvailableFolderID();
    std::string folderName = call.tuple->GetItem( 0 )->AsWString()->content();
    uint32 ownerID = call.client->GetCharacterID();
    uint32 creatorID = ownerID;

    m_db.SaveNewFolderToDatabase
    (
        folderID,
        folderName,
        ownerID,
        creatorID
    );

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 4 );

    tuple0->items[ 0 ] = new PyInt( ownerID );
    tuple0->items[ 1 ] = new PyInt( folderID );
    tuple0->items[ 2 ] = new PyString( folderName );
    tuple0->items[ 3 ] = new PyInt( creatorID );

    res = tuple0;

    return res;
}

PyResult BookmarkService::Handle_UpdateFolder(PyCallArgs &call) {
    uint32 folderID =  call.tuple->GetItem( 0 )->AsInt()->value();
    std::string folderName = call.tuple->GetItem( 1 )->AsWString()->content();
    uint32 ownerID = call.client->GetCharacterID();
    uint32 creatorID = ownerID;

    m_db.UpdateFolderInDatabase
    (
        folderID,
        folderName,
        ownerID,
        creatorID
    );

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 4 );

    tuple0->items[ 0 ] = new PyInt( ownerID );
    tuple0->items[ 1 ] = new PyInt( folderID );
    tuple0->items[ 2 ] = new PyString( folderName );
    tuple0->items[ 3 ] = new PyInt( creatorID );

    res = tuple0;

    return res;
}

PyResult BookmarkService::Handle_DeleteFolder(PyCallArgs &call) {
    uint32 folderID =  call.tuple->GetItem( 0 )->AsInt()->value();
    uint32 ownerID = call.client->GetCharacterID();

    m_db.DeleteFolderFromDatabase
    (
        folderID,
        ownerID
    );

    return(new PyNone());
}

PyResult BookmarkService::Handle_DeleteBookmarks(PyCallArgs &call)          //not working
{
  /**
00:48:20 [SvcCall] Service bookmark: calling DeleteBookmarks
00:48:20 L BookmarkService::Handle_DeleteBookmarks(): size= 1, 0 = ObjectEx
00:48:20 [SvcCall]   Call Arguments:
00:48:20 [SvcCall]       Tuple: 1 elements
00:48:20 [SvcCall]         [ 0] ObjectEx:
00:48:20 [SvcCall]         [ 0] Header:
00:48:20 [SvcCall]         [ 0]   Tuple: 2 elements
00:48:20 [SvcCall]         [ 0]     [ 0] Token: '__builtin__.set'
00:48:20 [SvcCall]         [ 0]     [ 1] Tuple: 1 elements
00:48:20 [SvcCall]         [ 0]     [ 1]   [ 0] List: 1 elements
00:48:20 [SvcCall]         [ 0]     [ 1]   [ 0]   [ 0] Integer field: 4
00:48:20 [SvcCall]         [ 0] List data:
00:48:20 [SvcCall]         [ 0]   Empty
00:48:20 [SvcCall]         [ 0] Dict data:
00:48:20 [SvcCall]         [ 0]   Empty
00:48:20 [SvcCall]   Call Named Arguments:
00:48:20 [SvcCall]     Argument 'machoVersion':
00:48:20 [SvcCall]         Integer field: 1
*/
  /* cant get python code to work...need more code for objectex in xmlp
    Call_DeleteBookmarks args;

    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    uint16 bookmarkID = call.byname["bookmarkID"]->AsInt()->value();
*/
  /*        orig code.....

    if(call.tuple->IsList())
    {
      sLog.Log( "BookmarkService::Handle_DeleteBookmarks()", "Call is PyList");
      PyList *list = call.tuple->GetItem( 0 )->AsList();
      uint32 i;
      uint32 bookmarkID;
      std::vector<unsigned long> bookmarkIDs;

      if( list->size() > 0 )
      {
          for(i=0; i<(list->size()); i++)
          {
              bookmarkID = call.tuple->GetItem( 0 )->AsList()->GetItem(i)->AsInt()->value();
              bookmarkIDs.push_back( bookmarkID );
          }

          m_db.DeleteBookmarksFromDatabase( call.client->GetCharacterID(),&bookmarkIDs );
      }else{
          sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple->GetItem( 0 )->AsList()->size() == 0.  Expected size >= 1.", call.client->GetName() );
          return NULL;
      }
    }else if(call.tuple->IsObjectEx())
    {
      sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "Call is ObjectEx.");
      return NULL;
    }else if(call.tuple->IsTuple())
    {
      sLog.Log( "BookmarkService::Handle_DeleteBookmarks()", "Call is PyTuple");

      //uint32 bookmarkID;
      //bookmarkID = call.tuple->GetItem( 0 )->AsObjectEx()->GetItem( 1 )->AsList();
      //m_db.DeleteBookmarkFromDatabase( call.client->GetCharacterID(), bookmarkID );

    }else{
      */
      sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "Service is not handled yet.  Returning NULL.");
  call.Dump(SERVICE__CALLS);
/*
    uint32 bookmarkID;
    if(call.tuple->IsObjectEx()) {
      PyObjectEx_Type2* bm_obj = (PyObjectEx_Type2*)bookmarkID;
      PyTuple* bm_tuple = bm_obj->GetArgs()->AsTuple();

      PyTuple* bookmarkID = new PyList();
      bookmarkID = call.tuple->GetItem( 0 )->AsObjectEx()->GetItem( 1 )->AsList();

      PyObjectEx* bm_object = call.tuple->GetItem( 0 )->AsObjectEx();
      PyList* bookmarkIDs = new PyList();
      bookmarkIDs = bm_object->GetItem( 1 )->AsList();
      if( bookmarkIDs->size() > 1 )
          m_db.DeleteBookmarksFromDatabase( call.client->GetCharacterID(), bookmarkIDs );  //error: no matching function for call to ‘BookmarkDB::DeleteBookmarksFromDatabase(uint32, PyTuple*)’
      else
          m_db.DeleteBookmarkFromDatabase( call.client->GetCharacterID(), *bookmarkIDs );  //error: no matching function for call to ‘BookmarkDB::DeleteBookmarkFromDatabase(uint32, PyTuple&)’
    }
  */
      return NULL;
      /*
    }

    return(new PyNone());
    */
}


PyResult BookmarkService::Handle_MoveBookmarksToFolder(PyCallArgs &call) {
  /**
23:39:40 E BookmarkService::Handle_MoveBookmarksToFolder(): Service is not handled yet.  Returning NULL.
23:39:40 [SvcCall]   Call Arguments:
23:39:40 [SvcCall]       Tuple: 2 elements
23:39:40 [SvcCall]         [ 0] Integer field: 2  <-folderID
23:39:40 [SvcCall]         [ 1] ObjectEx:
23:39:40 [SvcCall]         [ 1] Header:
23:39:40 [SvcCall]         [ 1]   Tuple: 2 elements
23:39:40 [SvcCall]         [ 1]     [ 0] Token: '__builtin__.set'
23:39:40 [SvcCall]         [ 1]     [ 1] Tuple: 1 elements
23:39:40 [SvcCall]         [ 1]     [ 1]   [ 0] List: 1 elements  <-bookmarkIDs as list
23:39:40 [SvcCall]         [ 1]     [ 1]   [ 0]   [ 0] Integer field: 13
23:39:40 [SvcCall]         [ 1] List data:
23:39:40 [SvcCall]         [ 1]   Empty
23:39:40 [SvcCall]         [ 1] Dict data:
23:39:40 [SvcCall]         [ 1]   Empty
*/
  /*
    Call_MoveBookmarksToFolder args;

    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    uint16 folderID = call.byname["folderID"]->AsInt()->value();
    //  need to get bookmarks as a list using call.byname["bookmarkID"]->AsList();
*/
      sLog.Error( "BookmarkService::Handle_MoveBookmarksToFolder()", "Service is not handled yet.  Returning NULL.");
  //call.Dump(SERVICE__CALLS);
      return NULL;
  //return(new PyNone());
  // needs either a 'real' return or nothing.....*SRVERROR* TypeError: 'NoneType' object is not iterable

    uint32 bookmarkID = 0;
    uint32 ownerID = call.client->GetCharacterID();
    uint32 folderID = call.tuple->GetItem( 0 )->AsInt()->value();

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 3 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyInt( ownerID);                // itemID
    tuple0->items[ 2 ] = new PyInt( folderID );           // systemID

    res = tuple0;

    return res;
}