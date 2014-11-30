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
#include "system/SystemEntity.h"
#include "system/SystemManager.h"

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

        PyCallable_REG_CALL(InsuranceBound, InsureShip);
        PyCallable_REG_CALL(InsuranceBound, UnInsureShip);
        PyCallable_REG_CALL(InsuranceBound, GetContracts);
        PyCallable_REG_CALL(InsuranceBound, GetInsurancePrice);

        m_strBoundObjectName = "InsuranceBound";
    }

    virtual ~InsuranceBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(InsureShip);
    PyCallable_DECL_CALL(UnInsureShip);
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

    PyCallable_REG_CALL(InsuranceService, GetContractForShip);
	//SetSessionCheck?
}

InsuranceService::~InsuranceService() {
    delete m_dispatch;
}

PyBoundObject* InsuranceService::_CreateBoundObject( Client* c, const PyRep* bind_args ) {
    return new InsuranceBound( m_manager, &m_db );
}

PyResult InsuranceBound::Handle_GetInsurancePrice( PyCallArgs& call ) {
    uint32 typeID = call.tuple->GetItem(0)->AsInt()->value();
    const ItemType *type = m_manager->item_factory.GetType(typeID);
    if(!type)
        return new PyNone;
    return new PyFloat(type->basePrice());
}

PyResult InsuranceBound::Handle_GetContracts( PyCallArgs& call ) {
    if (call.tuple->size() > 0) {
        Call_IntBoolArg args;
        if(!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Failed to decode arguments");
            return NULL;
        }

        return (m_db->GetInsuranceByOwnerID(call.client->GetCorporationID()));
    }

    return (m_db->GetInsuranceByOwnerID(call.client->GetCharacterID()));
}

PyResult InsuranceService::Handle_GetContractForShip( PyCallArgs& call ) {
    return (m_db.GetInsuranceByShipID(call.tuple->GetItem(0)->AsInt()->value()));
}

PyResult InsuranceBound::Handle_InsureShip( PyCallArgs& call ) {
	Call_InsureShip args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return NULL;
    }

    /* added check for groupID 237 (rookie ship - items 588, 596, 601, 606) as they cant be insured. */
    if (call.client->System() == NULL) {
        call.client->SendInfoModalMsg("Your system manager is null.");
        sLog.Error("InsuranceService::Handle_InsureShip", "System() == NULL for %s (%u))", call.client->GetName(), call.client->GetCharacterID());
        return new PyNone;
    }

    SystemEntity *pShip = call.client->System()->get( args.shipID );
	if(pShip->Item()->groupID() == EVEDB::invGroups::Rookie_ship) {
        call.client->SendInfoModalMsg("You cannot insure Rookie ships.");
        return new PyNone;
    }/* end rookie ship check  */

    // calculate the fraction value
    const ItemType *type = m_manager->item_factory.GetType(pShip->Item()->typeID());
    double paymentFraction = (args.amount / (type->basePrice()));
    float fraction = 0.0f;
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
    if (paymentFraction == 0.05) fraction = 0.5f;
    else if (paymentFraction == 0.1) fraction = 0.6f;
    else if (paymentFraction == 0.15) fraction = 0.7f;
    else if (paymentFraction == 0.2) fraction = 0.8f;
    else if (paymentFraction == 0.25) fraction = 0.9f;
    else if (paymentFraction == 0.3) fraction = 1.0f;

    if (fraction == 0.0)
        return new PyNone;

    // delete old insurance, if any
    if (call.byname.find("voidOld")->second->AsInt()->value());
        m_db->DeleteInsuranceByShipID(args.shipID);
/*  fix insurance return in shipDB.cpp:97  and 115
 * dbresulttorow*  does NOT work
    // add new insurance
    if (m_db->InsertInsuranceByShipID(args.shipID, fraction, args.isCorp)) {
        //  it sucessfully added, now, have the player pay for the insurance
        if (!call.client->AddBalance(- args.amount)) {
            _log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for insurance premium.",
            call.client->GetName(), args.amount, call.client->GetCharacterID() );
            call.client->SendErrorMsg("Failed to transfer money from your account.");
            m_db->DeleteInsuranceByShipID(args.shipID);
            return new PyNone;
			// at this point, the previous insurance was deleted, and the new insurance
			//   has been deleted.  should there be a check for keeping the old insurance incase
			//   the payment fails?
        }
	} else {  */
        call.client->SendErrorMsg("Failed to install new insurance contract.");
        return new PyNone;
    //}

    // TODO:  send mail detailing insurance coverage and length of coverage
    const char *subject = "New Ship Insurance";
    const char *body = "Dear valued customer,<BR>" \
                    "Congratulations on the insurance on your ship. A very wise choice indeed.<br>" \
                    "This letter is to confirm that we have issued an insurance contract for your ship, %s at a level of %f%." \
                    /*This contract will expire at 2006.11.15 14:14:27, after 12 weeks.*/
                    "Best,<BR>" \
                    "The Secure Commerce Commission,<BR>" \
                    "Reference ID: %u";
    std::string name = static_cast<std::string>(pShip->GetName());
    call.client->SelfEveMail(subject, body, name.c_str(), fraction, args.shipID);

    return (m_db->GetInsuranceByShipID(args.shipID));
}

PyResult InsuranceBound::Handle_UnInsureShip( PyCallArgs& call ) {
    uint32 shipID = call.tuple->GetItem(0)->AsInt()->value();
    // delete old insurance
    m_db->DeleteInsuranceByShipID(shipID);
    return new PyNone;
}