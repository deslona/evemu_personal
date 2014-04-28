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

/*
 * AgentMgr bound(agentID):
 *  -> DoAction(actionID or None)
 *  -> WarpToLocation(locationType, locationNumber, warpRange, is_gang)
 *
 *   Also sent an OnRemoteMessage(AgtMissionOfferWarning)
 *
 *   and various OnAgentMissionChange()
 *
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "missions/Agent.h"
#include "missions/AgentMgrService.h"

class AgentMgrBound
: public PyBoundObject {
public:

    PyCallable_Make_Dispatcher(AgentMgrBound)

    AgentMgrBound(PyServiceMgr *mgr, MissionDB *db, Agent *agt)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this)),
      m_agent(agt)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "AgentMgrBound";

        PyCallable_REG_CALL(AgentMgrBound, GetInfoServiceDetails)
        PyCallable_REG_CALL(AgentMgrBound, DoAction)
        PyCallable_REG_CALL(AgentMgrBound, GetMyJournalDetails)
        PyCallable_REG_CALL(AgentMgrBound, GetAgentLocationWrap)
        PyCallable_REG_CALL(AgentMgrBound, GetMissionBriefingInfo)
        PyCallable_REG_CALL(AgentMgrBound, GetMissionObjectiveInfo)
    }
    virtual ~AgentMgrBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetInfoServiceDetails)
    PyCallable_DECL_CALL(DoAction)
    PyCallable_DECL_CALL(GetMyJournalDetails)
    PyCallable_DECL_CALL(GetAgentLocationWrap)
    PyCallable_DECL_CALL(GetMissionBriefingInfo)
    PyCallable_DECL_CALL(GetMissionObjectiveInfo)

protected:
    MissionDB *const m_db;        //we do not own this
    Dispatcher *const m_dispatch;    //we own this
    Agent *const m_agent;    //we do not own this.
};

PyCallable_Make_InnerDispatcher(AgentMgrService)

AgentMgrService::AgentMgrService(PyServiceMgr *mgr)
: PyService(mgr, "agentMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AgentMgrService, GetAgents)
    PyCallable_REG_CALL(AgentMgrService, GetMyJournalDetails)
    PyCallable_REG_CALL(AgentMgrService, GetMyEpicJournalDetails)
    PyCallable_REG_CALL(AgentMgrService, GetSolarSystemOfAgent)
    PyCallable_REG_CALL(AgentMgrService, GetCareerAgents)
}

AgentMgrService::~AgentMgrService() {
    delete m_dispatch;
    std::map<uint32, Agent *>::iterator cur, end;
    cur = m_agents.begin();
    end = m_agents.end();
    for(; cur != end; cur++) {
        delete cur->second;
    }
}

Agent *AgentMgrService::_GetAgent(uint32 agentID) {
    std::map<uint32, Agent *>::iterator res;
    res = m_agents.find(agentID);
    if(res != m_agents.end())
        return(res->second);
    Agent *a = new Agent(agentID);
    if(!a->Load(&m_db)) {
        delete a;
        return NULL;
    }
    m_agents[agentID] = a;
    return(a);
}

PyBoundObject *AgentMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
        return NULL;
    }

    uint32 agentID = bind_args->AsInt()->value();

    Agent *agent = _GetAgent(agentID);
    if(agent == NULL) {
        codelog(CLIENT__ERROR, "%s: Unable to obtain agent %u", c->GetName(), agentID);
        return NULL;
    }

    return(new AgentMgrBound(m_manager, &m_db, agent));
}


//20:49:34 L AgentMgrService::Handle_GetAgents(): size= 0
PyResult AgentMgrService::Handle_GetAgents(PyCallArgs &call) {
  /*
22:07:37 L AgentMgrService::Handle_GetAgents(): size= 0
22:07:37 [SvcCall]   Call Arguments:
22:07:37 [SvcCall]       Tuple: Empty
22:07:37 [SvcCall]   Call Named Arguments:
22:07:37 [SvcCall]     Argument 'machoVersion':
22:07:37 [SvcCall]         List: 2 elements
22:07:37 [SvcCall]           [ 0] Integer field: 130409676630000000
22:07:37 [SvcCall]           [ 1] Integer field: -164492702
  sLog.Log( "AgentMgrService::Handle_GetAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    */

    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetAgents");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetAgents();
        if(result == NULL) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            result = new PyNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

