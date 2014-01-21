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
    Author:        Zhur
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/BookmarkService.h"

// Set the maximum number for any user-created bookmark.
const uint32 BookmarkService::MAX_BOOKMARK_ID = 0xFFFFFFFF;

PyCallable_Make_InnerDispatcher(BookmarkService)

BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BookmarkService, GetBookmarks)
    PyCallable_REG_CALL(BookmarkService, BookmarkLocation)
    PyCallable_REG_CALL(BookmarkService, DeleteBookmarks)
    PyCallable_REG_CALL(BookmarkService, UpdateBookmark)
    PyCallable_REG_CALL(BookmarkService, CreateFolder)

    nextBookmarkID = 1000;
}


BookmarkService::~BookmarkService() {
    delete m_dispatch;
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
    uint32 bookmarkID = 0;
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
    // call.tuple
    //       |
    //       |--> [0] PyInt:      sends sol system id for solarsystem bm, shipID for coordinate bm.
    //       |--> [1] PyInt:       ownerID = charID of char making the bm
    //       |--> [2] PyWString:  label (called memo in db) for the bookmark
    //       \--> [3] PyString:  text for the "note" field in the bookmark
    //
    /** 13:13:35 L BookmarkService::Handle_BookmarkLocation(): size= 4, 0 = 12801626, 1 = Integer, 2 = WString, 3 = WString
        13:13:35 L BookmarkService::Handle_BookmarkLocation(): itemID = 140000000, typeID = 5   */
    ////////////////////////////////////////

      sLog.Debug( "BookmarkService::Handle_BookmarkLocation()", "size= %u, 0 = %u, 1 = %s, 2 = %s, 3 = %s", call.tuple->size(), call.tuple->GetItem(0)->TypeString(), call.tuple->GetItem(1)->TypeString(), call.tuple->GetItem(2)->TypeString(), call.tuple->GetItem(3)->TypeString()  );
    if ( (call.tuple->size() < 4) )
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple is of size %u, expected 4.", call.client->GetName(), call.tuple->size() );
        return NULL;
    }

    if ( !(call.tuple->IsTuple()) )
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple is of the wrong type: '%s'.  Expected PyTuple type.", call.client->GetName(), call.tuple->TypeString() );
        return NULL;
    }

    if ( !(call.tuple->GetItem(0)->IsInt()) )
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(0) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem(1)->TypeString() );
        return NULL;
    }
    else
    {
        typeCheck = call.tuple->GetItem( 0 )->AsInt()->value();  // when in space gives current ship/pos ID
        if ( typeCheck >= 140000000 )      // entity #'s above 140m are player-owned, therefore this is "spot in xxx system"
        {
          point = call.client->GetPosition();       // Get x,y,z location.  bm type is coordinate
          locationID = call.client->GetLocationID();       // locationID of bm is current sol system
        }else{      // client is passing systemID from map.  char is marking a solar systemID
          point.x, point.y, point.z = 0, 0, 0;      // no x,y,z location.  bm type is solar system
          locationID = call.tuple->GetItem( 0 )->AsInt()->value();  // this is systemID from map
        }
        itemID = locationID;      //  locationID = itemID for coord bm.  shows jumps, s/c/r in bm window, green if in system
    }
    if ( !(call.tuple->GetItem(1)->IsInt()) )
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(1) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem(1)->TypeString() );
        return NULL;
    }
    else
        ownerID = call.tuple->GetItem( 1 )->AsInt()->value();

    if ( call.tuple->GetItem( 2 )->IsString() )
        memo = call.tuple->GetItem( 2 )->AsString()->content();
    else if ( call.tuple->GetItem( 2 )->IsWString() )
        memo = call.tuple->GetItem( 2 )->AsWString()->content();
    else
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(2) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(2)->TypeString() );
        return NULL;
    }

    if ( call.tuple->GetItem( 3 )->IsString() )
        note = call.tuple->GetItem( 3 )->AsString()->content();
    else if ( call.tuple->GetItem( 3 )->IsWString() )
        note = call.tuple->GetItem( 3 )->AsWString()->content();
    else
    {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "%s: call.tuple->GetItem(3) is of the wrong type: '%s'.  Expected PyString or PyWString type.", call.client->GetName(), call.tuple->GetItem(3)->TypeString() );
        return NULL;
    }

    // Get typeID for this itemID:
    typeID = m_db.FindBookmarkTypeID(typeCheck);


      sLog.Log( "BookmarkService::Handle_BookmarkLocation()", "itemID = %u, typeID = %u", itemID, typeID );
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
    // (bookmarkID, itemID, typeID, x, y, z, locationID,)
    ////////////////////////////////////////

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyInt( itemID );               // in some cases, this is the itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invTypes
    tuple0->items[ 3 ] = new PyInt( (uint32)(point.x) );    // X coordinate
    tuple0->items[ 4 ] = new PyInt( (uint32)(point.y) );    // Y coordinate
    tuple0->items[ 5 ] = new PyInt( (uint32)(point.z) );    // Z coordinate
    tuple0->items[ 6 ] = new PyInt( locationID );           // systemID

    res = tuple0;

    return res;
}

