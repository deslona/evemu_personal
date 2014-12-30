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

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "ship/BeyonceService.h"
#include "ship/DestinyManager.h"
#include "system/BookmarkService.h"
#include "system/SystemEntities.h"
#include "system/SystemManager.h"

class BeyonceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(BeyonceBound)

    BeyonceBound(PyServiceMgr *mgr, Client *c)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "BeyonceBound";

        PyCallable_REG_CALL(BeyonceBound, CmdFollowBall);
        PyCallable_REG_CALL(BeyonceBound, CmdOrbit);
        PyCallable_REG_CALL(BeyonceBound, CmdAlignTo);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoDirection);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoBookmark);
        PyCallable_REG_CALL(BeyonceBound, CmdSetSpeedFraction);
        PyCallable_REG_CALL(BeyonceBound, CmdStop);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuff);
        PyCallable_REG_CALL(BeyonceBound, CmdDock);
        PyCallable_REG_CALL(BeyonceBound, CmdStargateJump);
        PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuffAutopilot);
        PyCallable_REG_CALL(BeyonceBound, CmdAbandonLoot);

        if(c->Destiny() != NULL)
		    if(c->Bubble() != NULL)		//  client in station...no bubble
                c->Destiny()->SendSetState(c->Bubble());
    }
    virtual ~BeyonceBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(CmdFollowBall);
    PyCallable_DECL_CALL(CmdOrbit);
    PyCallable_DECL_CALL(CmdAlignTo);
    PyCallable_DECL_CALL(CmdGotoDirection);
    PyCallable_DECL_CALL(CmdGotoBookmark);
    PyCallable_DECL_CALL(CmdSetSpeedFraction);
    PyCallable_DECL_CALL(CmdStop);
    PyCallable_DECL_CALL(CmdWarpToStuff);
    PyCallable_DECL_CALL(CmdDock);
    PyCallable_DECL_CALL(CmdStargateJump);
    PyCallable_DECL_CALL(UpdateStateRequest);
    PyCallable_DECL_CALL(CmdWarpToStuffAutopilot);
    PyCallable_DECL_CALL(CmdAbandonLoot);

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(BeyonceService)

BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(BeyonceService, )
    PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
    delete m_dispatch;
}


PyBoundObject* BeyonceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "BeyonceService bind request for:" );
    bind_args->Dump( CLIENT__MESSAGE, "    " );

    return new BeyonceBound( m_manager, c );
}


PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetFormations");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        PyRep *res = m_db.GetFormations();
        if(res == NULL) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            res = new PyNone();
        }

        m_manager->cache_service->GiveCache(method_id, &res);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    //return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
    return new PyTuple(0);
}

/*
PyResult BeyonceService::Handle_(PyCallArgs &call) {
    PyRep *result = NULL;

    return result;
}
*/

PyResult BeyonceBound::Handle_CmdFollowBall(PyCallArgs &call) {
    Call_FollowBall args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return NULL;
    }

    double distance;
    if( args.distance->IsInt() )
        distance = args.distance->AsInt()->value();
    else if( args.distance->IsFloat() )
        distance = args.distance->AsFloat()->value();
    else
    {
        codelog(CLIENT__ERROR, "%s: Invalid type %s for distance argument received.", call.client->GetName(), args.distance->TypeString());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP) {
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}
    SystemManager *system = call.client->System();
    if(system == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return NULL;
    }
    SystemEntity *entity = system->get(args.ballID);
    if(entity == NULL) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.ballID);
        return NULL;
    }

    destiny->Follow(entity, distance);

    return NULL;
}

PyResult BeyonceBound::Handle_CmdSetSpeedFraction(PyCallArgs &call) {
    sLog.Log( "BeyonceBound", "Handle_CmdSetSpeedFraction" );
    call.Dump(SERVICE__CALLS);

    Call_SingleRealArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    destiny->SetSpeedFraction(arg.arg);

    return NULL;
}

/* AlignTo
 * This will look up the entityID to get it's position in space, then call
 * AlignTo to have it respond with gotopoint.
 * @author Xanarox
*/
PyResult BeyonceBound::Handle_CmdAlignTo(PyCallArgs &call) {

    CallAlignTo arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    SystemManager *system = call.client->System();
    if(system == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return NULL;
    }

    SystemEntity *entity = system->get(arg.entityID);
    if(entity == NULL) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
        return NULL;
    }

    //const GPoint &position = entity->GetPosition();
    destiny->AlignTo( entity, true );

    return NULL;
}