//15:39:45 L AgentMgrBound::Handle_DoAction(): size= 1, 0=None
PyResult AgentMgrBound::Handle_DoAction(PyCallArgs &call) {
  /*
17:00:59 L AgentMgrBound::Handle_DoAction(): size= 1, 0=None
17:00:59 [SvcCall]   Call Arguments:
17:00:59 [SvcCall]       Tuple: 1 elements
17:00:59 [SvcCall]         [ 0] (None)
17:00:59 [SvcCall]   Call Arguments:
17:00:59 [SvcCall]       Tuple: Empty
17:00:59 [SvcCall]   Call Named Arguments:
17:00:59 [SvcCall]     Argument 'machoVersion':
17:00:59 [SvcCall]         Integer field: 1
  sLog.Log( "AgentMgrBound::Handle_DoAction()", "size= %u, 0=%s", call.tuple->size(), call.tuple->GetItem( 0 )->TypeString() );
    call.Dump(SERVICE__CALLS);
    */
    //takes a single argument, which may be None, or may be an integer actionID
    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode args from '%s'", call.client->GetName());
        return NULL;
    }

    //TODO: send loyaltyPoints in the keywords return.
    //uint32 loyaltyPoints = m_agent->GetLoyaltyPoints(call.client);

    DoAction_Result res;
    res.dialogue = new PyList;

    std::map<uint32, std::string> choices;
//00:20:34 E AgentMgrBound::Handle_DoAction(): args.arg->IsInt() failed.  Expected type Int, got type None
    if(args.arg->IsInt())
        m_agent->DoAction( call.client, args.arg->AsInt()->value(), res.agentSays, choices );
    else   //* EVE client calls DoAction without parameters, let's see where it leads.
        m_agent->DoAction( call.client, 0, res.agentSays, choices );

    DoAction_Dialogue_Item choice;

    std::map<uint32, std::string>::iterator cur, end;
    cur = choices.begin();
    end = choices.end();
    for(; cur != end; cur++)
    {
        choice.actionID = cur->first;
        choice.actionText = cur->second;

        res.dialogue->AddItem( choice.Encode() );
    }

    return res.Encode();
}


//21:13:12 L AgentMgrBound::Handle_GetAgentLocationWrap(): size= 0
PyResult AgentMgrBound::Handle_GetAgentLocationWrap(PyCallArgs &call)
{
    return m_agent->GetLocation();
}

//21:13:12 L AgentMgrBound::Handle_GetMissionBriefingInfo(): size= 0
PyResult AgentMgrBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
  sLog.Log( "AgentMgrBound::Handle_GetMissionBriefingInfo()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    PyDict *res = new PyDict();

    res->SetItem("ContentID", new PyInt(123) ) ;
    res->SetItem("Mission Keywords", new PyString("Mission Keywords"));
    res->SetItem("Mission Title ID", new PyString("Mission Title ID") );
    res->SetItem("Mission Briefing ID", new PyString("Mission Briefing ID") );
    res->SetItem("Decline Time", new PyFloat( Win32TimeNow() + Win32Time_Hour ) );
    res->SetItem("Expiration Time", new PyFloat( Win32TimeNow()+Win32Time_Day ) );
    res->SetItem("Mission Image", new PyString("MissionImage") );

    return res;
}


