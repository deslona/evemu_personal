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
    Updates:        Allan
*/

#include "eve-server.h"

#include "EntityList.h"
#include "PyServiceCD.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"

PyCallable_Make_InnerDispatcher(AccountService)

AccountService::AccountService(PyServiceMgr *mgr)
: PyService(mgr, "account"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AccountService, GetCashBalance);
    PyCallable_REG_CALL(AccountService, GetEntryTypes);
    PyCallable_REG_CALL(AccountService, GetKeyMap);
    PyCallable_REG_CALL(AccountService, GiveCash);
    PyCallable_REG_CALL(AccountService, GiveCashFromCorpAccount);
    PyCallable_REG_CALL(AccountService, GetJournal);
    PyCallable_REG_CALL(AccountService, GetWalletDivisionsInfo);
}

AccountService::~AccountService() {
    delete m_dispatch;
}

PyResult AccountService::Handle_GetCashBalance(PyCallArgs &call) {
    //corrected, updated, optimized     -allan 26jan15

    bool isCorp = false;
    if (call.tuple->size() > 0) {
        if (call.tuple->GetItem(0)->IsBool())
            isCorp = call.tuple->GetItem(0)->AsBool()->value();
        else if (call.tuple->GetItem(0)->IsInt())
            isCorp = call.tuple->GetItem(0)->AsInt()->value();
    }

    double balance = 0;
    if (isCorp) {
        int16 accountKey = accountingKeyCash;
        if (call.tuple->size() > 1)
            accountKey = call.tuple->GetItem(1)->AsInt()->value();
        balance = m_db.GetCorpBalance( call.client->GetCorporationID(), accountKey );
    } else {
        if (call.byname.find("accountKey") != call.byname.end())
            balance = call.client->GetChar().get()->aurBalance();
        else
            balance = call.client->GetChar().get()->balance();
    }

    return new PyFloat(balance);
}

