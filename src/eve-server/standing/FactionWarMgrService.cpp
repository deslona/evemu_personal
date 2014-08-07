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
#include "cache/ObjCacheService.h"
#include "standing/FactionWarMgrService.h"

PyCallable_Make_InnerDispatcher(FactionWarMgrService)

FactionWarMgrService::FactionWarMgrService(PyServiceMgr *mgr)
: PyService(mgr, "facWarMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FactionWarMgrService, GetWarFactions);
    PyCallable_REG_CALL(FactionWarMgrService, GetFWSystems);
    PyCallable_REG_CALL(FactionWarMgrService, GetMyCharacterRankOverview);
    PyCallable_REG_CALL(FactionWarMgrService, GetMyCharacterRankInfo);
    PyCallable_REG_CALL(FactionWarMgrService, GetFactionMilitiaCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, GetCharacterRankInfo);
    PyCallable_REG_CALL(FactionWarMgrService, GetFactionalWarStatus);
    PyCallable_REG_CALL(FactionWarMgrService, IsEnemyFaction);
    PyCallable_REG_CALL(FactionWarMgrService, JoinFactionAsCharacter);

	/*
        return self.facWarMgr.IsEnemyFaction(enemyID, factionID)
        return self.facWarMgr.IsEnemyCorporation(enemyID, factionID)
        return self.facWarMgr.GetSystemsConqueredThisRun()
        ret = self.facWarMgr.GetCorporationWarFactionID(corpID)
        return self.facWarMgr.GetFactionCorporations(factionID)
         self.facWarMgr.JoinFactionAsCharacterRecommendationLetter, factionID, itemID)
            self.facWarMgr.JoinFactionAsAlliance(factionID)
            self.facWarMgr.JoinFactionAsCorporation(factionID)
        return self.facWarMgr.GetStats_FactionInfo()
            self.topStats = self.facWarMgr.GetStats_TopAndAllKillsAndVPs()
        for k, v in self.facWarMgr.GetStats_Character().items():
        for k, v in self.facWarMgr.GetStats_Corp().items():
        for k, v in self.facWarMgr.GetStats_Alliance().items():
        return self.facWarMgr.GetStats_Militia()
        return self.facWarMgr.GetStats_CorpPilots()
        status = self.facWarMgr.GetSystemStatus(session.solarsystemid2, session.warfactionid)
            */
}

FactionWarMgrService::~FactionWarMgrService()
{
    delete m_dispatch;
}

PyResult FactionWarMgrService::Handle_GetWarFactions(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetWarFactions");

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep *res = m_db.GetWarFactions();
        if(res == NULL)
            return NULL;
        m_manager->cache_service->GiveCache(method_id, &res);
    }

    return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

PyResult FactionWarMgrService::Handle_GetFWSystems( PyCallArgs& call )
{
    ObjectCachedMethodID method_id( GetName(), "GetFacWarSystems" );

    if( !m_manager->cache_service->IsCacheLoaded( method_id ) )
    {
        PyRep* res = m_db.GetWarFactions();
        if( res == NULL )
            return NULL;

        m_manager->cache_service->GiveCache( method_id, &res );
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult( method_id );
}

PyResult FactionWarMgrService::Handle_GetMyCharacterRankOverview( PyCallArgs& call ) {
  /**
00:16:11 [PacketError] Decode Call_SingleIntegerArg failed: tuple0 is the wrong size: expected 1, but got 0
00:16:11 [SvcError] Failed to decode args.

    Call_SingleIntegerArg arg;
    if( !arg.Decode( &call.tuple ) )
    {
        _log( SERVICE__ERROR, "Failed to decode args." );
        return NULL;
    }
*/
  sLog.Log( "FactionWarMgrService::Handle_GetMyCharacterRankOverview()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
// will need data from DB...
  util_Rowset rs;

    rs.header.push_back( "currentRank" );
    rs.header.push_back( "highestRank" );
    rs.header.push_back( "factionID" );
    rs.header.push_back( "lastModified" );

    return rs.Encode();
}

PyResult FactionWarMgrService::Handle_GetMyCharacterRankInfo( PyCallArgs& call ) {
  sLog.Log( "FactionWarMgrService::Handle_GetMyCharacterRankInfo()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
  util_Rowset rs;

    rs.header.push_back( "currentRank" );
    rs.header.push_back( "highestRank" );
    rs.header.push_back( "factionID" );
    rs.header.push_back( "lastModified" );

    return rs.Encode();
}

PyResult FactionWarMgrService::Handle_GetFactionMilitiaCorporation(PyCallArgs &call) {
  sLog.Log( "FactionWarMgrService::Handle_GetFactionMilitiaCorporation()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }
    return(new PyInt(m_db.GetFactionMilitiaCorporation(arg.arg)));
}

PyResult FactionWarMgrService::Handle_GetCharacterRankInfo(PyCallArgs &call) {
  sLog.Log( "FactionWarMgrService::Handle_GetCharacterRankInfo()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

//22:48:28 L FactionWarMgrService::Handle_GetFactionalWarStatus(): size= 0
PyResult FactionWarMgrService::Handle_GetFactionalWarStatus(PyCallArgs &call) {
  sLog.Log( "FactionWarMgrService::Handle_GetFactionalWarStatus()", "size=%u ", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

//22:48:28 L FactionWarMgrService::Handle_IsEnemyFaction(): size= 2
PyResult FactionWarMgrService::Handle_IsEnemyFaction(PyCallArgs &call) {
  sLog.Log( "FactionWarMgrService::Handle_IsEnemyFaction()", "size=%u ", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

PyResult FactionWarMgrService::Handle_JoinFactionAsCharacter(PyCallArgs &call) {
  sLog.Log( "FactionWarMgrService::Handle_JoinFactionAsCharacter()", "size=%u ", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

  return NULL;
}

/**
    def LeaveFactionAsAlliance(self, factionID):
        self.facWarMgr.LeaveFactionAsAlliance(factionID)

    def LeaveFactionAsCorporation(self, factionID):
        self.facWarMgr.LeaveFactionAsCorporation(factionID)

    def WithdrawJoinFactionAsAlliance(self, factionID):
        self.facWarMgr.WithdrawJoinFactionAsAlliance(factionID)

    def WithdrawJoinFactionAsCorporation(self, factionID):
        self.facWarMgr.WithdrawJoinFactionAsCorporation(factionID)

    def WithdrawLeaveFactionAsAlliance(self, factionID):
        self.facWarMgr.WithdrawLeaveFactionAsAlliance(factionID)

    def WithdrawLeaveFactionAsCorporation(self, factionID):
        self.facWarMgr.WithdrawLeaveFactionAsCorporation(factionID)

    def GetFactionalWarStatus(self):
        return self.facWarMgr.GetFactionalWarStatus()

    def GetWarFactions(self):
        return self.facWarMgr.GetWarFactions()

    def GetCharacterRankInfo(self, charID, corpID = None):
        if corpID is None or self.GetCorporationWarFactionID(corpID) is not None:
            if charID == session.charid:
                return self.facWarMgr.GetMyCharacterRankInfo()
            else:
                return self.facWarMgr.GetCharacterRankInfo(charID)

    def GetCharacterRankOverview(self, charID):
        if not charID == session.charid:
            return None
        return self.facWarMgr.GetMyCharacterRankOverview()

    def RefreshCorps(self):
        return self.facWarMgr.RefreshCorps()

*/