/** not handled */
PyResult AgentMgrService::Handle_GetMyJournalDetails(PyCallArgs &call) {
  sLog.Log( "AgentMgrService::Handle_GetMyJournalDetails()", "size= %u", call.tuple->size() );
  /*
20:12:42 [SvcCall] Service agentMgr: calling GetMyJournalDetails
20:12:42 [SvcCall]   Call Arguments:
20:12:42 [SvcCall]       Tuple: Empty
20:12:42 [SvcCall]   Call Named Arguments:
20:12:42 [SvcCall]     Argument 'machoVersion':
20:12:42 [SvcCall]         Integer field: 1
    call.Dump(SERVICE__CALLS);
    */
    PyRep *result = NULL;
    PyTuple *t = new PyTuple(3);
    //missions:
    t->items[0] = new PyList();
    //offers:
    t->items[1] = new PyList();
    //research:
    t->items[2] = new PyList();
    result = t;

    return result;
}

PyResult AgentMgrBound::Handle_GetMyJournalDetails(PyCallArgs &call) {
  sLog.Log( "AgentMgrBound::Handle_GetMyJournalDetails()", "size= %u", call.tuple->size() );

}

PyResult AgentMgrService::Handle_GetMyEpicJournalDetails( PyCallArgs& call )
{
    //no args

  sLog.Log( "AgentMgrBound::Handle_GetMyEpicJournalDetails()", "size= %u", call.tuple->size() );

    return new PyList;
}

///  this really needs to be a cached object....
PyResult AgentMgrService::Handle_GetSolarSystemOfAgent(PyCallArgs &call)
{/*
  uint8 size = call.tuple->size();
  uint32 int1 = call.tuple->GetItem(0)->AsInt()->value();
  sLog.Log( "AgentMgrService::Handle_GetSolarSystemOfAgent()", "size= %u, 0=%s(%u)", size, call.tuple->GetItem( 0 )->TypeString(), int1 );
22:28:49 [SvcCall]   Call Arguments:
22:28:49 [SvcCall]       Tuple: 1 elements
22:28:49 [SvcCall]         [ 0] Integer field: 3019442      // <- this value increments @ 10/sec
22:28:49 [SvcCall]   Call Named Arguments:
22:28:49 [SvcCall]     Argument 'machoVersion':
22:28:49 [SvcCall]         Integer field: 1

    call.Dump(SERVICE__CALLS);
    */

    DBQueryResult res;

    uint32 AgentID = call.tuple->GetItem(0)->AsInt()->value();

    if(!sDatabase.RunQuery(res,
        " SELECT"
        "  a.locationID,"
        "  s.solarSystemID"
        " FROM agtAgents AS a"
        "  LEFT JOIN staStations AS s ON s.stationID = a.locationID"
        " WHERE a.agentID = %u",AgentID))
    {
        _log(DATABASE__ERROR, "Failed to query for Agent = %u", AgentID);
    }
    //return (DBResultToRowset(res));
    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__ERROR, "SystemID of Agent %u not found.", AgentID);
        return NULL;
    }

    PyTuple *t = new PyTuple(1);
    t->items[0] = new PyInt( row.GetUInt(1) );

    return t;

}

PyResult AgentMgrService::Handle_GetCareerAgents(PyCallArgs &call)
{
  sLog.Log( "AgentMgrBound::Handle_GetCareerAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

    return new PyInt( 0 );
}

//17:09:07 L AgentMgrBound::Handle_GetInfoServiceDetails(): size= 0
PyResult AgentMgrBound::Handle_GetInfoServiceDetails( PyCallArgs& call ) {
  sLog.Log( "AgentMgrBound::Handle_GetInfoServiceDetails()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    //takes no arguments
    return new PyNone;
}

//15:46:37 L AgentMgrBound::Handle_GetMissionObjectiveInfo(): size= 0
PyResult AgentMgrBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{/*     called when clicking on line item (actionText)
06:08:55 L AgentMgrBound::Handle_GetMissionObjectiveInfo(): size= 0
06:08:55 [SvcCall]   Call Arguments:
06:08:55 [SvcCall]       Tuple: Empty
06:08:55 [SvcCall]   Call Named Arguments:
06:08:55 [SvcCall]     Argument 'machoVersion':
06:08:55 [SvcCall]         Integer field: 1
*/
    return new PyInt( 0 );
}
