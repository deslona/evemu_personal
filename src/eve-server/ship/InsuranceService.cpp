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
Author: Zhur
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "ship/InsuranceService.h"

/* TODO:
* - handle ship-destroyed event (remove insurance, pay out value, etc...)
* - use history market value from marketProxy!
*/

class InsuranceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(InsuranceBound)

    InsuranceBound(PyServiceMgr *mgr, ShipDB* db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this)) {
        _SetCallDispatcher(m_dispatch);

        PyCallable_REG_CALL(InsuranceBound, GetContracts);
        PyCallable_REG_CALL(InsuranceBound, GetInsurancePrice);

        m_strBoundObjectName = "InsuranceBound";
    }

    virtual ~InsuranceBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetContracts);
    PyCallable_DECL_CALL(GetInsurancePrice);

protected:
    ShipDB* m_db;
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(InsuranceService)

InsuranceService::InsuranceService(PyServiceMgr *mgr)
: PyService(mgr, "insuranceSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(InsuranceService, InsureShip);
    PyCallable_REG_CALL(InsuranceService, GetInsurancePrice);
    PyCallable_REG_CALL(InsuranceService, GetContractForShip);
	//SetSessionCheck?
}

InsuranceService::~InsuranceService() {
    delete m_dispatch;
}

PyBoundObject* InsuranceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "InsuranceService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, " " );

    return new InsuranceBound( m_manager, &m_db );
}

//04:23:28 L InsuranceBound::Handle_GetInsurancePrice(): size= 1, 0=Integer [this passes typeID]
PyResult InsuranceBound::Handle_GetInsurancePrice( PyCallArgs& call ) {
    /*
    Call Arguments:
        Tuple: 1 elements
            [ 0] Integer field: 606     // typeID
    */
    sLog.Log("InsuranceBound", "Handle_GetInsurancePrice() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    uint32 typeID = call.tuple->GetItem(0)->AsInt()->value();

    const ItemType *type = m_manager->item_factory.GetType(typeID);
    if(!type)
        return new PyNone;

    return new PyFloat(type->basePrice());
}

PyResult InsuranceBound::Handle_GetContracts( PyCallArgs& call ) {
  /**   look into this.....
            contracts = self.GetInsuranceMgr().GetContracts()
            contracts = self.GetInsuranceMgr().GetContracts(1)  <-- corp contracts?
            contracts = sm.RemoteSvc('insuranceSvc').GetContracts()
            */
    return m_db->GetInsuranceContractsByOwnerID(call.client->GetCharacterID());
}



PyResult InsuranceService::Handle_GetInsurancePrice( PyCallArgs& call ) {
    sLog.Log("InsuranceService", "Handle_GetInsurancePrice() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);
    uint32 typeID = call.tuple->GetItem(0)->AsInt()->value();

    const ItemType *type = m_manager->item_factory.GetType(typeID);
    if(!type)
        return new PyNone;

    return new PyFloat(type->basePrice());
}

PyResult InsuranceService::Handle_GetContractForShip( PyCallArgs& call ) {
    /*
    Call Arguments:
        Tuple: 1 elements
            [ 0] Integer field: 140000078   // shipID
    */

    return m_db.GetInsuranceInfoByShipID(call.tuple->GetItem(0)->AsInt()->value());
}

PyResult InsuranceService::Handle_InsureShip( PyCallArgs& call ) {
    /*
15:05:24 [SvcCall]   Call Arguments:
15:05:24 [SvcCall]       Tuple: 3 elements
15:05:24 [SvcCall]         [ 0] Integer field: 140000227
15:05:24 [SvcCall]         [ 1] Real field: 1558.900000
15:05:24 [SvcCall]         [ 2] Integer field: 0
15:05:24 L Client: Info Modal to Lee Domani:
15:05:24 [ClientMessage] You cannot insure Rookie ships.

15:11:24 [SvcCall]   Call Arguments:
15:11:24 [SvcCall]       Tuple: 3 elements
15:11:24 [SvcCall]         [ 0] Integer field: 140000307
15:11:24 [SvcCall]         [ 1] Real field: 1370977.900000
15:11:24 [SvcCall]         [ 2] Integer field: 0

    */
    sLog.Log("InsuranceService", "Handle_InsureShip() size=%u", call.tuple->size() );
  call.Dump(SERVICE__CALLS);
  call.client->SendInfoModalMsg("The Insurance System is not Functional at this time.");
  return new PyNone;

    /* INSURANCE FRACTION TABLE:
            Label    Fraction  Pay
            -------- --------- ------
            Platin   1.0       0.3
            Gold     0.9       0.25
            Silver   0.8       0.2
            Bronze   0.7       0.15
            Standard 0.6       0.1
            Basic    0.5       0.05
    */

    uint32 shipID = call.tuple->GetItem(0)->AsInt()->value();
    /** added check for groupID 237 (rookie ship - items 588, 596, 601, 606) as they cant be insured. */
    DBQueryResult result;

    if (!sDatabase.RunQuery(result, " SELECT typeID FROM entity WHERE itemID = %u ", shipID)) {
        sLog.Error( "InsuranceBound::Handle_InsureShip()", "Error in query: %s", result.error.c_str() );
        return new PyNone;
    }

    DBResultRow row;
    if(result.GetRow(row)) {
        uint32 shipType = row.GetUInt( 0 );   //EVEDB::invGroups::Rookie_ship
        if(( shipType == 588) || ( shipType == 596 ) || ( shipType == 601 ) || ( shipType == 606 )) {
            call.client->SendInfoModalMsg("You cannot insure Rookie ships.");
            return new PyNone;
        }
    } else {
        call.client->SendErrorMsg("Error Searching shipType in DB.");
        return new PyNone;
    }/** end rookie ship check  */

    double payment = call.tuple->GetItem(1)->AsFloat()->value();
    uint32 unknown = call.tuple->GetItem(2)->AsInt()->value();

    ShipRef ship = m_manager->item_factory.GetShip( shipID );

    // calculate the fraction value
    double shipValue = ship->type().basePrice() *100;
    double paymentFraction = payment/shipValue;
    double fraction = 0.0;
    if(paymentFraction == 0.05)
        fraction = 0.5;
    else if(paymentFraction == 0.1)
        fraction = 0.6;
    else if(paymentFraction == 0.15)
        fraction = 0.7;
    else if(paymentFraction == 0.2)
        fraction = 0.8;
    else if(paymentFraction == 0.25)
        fraction = 0.9;
    else if(paymentFraction == 0.3)
        fraction = 1.0;

    if(fraction == 0.0)
        return new PyNone;

    //  let the player pay for the insurance
    if(!call.client->AddBalance(-payment)) {
        _log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for payment of insurance",
            call.client->GetName(), payment, call.client->GetCharacterID() );
        call.client->SendErrorMsg("Failed to transfer money from your account.");
        return new PyNone;
    }

    // delete old insurance, if any
    m_db.DeleteInsuranceByShipID(shipID);
/**
  *  this is commented out because i cannot get the insurance contract sent back to the client correctly, so it's not showing
  *  in the insurance window.  the return usually crashes the server when using DBResultToRowset
*/
    // add new insurance
    bool add = m_db.InsertInsuranceByShipID(shipID, fraction);
    if(add) return new PyNone;
    else {
        call.client->SendErrorMsg("Failed to install new insurance contract.");
        return new PyNone;
    }

    // TODO:  send mail detailing insurance coverage and length of coverage

}