PyResult BeyonceBound::Handle_CmdGotoDirection(PyCallArgs &call) {
  /**
04:45:32 L BeyonceBound: Handle_CmdGotoDirection
04:45:32 [SvcCall]   Call Arguments:
04:45:32 [SvcCall]       Tuple: 3 elements
04:45:32 [SvcCall]         [ 0] Real field: -0.043847
04:45:32 [SvcCall]         [ 1] Real field: 0.860934
04:45:32 [SvcCall]         [ 2] Real field: 0.506824

    sLog.Log( "BeyonceBound", "Handle_CmdGotoDirection" );
  call.Dump(SERVICE__CALLS);
*/
    Call_PointArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    destiny->GotoDirection( GPoint( arg.x, arg.y, arg.z ) );

    return NULL;
}

PyResult BeyonceBound::Handle_CmdGotoBookmark(PyCallArgs &call) {

    if( !(call.tuple->GetItem( 0 )->IsInt()) )
    {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: Invalid type %s for bookmarkID received.", call.client->GetName(), call.tuple->GetItem( 0 )->TypeString() );
        return NULL;
    }
    uint32 bookmarkID = call.tuple->GetItem( 0 )->AsInt()->value();

    DestinyManager *pDestiny = call.client->Destiny();
    if( pDestiny == NULL )
    {
        sLog.Error( "%s: Client has no destiny manager!", call.client->GetName() );
		return NULL;
	} else if(pDestiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

	double x = 0.0, y = 0.0, z = 0.0;
    uint32 itemID = 0, typeID = 0;
    GPoint bookmarkPosition  = (NULL_ORIGIN);

    BookmarkService *pBMSvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));

    if( pBMSvc == NULL )
    {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
        return NULL;
    }
    else
    {
        pBMSvc->LookupBookmark(call.client->GetCharacterID(), bookmarkID, itemID, typeID, x, y, z);

        if( typeID == 5 )
        {
            // Bookmark type is coordinate, so use these directly from the bookmark system call:
            bookmarkPosition.x = x;     // From bookmark x
            bookmarkPosition.y = y;     // From bookmark y
            bookmarkPosition.z = z;     // From bookmark z

            pDestiny->GotoDirection( bookmarkPosition );
        }
        else
        {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            SystemManager *sm = call.client->System();
            if(sm == NULL) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: no system manager found", call.client->GetName() );
                return NULL;
            }
            SystemEntity *se = sm->get( itemID );
            if(se ==  NULL) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
                return NULL;
            }

            pDestiny->GotoDirection( se->GetPosition() );
        }
    }

    return NULL;
}

PyResult BeyonceBound::Handle_CmdOrbit(PyCallArgs &call) {
  /*
            bp.CmdOrbit(id, range)
            */
    sLog.Log( "BeyonceBound", "Handle_CmdOrbit" );
  call.Dump(SERVICE__CALLS);
    Call_Orbit arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    double range;
	if( arg.range->IsInt() )
		range = arg.range->AsInt()->value();
	else if( arg.range->IsFloat() )
		range = arg.range->AsFloat()->value();
    else
    {
		codelog(CLIENT__ERROR, "%s: Invalid type %s for range argument received.", call.client->GetName(), arg.range->TypeString());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    SystemManager *system = call.client->System();
    if(system == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return NULL;
    }
    SystemEntity *entity = system->get(arg.entityID);
    if(entity == NULL) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), arg.entityID);
        return NULL;
    }

    destiny->Orbit(entity, range);
    return NULL;
}

