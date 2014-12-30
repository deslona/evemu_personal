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
    Author:     Allan
*/

#include "eve-server.h"

#include "scanning/Scanning.h"



Scanning::Scanning()
{

}

Scanning::~Scanning()
{

}

/*
 * Data()
{

  scanSvc.GetProbeData (probeID)
  scanSvc.GetActiveProbes (probeID)
  scanSvc.GetProbeLabel(probe.probeID)
} */

void Scanning::ScanStart()
{

}

PyRep *Scanning::ScanResult() {

    /*
                  [PyObjectData Name: util.KeyVal]
                    [PyDict 12 kvp]
                      [PyString "deviation"]
                      [PyInt 0]
                      [PyString "typeID"]
                      [PyInt 28356]
                      [PyString "probeID"]
                      [PyIntegerVar 1328534410]
                      [PyString "scanGroupID"]
                      [PyInt 128]
                      [PyString "certainty"]
                      [PyFloat 1]
                      [PyString "pos"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyToken foo.Vector3]
                          [PyTuple 3 items]
                            [PyFloat 924428329679.125]
                            [PyFloat 8194006917.24071]
                            [PyFloat 165918845190.904]
                      [PyString "groupID"]
                      [PyInt 885]
                      [PyString "strengthAttributeID"]
                      [PyInt 1136]
                      [PyString "dungeonName"]
                      [PyString "Drone Squad"]
                      [PyString "degraded"]
                      [PyBool False]
                      [PyString "data"]
                      [PyObjectEx Type2]
                        [PyTuple 2 items]
                          [PyTuple 1 items]
                            [PyObjectEx Type2]
                              [PyTuple 2 items]
                                [PyTuple 1 items]
                                  [PyToken foo.Vector3]
                                [PyTuple 3 items]
                                  [PyFloat 924428329679.125]
                                  [PyFloat 8194006917.24071]
                                  [PyFloat 165918845190.904]
                          [PyTuple 3 items]
                            [PyFloat -1249402429440]
                            [PyFloat 17663139840]
                            [PyFloat -2266524672000]
                      [PyString "id"]
                      [PyString "ODR-392"]
                      */
    return new PyInt(0);
}

void Scanning::SurveyScan() {
    /*
     *
     *                      [PyString "OnSpecialFX"]
     *                      [PyTuple 14 items]
     *                        [PyIntegerVar 1002331681462]
     *                        [PyIntegerVar 1002332233248]
     *                        [PyInt 444]
     *                        [PyNone]
     *                        [PyNone]
     *                        [PyList 0 items]
     *                        [PyString "effects.SurveyScan"]
     *                        [PyBool False]
     *                        [PyInt 1]
     *                        [PyInt 1]
     *                        [PyFloat 4250]
     *                        [PyInt 0]
     *                        [PyIntegerVar 129509430135552798]
     *                        [PyNone]
     *
     *
     *                [PyList 2 items]
     *                  [PyTuple 2 items]
     *                    [PyString "OnSurveyScanComplete"]
     *                    [PyList 0 items]
     *
     */
}


/*
AttrScanGravimetricStrengthBonus = 238,
AttrScanLadarStrengthBonus = 239,
AttrScanMagnetometricStrengthBonus = 240,
AttrScanRadarStrengthBonus = 241,
AttrScanSpeedMultiplier = 242,
*/