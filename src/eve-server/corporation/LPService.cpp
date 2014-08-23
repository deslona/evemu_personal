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
    Author:        Bloody.Rabbit,   Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/LPService.h"

PyCallable_Make_InnerDispatcher(LPService)

LPService::LPService(PyServiceMgr *mgr)
: PyService(mgr, "LPSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(LPService, TakeOffer);
    PyCallable_REG_CALL(LPService, ExchangeConcordLP);
    PyCallable_REG_CALL(LPService, GetLPExchangeRates);
    PyCallable_REG_CALL(LPService, GetLPsForCharacter);
    PyCallable_REG_CALL(LPService, GetLPForCharacterCorp);
    PyCallable_REG_CALL(LPService, GetAvailableOffersFromCorp);

}

LPService::~LPService()
{
    delete m_dispatch;
}

PyResult LPService::Handle_TakeOffer( PyCallArgs& call )
{/**
        sm.RemoteSvc('LPSvc').ExchangeConcordLP(corpID, amount)
        */
  sLog.Log( "LPService::Handle_TakeOffer()", "size= %u", call.tuple->size() );

  call.Dump(SERVICE__CALLS);
    return new PyList;
}

PyResult LPService::Handle_ExchangeConcordLP( PyCallArgs& call )
{/**
            ret = sm.RemoteSvc('LPSvc').TakeOffer(self.cache.corpID, data.offerID)
            */
  sLog.Log( "LPService::Handle_ExchangeConcordLP()", "size= %u", call.tuple->size() );

  call.Dump(SERVICE__CALLS);
    return new PyList;
}

//17:09:54 L LPService::Handle_GetLPExchangeRates(): size= 0
PyResult LPService::Handle_GetLPExchangeRates( PyCallArgs& call )
{/**
            self.cache.exchangeRates = sm.RemoteSvc('LPSvc').GetLPExchangeRates()

05:42:04 [SvcCall] Service LPSvc: calling GetLPExchangeRates
05:42:04 [SvcCall]   Call Arguments:
05:42:04 [SvcCall]       Tuple: Empty
05:42:04 [SvcCall]   Call Named Arguments:
05:42:04 [SvcCall]     Argument 'machoVersion':
05:42:04 [SvcCall]         Integer field: 1
*/
    return new PyList;
}

//18:46:38 L CharMgrService::Handle_GetLPForCharacterCorp(): size= 1, 0=Integer(1000049)
PyResult LPService::Handle_GetLPForCharacterCorp( PyCallArgs& call )
{/**
                self.cache.lps = sm.RemoteSvc('LPSvc').GetLPForCharacterCorp(corpID)
         self.cache.concordLps = sm.RemoteSvc('LPSvc').GetLPForCharacterCorp(const.ownerCONCORD)

04:39:44 [SvcCall] Service LPSvc: calling GetLPForCharacterCorp
04:39:44 L CharMgrService::Handle_GetLPForCharacterCorp(): size= 1, 0=Integer(1000049)
04:39:44 [SvcCall]   Call Arguments:
04:39:44 [SvcCall]       Tuple: 1 elements
04:39:44 [SvcCall]         [ 0] Integer field: 1000049
04:39:44 [SvcCall]   Call Named Arguments:
04:39:44 [SvcCall]     Argument 'machoVersion':
04:39:44 [SvcCall]         Integer field: 1
*/
    return new PyInt( 0 );
}

PyResult LPService::Handle_GetLPsForCharacter( PyCallArgs& call )
{
    //no args
  sLog.Log( "LPService::Handle_GetLPsForCharacter()", "size= %u", call.tuple->size() );

  call.Dump(SERVICE__CALLS);
    return new PyList;
}

//18:55:57 L CharMgrService::Handle_GetAvailableOffersFromCorp(): size=2, 0=Integer(), 1=Boolean()
PyResult LPService::Handle_GetAvailableOffersFromCorp( PyCallArgs& call )
{/**
            self.cache.offers = sm.RemoteSvc('LPSvc').GetAvailableOffersFromCorp(self.cache.corpID, True)

05:42:04 [SvcCall] Service LPSvc: calling GetAvailableOffersFromCorp
05:42:04 [SvcCall]   Call Arguments:
05:42:04 [SvcCall]       Tuple: 2 elements
05:42:04 [SvcCall]         [ 0] Integer field: 1000049
05:42:04 [SvcCall]         [ 1] Boolean field: true
05:42:04 [SvcCall]   Call Named Arguments:
05:42:04 [SvcCall]     Argument 'machoVersion':
05:42:04 [SvcCall]         Integer field: 1
*/

    return new PyList;
}