PyResult BeyonceBound::Handle_CmdWarpToStuff(PyCallArgs &call) {
  /**
21:05:18 L BeyonceBound: Handle_CmdWarpToStuff
item, id
epinstance, instanceid
scan, resultid, minrange, fleet(bool)
[warpfleet] item, id, minrange, fleet=1
[warptomember] char, charid, minrange
[warpfleettomember] char, charid, minrange, fleet=1
bookmark, bmid, minrange, fleet(bool)
[toItem] item, id, minrange
tutorial, none
bookmark, bmid
[hiddendungeon] epinstance, instanceid
[launchpickup] launch, launchid

*/
        //sLog.Warning( "BeyonceBound", "Handle_CmdWarpToStuff" );
    CallWarpToStuff arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You are already warping");
		return NULL;
	}

    if( arg.type == "item" ) {
		// This section handles Warping to any object in the Overview

		// get the warp-to distance specified by the client
		int32 distance = 0;
		std::map<std::string, PyRep *>::const_iterator res = call.byname.find("minRange");
		if(res == call.byname.end()) {
			distance = 0;
		} else if(!res->second->IsInt() && !res->second->IsFloat()) {
			codelog(CLIENT__ERROR, "%s: range of invalid type %s, expected Integer or Real; using 15 km.", call.client->GetName(), res->second->TypeString());
			distance = 15000;
		} else {
			distance =
			res->second->IsInt()
			? res->second->AsInt()->value()
			: res->second->AsFloat()->value();
		}

		SystemManager *sm = call.client->System();
		if(sm == NULL) {
			codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
			return NULL;
		}
		SystemEntity *se = sm->get(arg.ID);
		if(se ==  NULL) {
			codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), arg.ID);
			return NULL;
		}


        double distanceFromBodyOrigin = 0.0;
        double distanceFromSystemOrigin = 0.0;
		GPoint warpToPoint(se->GetPosition());  // Make a warp-in point variable
        float warpPointAdj = -1.0f;
		if( IsStaticMapItem(se->GetID()) ) {
			switch( ((SimpleSystemEntity *)(se))->data.groupID ) {
				case EVEDB::invGroups::Sun:
				case EVEDB::invGroups::Planet:
				case EVEDB::invGroups::Moon:
                {
                    // Calculate final distance out from origin of celestial body along common warp-to vector:
                    distanceFromBodyOrigin = se->GetRadius();            // Add celestial body's radius
                    distanceFromBodyOrigin += 20000000;                    // Add 20,000km along common vector from celestial body origin to ensure
                    // client camera rotation about ship does not take camera inside the celestial body's wireframe

                    // Calculate final warp-to point along common vector from celestial body's origin and add randomized position adjustment for multiple ships coming out of warp to not bump
                    GPoint celestialOrigin(se->GetPosition());                            // Make a celestial body origin point variable
                    GVector vectorFromOrigin(celestialOrigin, NULL_ORIGIN);                    // Make a celestial body TO system origin origin vector variable
                    if( vectorFromOrigin.length() == 0 )
                    {
                        // This is the special case where we are warping to the Star, so we have to construct
                        // a vector from the star's center (0,0,0) to the warp-in point using the distanceFromBodyOrigin
                        // calculated earlier:
                        vectorFromOrigin = GVector( celestialOrigin, call.client->GetPosition() );
                        vectorFromOrigin.normalize();
                        vectorFromOrigin *= distanceFromBodyOrigin;
                    }
                    GVector vectorToWarpPoint(vectorFromOrigin);                        // Make a vector to the Warp-In point
                    distanceFromSystemOrigin = vectorFromOrigin.length();                // Calculate distance from system origin to celestial body origin

                    // Calculate warp-in point to provide different juxtapositioning of celestial body to the solar system origin, i.e, the sun
                    // This also provides a common warp-in point for the sun itself, which is the first case in this if-else if-else clause:
                    if( distanceFromSystemOrigin < (5.0 * ONE_AU_IN_METERS) )
                    {
                        // For all celestial bodies with orbit radius of under 5AU, including the sun,
                        GVector rotationVector( 1.0, 1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
                    }
                    else if( distanceFromSystemOrigin < (15.0 * ONE_AU_IN_METERS) )
                    {
                        // For all celestial bodies with orbit radius of under 15AU but more than 5AU,
                        GVector rotationVector( -1.0, -1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
                    }
                    else if( distanceFromSystemOrigin < (25.0 * ONE_AU_IN_METERS) )
                    {
                        // For all celestial bodies with orbit radius of under 25AU but more than 15AU,
                        GVector rotationVector( 1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
                    }
                    else if( distanceFromSystemOrigin < (35.0 * ONE_AU_IN_METERS) )
                    {
                        // For all celestial bodies with orbit radius of under 35AU but more than 25AU,
                        GVector rotationVector( -1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
                    }
                    else
                    {
                        // For all celestial bodies with orbit radius of more than 35AU,
                        GVector rotationVector( -1.0, 1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;
                    }

                    // Randomize warp-in point:     why?
                    //warpToPoint.MakeRandomPointOnSphereLayer(1000.0,(1000.0+call.client->GetRadius()));

                    //When you "warp to within 0m" you will get to a distance that is approximately 0.65 * the radius of the star.
                    //warpPointAdj = ((se->GetRadius() *2) * 0.65);
					break;
				}
				default:
                    warpPointAdj = se->GetRadius();
					break;
			}
        } else
            warpPointAdj = se->GetRadius();

        distance += call.client->GetRadius();

        /* client stops warp at targetpoint - stopdistance along common line between the two.
         *  the server will need to be told what and how to match the client.
         *  i am doing that here.
         * set targetpoint = point of object minus distance along common vector
         */
        GVector vectorFromOrigin( warpToPoint, call.client->GetPosition() );
        vectorFromOrigin.normalize();   //we now have a direction
        GPoint stopPoint = vectorFromOrigin * -warpPointAdj;
        warpToPoint -= stopPoint;

        destiny->WarpTo(warpToPoint, distance);
	} else if( arg.type == "bookmark" ) {
        //  bookmark, bmid, minrange, fleet(bool)
        // This section handles Warping to any Bookmark
        int32 distance = 0;
        double x = 0.0, y = 0.0, z = 0.0;
        uint32 itemID = 0, typeID = 0;
        GPoint bookmarkPosition  = (NULL_ORIGIN);

        BookmarkService *bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));

        if( bkSrvc == NULL ) {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
            return NULL;
        } else {
            bkSrvc->LookupBookmark(call.client->GetCharacterID(), arg.ID, itemID, typeID, x, y, z);

            // Calculate the warp-to distance specified by the client and add this to the final warp-to distance
            std::map<std::string, PyRep *>::const_iterator res = call.byname.find("minRange");
            distance +=
                res->second->IsInt()
                ? res->second->AsInt()->value()
                : res->second->AsFloat()->value();

            if( typeID == 5 ) {
                // Bookmark type is coordinate, so use these directly from the bookmark system call:
                bookmarkPosition.x = x;     // From bookmark x
                bookmarkPosition.y = y;     // From bookmark y
                bookmarkPosition.z = z;     // From bookmark z

                destiny->WarpTo(bookmarkPosition, distance);
            } else {
                DBQueryResult result;
                   DBResultRow row;
                uint32 groupID = 0;

                // Query database 'invTypes' table for the supplied typeID and retrieve the groupID for this type:
                if (!sDatabase.RunQuery(result,
                    " SELECT "
                    "    groupID "
                    " FROM invTypes "
                    " WHERE typeID = %u ", typeID))
                {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Error in query: %s", result.error.c_str() );
                    return NULL;
                }

                // Query went through, but check to see if there were zero rows, ie typeID was invalid,
                // and if not, then get the groupID from the row:
                if ( !(result.GetRow(row)) )
                {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Invalid typeID: %u, no rows returned in db query.", typeID );
                    return NULL;
                }
                groupID = row.GetUInt( 0 );

                // Calculate distance from target warpable object that the ship will warp to, using minimum safe distance
                // based upon groupID of the target object:
                switch( groupID )
                {
                    case 6: // target object is a SUN
                    case 7: // target object is a PLANET
                    case 8: // target object is a MOON
                        distance += 20000;
                        break;
                    default:
                        break;
                }

                // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
                SystemManager *sm = call.client->System();
                if(sm == NULL) {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: no system manager found", call.client->GetName() );
                    return NULL;
                }
                SystemEntity *se = sm->get( itemID );
                if(se ==  NULL) {
                    sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: unable to find location %d", call.client->GetName(), itemID );
                    return NULL;
                }

                // Add radiuses for ship and destination object:
                distance += call.client->GetRadius() + se->GetRadius();

				/*  check for fleet warp here
				 * else client warp solo
				 *
				 * std::map<std::string, PyRep *>::const_iterator res = call.byname.find("fleet");
				 * res->second->AsBool()->value()
				 * call.client->FleetWarp( se->GetPosition(), distance );
				 */

                destiny->WarpTo(bookmarkPosition, distance);
            }
		}
	}
	// none of the systems below are implemented.  hold on coding till systems are working.
	else if( arg.type == "launch" )
	{ // launchpickup - launch, launchid
		call.client->SendErrorMsg("WarpToLaunch is not implemented at this time.  See Allan for updates.");
	}
	else if( arg.type == "scan" )
	{//  scan, resultid, minrange, fleet(bool)
		call.client->SendErrorMsg("WarpToScan is not implemented at this time.  See Allan for updates.");
	}
	else if( arg.type == "epinstance" )
	{// epinstance, instanceid
		call.client->SendErrorMsg("WarpToInstance is not implemented at this time.  See Allan for updates.");
	}
	else if( arg.type == "tutorial" )
	{ // tutorial, none
		call.client->SendErrorMsg("WarpToTutorial is not implemented at this time.  See Allan for updates.");
	}
	//  fleet warping
	else if( arg.type == "char" )
	{// [warptomember] char, charid, minrange
		// [warpfleettomember] char, charid, minrange, fleet=1
		call.client->SendErrorMsg("WarpToChar is not implemented at this time.  See Allan for updates.");
	}
    else
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unhandled arg.type value: '%s'.", arg.type.c_str() );

    return NULL;
}

PyResult BeyonceBound::Handle_CmdWarpToStuffAutopilot(PyCallArgs &call) {
  //  sends targeted celestial itemID as arg.destID
        //sLog.Warning( "BeyonceBound", "Handle_CmdWarpToStuffAutopilot" );
    CallWarpToStuffAutopilot arg;

    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
	}

	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    //Change this to change the default autopilot distance (Faster Autopilot FTW)
    int32 distance = 5000; //15000

    //Don't update destiny until done with warp
    SystemManager *sm = call.client->System();
    if(sm == NULL) {
        codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
        return NULL;
    }
    SystemEntity *se = sm->get(arg.destID);
    if(se ==  NULL) {
	  codelog(CLIENT__ERROR, "%s: unable to find destinationID %u", call.client->GetName(), arg.destID);
        return NULL;
    }
    // autopilot check      --this has adverse effects at this time.  -allan 27Dec14
	//call.client->SetAutoPilot(true);

	//Adding in object radius
    distance += call.client->GetRadius() + se->GetRadius();
    destiny->WarpTo(se->GetPosition(), distance);

    return NULL;
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
    codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());

    //no arguments.

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

    destiny->SendSetState(call.client->Bubble());

    return NULL;
}

