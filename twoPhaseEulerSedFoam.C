/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    twoPhaseEulerFoam

Description
    Solver for a system of 2 incompressible fluid phases with one phase
    dispersed, e.g. gas bubbles in a liquid or solid particles in a gas.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "nearWallDist.H"
#include "wallFvPatch.H"
#include "Switch.H"

#include "IFstream.H"
#include "OFstream.H"
#include "symmetryFvPatchFields.H"

#include "dragModel.H"
#include "phaseModel.H"
#include "kineticTheoryModel.H"
#include "mathematicalConstants.H"
#include "pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "readGravitationalAcceleration.H"
    #include "createFields.H"
    #include "readPPProperties.H"
    #include "initContinuityErrs.H"
    #include "readTimeControls.H"
    #include "CourantNo.H"
    #include "setInitialDeltaT.H"
    #include "createGradP.H"
    #include "OpenFileForGradPOSC.H"
    pimpleControl pimple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;
    if(SUS.value()==0) Info<<"Turbulence suspension term is neglected"<<endl;
    else if(SUS.value()>0) Info<<"Turbulence suspension term is included"<<endl;
    else Info<<"Turbulence suspension coeffcient SUS can't be negative"<<endl;
    while (runTime.run())
    {
        #include "readTwoPhaseEulerFoamControls.H"
        #include "CourantNos.H"
        #include "alphaCourantNo.H"
        #include "setDeltaT.H"

        runTime++;
        Info<< "Time = " << runTime.timeName() << nl << endl;

        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            #include "alphaEqn.H"
            #include "liftDragCoeffs.H"
            if (pimple.turbCorr())
            {
                #include "kEpsilon.H"
            }
            #include "callKineticTheory.H"
            #include "UEqns.H"
            // --- Pressure corrector loop
            while (pimple.correct())
            {
                #include "pEqn.H"
//                #include "liftDragCoeffs.H"
                if (correctAlpha && !pimple.finalIter())
                {
                    #include "alphaEqn.H"
                }
            }
            #include "DDtU.H"
        }
        #include "OutputGradPOSC.H"
        #include "write.H"

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
