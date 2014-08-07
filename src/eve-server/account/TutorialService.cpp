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
#include "account/TutorialService.h"

PyCallable_Make_InnerDispatcher(TutorialService)

TutorialService::TutorialService(PyServiceMgr *mgr)
: PyService(mgr, "tutorialSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(TutorialService, GetTutorialInfo);
    PyCallable_REG_CALL(TutorialService, GetTutorials);
    PyCallable_REG_CALL(TutorialService, GetCriterias);
    PyCallable_REG_CALL(TutorialService, GetCategories);
    PyCallable_REG_CALL(TutorialService, GetContextHelp);
    PyCallable_REG_CALL(TutorialService, GetCharacterTutorialState);
    PyCallable_REG_CALL(TutorialService, GetTutorialsAndConnections);
    PyCallable_REG_CALL(TutorialService, GetCareerAgents);
}

TutorialService::~TutorialService() {
    delete m_dispatch;
}

PyResult TutorialService::Handle_GetTutorialInfo(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetTutorialInfo()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    Call_GetTutorialInfo args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Can't parse args.");
        return NULL;
    }

    Rsp_GetTutorialInfo rsp;

    rsp.pagecriterias = m_db.GetPageCriterias(args.tutorialID);
    if(rsp.pagecriterias == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting pagecriterias for tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.pages = m_db.GetPages(args.tutorialID);
    if(rsp.pages == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting pages for tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.tutorial = m_db.GetTutorial(args.tutorialID);
    if(rsp.tutorial == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting tutorial %u.", args.tutorialID);
        return NULL;
    }

    rsp.criterias = m_db.GetTutorialCriterias(args.tutorialID);
    if(rsp.criterias == NULL) {
        codelog(SERVICE__ERROR, "An error occured while getting criterias for tutorial %u.", args.tutorialID);
        return NULL;
    }

    return(rsp.Encode());
}

PyResult TutorialService::Handle_GetTutorials(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetTutorials()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    return(m_db.GetAllTutorials());
}

PyResult TutorialService::Handle_GetCriterias(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetCriterias()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    return(m_db.GetAllCriterias());
}

PyResult TutorialService::Handle_GetCategories(PyCallArgs &call) {
  sLog.Log( "TutorialService::Handle_GetCategories()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
    return(m_db.GetCategories());
}

PyResult TutorialService::Handle_GetContextHelp( PyCallArgs& call )
{
  sLog.Log( "TutorialService::Handle_GetContextHelp()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);

    return new PyList;
}

//00:25:53 L TutorialService::Handle_GetCharacterTutorialState(): size= 0
PyResult TutorialService::Handle_GetCharacterTutorialState( PyCallArgs& call ) {
  /*  Empty Call  */

    return new PyInt( 0 );
}

//00:25:58 L TutorialService::Handle_GetTutorialsAndConnections(): size= 0
PyResult TutorialService::Handle_GetTutorialsAndConnections( PyCallArgs& call ) {
  /*  Empty Call  */
/**
            t, tc = sm.RemoteSvc('tutorialSvc').GetTutorialsAndConnections()
            self.tutorials = t.Index('tutorialID')
            tc = tc.Filter('tutorialID')
            self.tutorialConnections = defaultdict(dict)
            for tutID, rows in tc.iteritems():
                for each in rows:
                    self.tutorialConnections[tutID][each.raceID] = each.nextTutorialID

AttributeError: 'NoneType' object has no attribute 'iteritems'
 */

    return new PyInt( 0 );
}

PyResult TutorialService::Handle_GetCareerAgents( PyCallArgs& call ) {
  /*  Empty Call  */
  /**
        agentMapping = sm.RemoteSvc('tutorialSvc').GetCareerAgents()
        for careerType in agentMapping:
            agentIDs = []
            if careerType not in self.careerAgents:
                self.careerAgents[careerType] = {}
                self.careerAgents[careerType]['agent'] = {}
                self.careerAgents[careerType]['station'] = {}
            agentIDs = agentMapping.get(careerType, [])
            agents = sm.RemoteSvc('tutorialSvc').GetTutorialAgents(agentIDs)
            for agent in agents:
                self.careerAgents[careerType]['agent'][agent.agentID] = agent
                self.careerAgents[careerType]['station'][agent.agentID] = sm.GetService('map').GetStation(agent.stationID)
*/

    return new PyInt( 0 );
}


/**
            sm.RemoteSvc('tutorialSvc').LogCompleted(tutorialID, pageNo, int(time))
        elif status == 'aborted':
            stat[sequenceID] = 'done'
            sm.RemoteSvc('tutorialSvc').LogAborted(tutorialID, pageNo, int(time))

                categories = sm.RemoteSvc('tutorialSvc').GetCategories()
                for category in categories:
                    self.categories[category.categoryID] = category
                    self.categories[category.categoryID].categoryName = localization.GetByMessageID(category.categoryNameID)
                    self.categories[category.categoryID].description = localization.GetByMessageID(category.descriptionID)

                criterias = sm.RemoteSvc('tutorialSvc').GetCriterias()
                for criteria in criterias:
                    self.criterias[criteria.criteriaID] = criteria
            actions = sm.RemoteSvc('tutorialSvc').GetActions()
            for action in actions:
                self.actions[action.actionID] = action

            tutData = sm.RemoteSvc('tutorialSvc').GetTutorialInfo(tutorialID)
                sm.RemoteSvc('tutorialSvc').LogAppClosed(tutorialID, pageNo, int(time))
                        sm.RemoteSvc('tutorialSvc').LogClosed(tutorialID, pageNo, int(time))
                    sm.RemoteSvc('tutorialSvc').LogStarted(tutorialID, pageNo, int(time))
            sm.RemoteSvc('tutorialSvc').LogCompleted(tutorialID, pageNo, int(time))
        return sm.RemoteSvc('tutorialLocationSvc').GiveTutorialGoodies(tutorialID, pageID, pageNo)
                tutData = sm.RemoteSvc('tutorialSvc').GetTutorialInfo(VID)

        rs = sm.RemoteSvc('tutorialSvc').GetCharacterTutorialState()
        if not rs or len(rs) == 0:
            return



            */