PyResult BeyonceBound::Handle_CmdStop(PyCallArgs &call) {
	//sLog.Warning( "BeyonceBound", "Handle_CmdStop" );
	DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	}

    // Only disallow Stopping ship when in warp state AND ship speed is greater than 0.75 times ship's maxVelocity
    if( (destiny->GetState() == Destiny::DSTBALL_WARP)
        && (destiny->GetVelocity().length() >= (0.75*call.client->GetShip()->GetAttribute(AttrMaxVelocity).get_float())) ) {
            call.client->SendNotifyMsg( "You can't do this while warping");
            return NULL;
    }

    destiny->Stop();

    return NULL;
}

// CmdTurboDock (in client code)
PyResult BeyonceBound::Handle_CmdDock(PyCallArgs &call) {
  /**
14:09:25 L BeyonceBound::Handle_CmdDock(): size= 2
14:09:25 [SvcCall]   Call Arguments:
14:09:25 [SvcCall]       Tuple: 2 elements
14:09:25 [SvcCall]         [ 0] Integer field: 60012124
14:09:25 [SvcCall]         [ 1] Integer field: 140001323
    call.Dump(SERVICE__CALLS);
	*/
  sLog.Log( "BeyonceBound::Handle_CmdDock()", "size= %u", call.tuple->size() );
    Call_TwoIntegerArgs arg;  //sends stationID, shipID
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
    if(destiny == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}
    SystemManager *sm = call.client->System();
    if(sm == NULL) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", call.client->GetName());
        return NULL;
    }

    // Set client to know what station it's trying to dock into just in case docking is delayed
    //  this also sets m_dockStationID for radius checks and other thigns
    call.client->SetDockStationID( arg.arg1 );

    // Attempt to Dock:
    return destiny->AttemptDockOperation();
}