/**  is no longer pylist type...*/
PyResult BookmarkService::Handle_DeleteBookmarks(PyCallArgs &call)
{
    if(call.tuple->IsList())
    {
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
      }/*
        sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple is of the wrong type: '%s'.  Expected PyList type.", call.client->GetName(), call.tuple->TypeString() );
        sLog.Warning( "BookmarkService::Handle_DeleteBookmarks()", "  tuple->size: %u",call.tuple->size() );
        return NULL;*/
    }else if(call.tuple->IsTuple())
    {
      uint32 bookmarkID;
      std::vector<unsigned long> bookmarkIDs;

      bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();
      bookmarkIDs.push_back( bookmarkID );
      m_db.DeleteBookmarksFromDatabase( call.client->GetCharacterID(),&bookmarkIDs );
      return(new PyNone());
      /*
        sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple is of the wrong type: '%s'.  Expected Tuple  type.", call.client->GetName(), call.tuple->TypeString() );
        sLog.Warning( "BookmarkService::Handle_DeleteBookmarks()", "  tuple->size: %u",call.tuple->size() );
        return NULL;*/
    }else{
      sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "Call is neither List nor tuple.  Returning NULL.");
      return NULL;
    }
}

PyResult BookmarkService::Handle_UpdateBookmark(PyCallArgs &call)
{
    uint32 bookmarkID;
    uint32 ownerID;
    uint32 itemID;
    uint32 typeID;
    uint32 flag;
    std::string memo;
    uint64 created;
    double x;
    double y;
    double z;
    uint32 locationID;
    std::string newLabel;
    std::string newNote;
    uint32 creatorID;
    uint32 folderID;
    std::string note;

    // bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value()
    if ( !(call.tuple->GetItem( 0 )->IsInt()) )
    {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 0 ) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->GetItem( 0 )->TypeString() );
        return NULL;
    }
    else
        bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();

    // newLabel = call.tuple->GetItem( 3 )->AsWString()->content()
    if ( !(call.tuple->GetItem( 3 )->IsWString()) )
    {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.tuple->GetItem( 3 ) is of the wrong type: '%s'.  Expected PyWString type.", call.client->GetName(), call.tuple->GetItem( 3 )->TypeString() );
        return NULL;
    }
    else
        newLabel = call.tuple->GetItem( 3 )->AsWString()->content();

    //call.byname.find("note") == call.byname.end()
    //newNote = call.byname.find("note")->second->AsWString() when note != ""
    //newNote = call.byname.find("note")->second->AsString() when note == ""
    if( call.byname.find("note") == call.byname.end() )
    {
        sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\") could not be found.", call.client->GetName() );
        return NULL;
    }
    else
    {
        if( call.byname.find("note")->second->IsWString() )
            newNote = call.byname.find("note")->second->AsWString()->content();
        else if( call.byname.find("note")->second->IsString() )
            newNote = call.byname.find("note")->second->AsString()->content();
        else
        {
            sLog.Error( "BookmarkService::Handle_UpdateBookmark()", "%s: call.byname.find(\"note\")->second is of the wrong type: '%s'.  Expected PyWString or PyString type.", call.client->GetName(), call.byname.find("note")->second->TypeString() );
            return NULL;
        }
    }

    // Take newLabel and newNote to create new 'memo' value:
    memo = newLabel + newNote;

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

    m_db.UpdateBookmarkInDatabase(bookmarkID,call.client->GetCharacterID(),memo);

    PyTuple* res = NULL;

    PyTuple* tuple0 = new PyTuple( 7 );

    tuple0->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
    tuple0->items[ 1 ] = new PyNone();                      // in some cases, this is the itemID
    tuple0->items[ 2 ] = new PyInt( typeID );               // typeID from invTypes
    tuple0->items[ 3 ] = new PyInt( (uint32)x );            // Sample X coordinate
    tuple0->items[ 4 ] = new PyInt( (uint32)y );            // Sample Y coordinate
    tuple0->items[ 5 ] = new PyInt( (uint32)z );            // Sample Z coordinate
    tuple0->items[ 6 ] = new PyInt( locationID );           // Sample systemID

    res = tuple0;

    return res;
}

//   18:11:46 L BookmarkService::Handle_CreateFolder():   tuple->size: 1
PyResult BookmarkService::Handle_CreateFolder(PyCallArgs &call) {
    sLog.Log( "BookmarkService::Handle_CreateFolder()", "  tuple->size: %u, tuple->type:  '%s'",call.tuple->size(), call.tuple->GetItem( 0 )->TypeString() );

    return NULL;
}
