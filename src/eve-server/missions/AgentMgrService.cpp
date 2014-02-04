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
        PyCallable_REG_CALL(AgentMgrBound, GetMissionBriefingInfo)
        PyCallable_REG_CALL(AgentMgrBound, GetAgentLocationWrap)
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
    PyCallable_DECL_CALL(GetMissionBriefingInfo)
    PyCallable_DECL_CALL(GetAgentLocationWrap)
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
    _log(CLIENT__MESSAGE, "%s bind request for:", GetName());
    bind_args->Dump(CLIENT__MESSAGE, "    ");

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


PyResult AgentMgrService::Handle_GetAgents(PyCallArgs &call) {
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

PyResult AgentMgrService::Handle_GetMyJournalDetails(PyCallArgs &call) {
    PyRep *result = NULL;

    PyTuple *t = new PyTuple(3);
    t->items[0] = new PyList();
    t->items[1] = new PyList();
    t->items[2] = new PyList();
    result = t;

    return result;
}

PyResult AgentMgrService::Handle_GetMyEpicJournalDetails( PyCallArgs& call )
{
    //no args

  sLog.Log( "AgentMgrBound::Handle_GetMyEpicJournalDetails()", "size= %u", call.tuple->size() );

    return new PyList;
}

//21:02:49 L AgentMgrBound::Handle_GetSolarSystemOfAgent(): size= 1
PyResult AgentMgrService::Handle_GetSolarSystemOfAgent(PyCallArgs &call)
{
  //uint8 size = call.tuple->size();
  //uint16 int1 = call.tuple->GetItem(0)->AsInt()->value();
  //uint16 int2 = call.tuple->GetItem(1)->AsInt()->value();
  sLog.Log( "AgentMgrService::Handle_GetSolarSystemOfAgent()", "size= %u, 0=%s", call.tuple->size(), call.tuple->GetItem( 0 )->TypeString() );

    return new PyInt( 0 );
}

PyResult AgentMgrBound::Handle_GetInfoServiceDetails( PyCallArgs& call )
{
    //takes no arguments

  sLog.Log( "AgentMgrBound::Handle_GetInfoServiceDetails()", "size= %u", call.tuple->size() );

    return new PyNone;
}

//21:13:12 L AgentMgrBound::Handle_DoAction(): size= 1
PyResult AgentMgrBound::Handle_DoAction(PyCallArgs &call) {
  sLog.Log( "AgentMgrBound::Handle_DoAction()", "size= %u, 0=%s", call.tuple->size(), call.tuple->GetItem( 0 )->TypeString() );
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
    if( !(args.arg->IsInt()) )
    {
        sLog.Error( "AgentMgrBound::Handle_DoAction()", "args.arg->IsInt() failed.  Expected type Int, got type %s", args.arg->TypeString() );
    }
    else
        m_agent->DoAction( call.client, args.arg->AsInt()->value(), res.agentSays, choices );

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


PyResult AgentMgrBound::Handle_GetMyJournalDetails(PyCallArgs &call) {
  sLog.Log( "AgentMgrBound::Handle_GetMyJournalDetails()", "size= %u", call.tuple->size() );
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

/** not handled */
//21:13:12 L AgentMgrBound::Handle_GetMissionBriefingInfo(): size= 0
PyResult AgentMgrBound::Handle_GetMissionBriefingInfo(PyCallArgs &call)
{
    return new PyInt( 0 );
}

//21:13:12 L AgentMgrBound::Handle_GetAgentLocationWrap(): size= 0
PyResult AgentMgrBound::Handle_GetAgentLocationWrap(PyCallArgs &call)
{
    return new PyInt( 0 );
}

PyResult AgentMgrBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{
  sLog.Log( "AgentMgrBound::Handle_GetMissionObjectiveInfo()", "size= %u", call.tuple->size() );

    return new PyInt( 0 );
}

PyResult AgentMgrService::Handle_GetCareerAgents(PyCallArgs &call)
{
    sLog.Debug("Server", "Called GetCareerAgents Stub.");

    return new PyInt( 0 );
}