PyResult AccountService::Handle_GetEntryTypes(PyCallArgs &call) {
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetEntryTypes");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetEntryTypes();
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

PyResult AccountService::Handle_GetKeyMap(PyCallArgs &call) {
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetKeyMap");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetKeyMap();
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

//give cash takes (ownerID, retval['qty'], retval['reason'][:40])
PyResult AccountService::Handle_GiveCash(PyCallArgs &call) {
    Call_GiveCash args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    if(args.amount == 0) return NULL;

    if(args.amount < 0 || args.amount > call.client->GetBalance()) {
        _log(CLIENT__ERROR, "%s: Invalid amount in GiveCash(): %.2f", call.client->GetName(), args.amount);
        call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
        return NULL;
    }

    if (IsCorp(args.toID))
        return GiveCashToCorp(call.client, args.toID, args.amount, args.reason.c_str(), refPlayerDonation);
    else{
        Client *pToClient = m_manager->entity_list.FindCharacter(args.toID);
        return GiveCashToChar(call.client, pToClient, args.amount, args.reason.c_str(), refPlayerDonation);
    }
}

PyTuple * AccountService::GiveCashToCorp(Client * const client, uint32 corpID, double amount, const char *reason, JournalRefType refTypeID) {
    if(!client->AddBalance(-amount)) {
        _log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for donation to %u",
            client->GetName(),
            amount,
            client->GetCharacterID(),
            corpID );
        client->SendErrorMsg("Failed to transfer money from your account.");
        return NULL;
    }
    if(!m_db.AddBalanceToCorp(corpID, amount)) {
        _log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u",
            client->GetName(),
            amount,
            corpID,
            client->GetCharacterID());
        client->SendErrorMsg("Failed to transfer money to your destination.");

        //try to refund the money..
        client->AddBalance(amount);

        return NULL;
    }

    uint16 accountKey = accountingKeyCash;  //FIXME  get proper corp wallet division
    double cnb = m_db.GetCorpBalance(corpID, accountKey);

    // Send notification about the cash change
    OnAccountChange oac;
    oac.accountKey = "cash";
    oac.balance = cnb;
    oac.ownerid = corpID;
    PyTuple * answer = oac.Encode();

    MulticastTarget mct;
    mct.corporations.insert(corpID);
    m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

    //record the transactions in the wallet.
    if(!m_db.GiveCash(
        client->GetCharacterID(),
        refTypeID,
        client->GetCharacterID(),
        corpID,
        "unknown",
        client->GetAccountID(),
        accountingKeyCash,
        -amount,
        client->GetBalance(),
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
        //no good reason to return... the money has actually been moved.
    }

    if(!m_db.GiveCash(
        corpID,
        refTypeID,
        client->GetCharacterID(),
        corpID,
        "unknown",
        corpID,
        accountingKeyCash,      //TODO set proper wallet division here
        amount,
        cnb,
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on receiving side");
        //no good reason to return... the money has actually been moved.
    }

    //send back the new balance
    PyTuple *ans= new PyTuple(2);
    ans->items[0]=new PyFloat(cnb);//new balance
    ans->items[1]=new PyFloat(cnb);//new balance, not an error need to send it 2 times

    return ans;
}

PyTuple * AccountService::GiveCashToChar(Client * const from, Client * const to, double amount, const char *reason, JournalRefType refTypeID) {
    if(!from->AddBalance(-amount)) {
        _log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for donation to %u",
            from->GetName(),
            amount,
            from->GetCharacterID(),
            to->GetCharacterID() );
        from->SendErrorMsg("Failed to transfer money from your account.");
        return NULL;
    }
    if(!to->AddBalance(amount)) {
        _log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u",
            from->GetName(),
            amount,
            to->GetCharacterID(),
            from->GetCharacterID());
        from->SendErrorMsg("Failed to transfer money to your destination.");

        //try to refund the money..
        from->AddBalance(amount);

        return NULL;
    }

    //record the transactions in the wallet.
    //first on the send side.
    if(!m_db.GiveCash(
        from->GetCharacterID(),
        refTypeID,
        from->GetCharacterID(),
        to->GetCharacterID(),
        "unknown",
        from->GetAccountID(),
        accountingKeyCash,
        -amount,
        from->GetBalance(),
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on receiving side");
        //no good reason to return... the money has actually been moved.
    }

    //then on the receive side.
    if(!m_db.GiveCash(
        to->GetCharacterID(),
        refTypeID,
        to->GetCharacterID(),
        from->GetCharacterID(),
        "unknown",
        to->GetAccountID(),
        accountingKeyCash,
        amount,
        to->GetBalance(),
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
        //no good reason to return... the money has actually been moved.
    }


    //send back the new balance
    PyTuple *ans= new PyTuple(2);
    ans->items[0]=new PyFloat(from->GetBalance());//new balance
    ans->items[1]=new PyFloat(from->GetBalance());//new balance, not an error need to send it 2 times

    return ans;
}

PyResult AccountService::Handle_GetJournal(PyCallArgs &call) {
/*
 *   02:46:06 L AccountService::Handle_GetJournal(): size= 6, 0=Integer, 1=Long, 2=None, 3=Boolean, 4=None, 5=Integer
 *   keyvalues = sm.GetService('account').GetJournal(accountKey, fromDate, entryTypeID, corpAccount, transactionID, rev)
 */

    Call_GetJournal args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    bool ca = false;
    if( args.corpAccount->IsBool() )
        ca = args.corpAccount->AsBool()->value();
    else if( args.corpAccount->IsInt() )
        ca = ( args.corpAccount->AsInt()->value() != 0 );
    else
    {
        // problem
        _log( SERVICE__WARNING, "%s: Unsupported value for corpAccount", GetName() );

        return NULL;
    }

    return m_db.GetJournal(
        ( ca ? call.client->GetCorporationID() : call.client->GetCharacterID() ),
        args.entryTypeID,
        args.accountKey,
        args.fromDate
    );
}

//givecash takes (ownerID, retval['qty'], retval['reason'][:40])
//GiveCashFromCorpAccount(ownerID, retval['qty'], retval['reason'][:40])
// notify OnAccountChange:
//         accountKey: 'cash', ownerID: charID or corpID, new balance

PyResult AccountService::Handle_GiveCashFromCorpAccount(PyCallArgs &call) { //TODO:  fix corpAccountKey
    /*[00m18:58:35 [SvcCall] Service account: calling GiveCashFromCorpAccount
     * 18:58:35 [PacketError] Decode Call_GiveCorpCash failed: tuple0 is the wrong size: expected 4, but got 3
     * 18:58:35 [ClientError] Handle_GiveCashFromCorpAccount(/usr/local/src/eve/evemu_personal/src/eve-server/account/AccountService.cpp:347): Invalid arguments
     */

    sLog.Log( "AccountService::Handle_GiveCashFromCorpAccount()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALLS);

    Call_GiveCorpCash args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
        return NULL;
    }

    if(args.amount == 0) return NULL;

    uint16 accountKey = accountingKeyCash;  //FIXME  get proper corp wallet division

    if(args.amount < 0 || args.amount > m_db.GetCorpBalance(call.client->GetCorporationID(), accountKey)) {
        _log(CLIENT__ERROR, "%s: Invalid amount in GiveCashFromCorpAccount(): %.2f", call.client->GetName(), args.amount);
        call.client->SendErrorMsg("Invalid amount '%.2f'", args.amount);
        return NULL;
    }

    SystemManager *system = call.client->System();
    if(system == NULL) {
        codelog(CLIENT__ERROR, "%s: bad system", call.client->GetName());
        return NULL;
    }

    //NOTE: this will need work once we reorganize the entity list...
    Client *other = m_manager->entity_list.FindCharacter(args.toID);
    if(other == NULL) {
        _log(CLIENT__ERROR, "%s: Failed to find character %u", call.client->GetName(), args.toID);
        call.client->SendErrorMsg("Unable to find the target");
        return NULL;
    }


    return WithdrawCashToChar(call.client, other, args.amount, args.reason.c_str(), refCorporationAccountWithdrawal);
}

PyTuple * AccountService::WithdrawCashToChar(Client * const client, Client * const other, double amount, const char *reason, JournalRefType refTypeID) {
    // remove money from the corp
    uint32 corpID = client->GetCorporationID();
    if (!m_db.AddBalanceToCorp(corpID, double(-amount))) {
        _log(CLIENT__ERROR, "%s: Failed to remove %.2f ISK from %u for withdrawal to %u",
            client->GetName(),
            amount,
            corpID,
            other->GetCharacterID() );
        client->SendErrorMsg("Failed to transfer money from your account.");
        return NULL;
    }

    uint16 accountKey = accountingKeyCash;  //FIXME  get proper corp wallet division

    double ncb = m_db.GetCorpBalance(corpID, accountKey);

    // Send notification about the cash change
    OnAccountChange oac;
    oac.accountKey = "cash";
    oac.balance = ncb;
    oac.ownerid = corpID;
    PyTuple * answer = oac.Encode();

    MulticastTarget mct;
    mct.corporations.insert(corpID);
    m_manager->entity_list.Multicast("OnAccountChange", "*corpid&corpAccountKey", &answer, mct);

    if(!other->AddBalance(amount)) {
        _log(CLIENT__ERROR, "%s: Failed to add %.2f ISK to %u for donation from %u",
            client->GetName(),
            amount,
            corpID,
            client->GetCharacterID());
        client->SendErrorMsg("Failed to transfer money to your destination.");

        //try to refund the money..
        m_db.AddBalanceToCorp(corpID, double(amount));
        // if we're here, we have a more serious problem than
        // corp's balance not being displayed properly, so i won't bother with it

        return NULL;
    }

    //record the transactions in the wallet.
    //first on the send side.
    char argID[15];
    snprintf(argID, 14, "%u", client->GetCharacterID());
    if(!m_db.GiveCash(
        corpID,
        refTypeID,
        corpID,
        other->GetCharacterID(),
        argID,
        corpID,
        accountingKeyCash,      //TODO set proper wallet division here
        -amount,
        ncb,
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on receiving side");
        //no good reason to return... the money has actually been moved.
    }

    //then on the receive side.
    if(!m_db.GiveCash(
        other->GetCharacterID(),
        refTypeID,
        corpID,
        other->GetCharacterID(),
        argID,
        other->GetAccountID(),
        accountingKeyCash,
        amount,
        other->GetBalance(),
        reason
        )
    ) {
        codelog(CLIENT__ERROR, "Failed to record transaction on sending side");
        //no good reason to return... the money has actually been moved.
    }


    //send back the new balance
    PyTuple *ans= new PyTuple(2);

    // maybe this needs it this way, just like the other ones...
    // i'm not sure, but it works for sure
    ans->items[0]=new PyFloat(ncb);
    ans->items[1]=new PyFloat(ncb);

    return ans;
}

PyResult AccountService::Handle_GetWalletDivisionsInfo(PyCallArgs &call) {
    return (m_db.GetWalletDivisionsInfo(call.client->GetCorporationID()));
}