PyResult BeyonceBound::Handle_CmdStargateJump(PyCallArgs &call) {
  /**CmdStargateJump(destID, theJump.toCelestialID, session.shipid)
  */
        sLog.Warning( "BeyonceBound", "Handle_CmdStargateJump" );
    // sends 3 args....fromGateID, toGateID, and shipID
    Call_StargateJump arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return NULL;
    }

    DestinyManager *destiny = call.client->Destiny();
	if(destiny == NULL) {
		codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
		return NULL;
	} else if(destiny->GetState() == Destiny::DSTBALL_WARP){
		call.client->SendNotifyMsg( "You can't do this while warping");
		return NULL;
	}

    call.client->StargateJump(arg.fromStargateID, arg.toStargateID);
    return NULL;
}

PyResult BeyonceBound::Handle_CmdAbandonLoot(PyCallArgs &call) {
	/*  remotePark.CmdAbandonLoot(wrecks)  <- this is pylist from 'abandonAllWrecks'
	 *  remotePark.CmdAbandonLoot([wreckID]) <- single itemID
	 */
  sLog.Log( "BeyonceBound::Handle_CmdAbandonLoot()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALLS);

	Call_SingleIntList arg;
	if(!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return NULL;
	}
	//arg.ints is list sent by client

    return NULL;
}
