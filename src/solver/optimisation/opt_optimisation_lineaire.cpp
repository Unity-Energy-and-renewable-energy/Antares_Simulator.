/*
 * Copyright 2007-2024, RTE (https://www.rte-france.com)
 * See AUTHORS.txt
 * SPDX-License-Identifier: MPL-2.0
 * This file is part of Antares-Simulator,
 * Adequacy and Performance assessment for interconnected energy networks.
 *
 * Antares_Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public Licence 2.0 as published by
 * the Mozilla Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Antares_Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Mozilla Public Licence 2.0 for more details.
 *
 * You should have received a copy of the Mozilla Public Licence 2.0
 * along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
 */

#include <antares/logs/logs.h>
#include "antares/solver/optimisation/LinearProblemMatrix.h"
#include "antares/solver/optimisation/constraints/constraint_builder_utils.h"
#include "antares/solver/optimisation/opt_export_structure.h"
#include "antares/solver/optimisation/opt_fonctions.h"
#include "antares/solver/simulation/ISimulationObserver.h"
#include "antares/solver/simulation/sim_structure_probleme_economique.h"
#include "antares/solver/utils/filename.h"

using namespace Antares::Solver;
using Antares::Solver::Optimization::OptimizationOptions;

namespace
{
double OPT_ObjectiveFunctionResult(const PROBLEME_HEBDO* Probleme,
                                   const int NumeroDeLIntervalle,
                                   const int optimizationNumber)
{
    if (optimizationNumber == PREMIERE_OPTIMISATION)
    {
        return Probleme->coutOptimalSolution1[NumeroDeLIntervalle];
    }
    else
    {
        return Probleme->coutOptimalSolution2[NumeroDeLIntervalle];
    }
}

void OPT_EcrireResultatFonctionObjectiveAuFormatTXT(
  double optimalSolutionCost,
  const OptPeriodStringGenerator& optPeriodStringGenerator,
  int optimizationNumber,
  Solver::IResultWriter& writer)
{
    Yuni::Clob buffer;
    auto filename = createCriterionFilename(optPeriodStringGenerator, optimizationNumber);

    logs.info() << "Solver Criterion File: `" << filename << "'";

    buffer.appendFormat("* Optimal criterion value :   %11.10e\n", optimalSolutionCost);
    writer.addEntryFromBuffer(filename, buffer);
}

namespace
{
void notifyProblemHebdo(const PROBLEME_HEBDO* problemeHebdo,
                        int optimizationNumber,
                        Solver::Simulation::ISimulationObserver& simulationObserver,
                        const OptPeriodStringGenerator* optPeriodStringGenerator)
{
    simulationObserver.notifyHebdoProblem(*problemeHebdo,
                                          optimizationNumber,
                                          createMPSfilename(*optPeriodStringGenerator,
                                                            optimizationNumber));
}
} // namespace

bool runWeeklyOptimization(const OptimizationOptions& options,
                           PROBLEME_HEBDO* problemeHebdo,
                           Solver::IResultWriter& writer,
                           int optimizationNumber,
                           Solver::Simulation::ISimulationObserver& simulationObserver)
{
    const int NombreDePasDeTempsPourUneOptimisation = problemeHebdo
                                                        ->NombreDePasDeTempsPourUneOptimisation;

    int DernierPdtDeLIntervalle;
    for (uint pdtHebdo = 0, numeroDeLIntervalle = 0; pdtHebdo < problemeHebdo->NombreDePasDeTemps;
         pdtHebdo = DernierPdtDeLIntervalle, numeroDeLIntervalle++)
    {
        int PremierPdtDeLIntervalle = pdtHebdo;
        DernierPdtDeLIntervalle = pdtHebdo + NombreDePasDeTempsPourUneOptimisation;

        OPT_InitialiserLesBornesDesVariablesDuProblemeLineaire(problemeHebdo,
                                                               PremierPdtDeLIntervalle,
                                                               DernierPdtDeLIntervalle,
                                                               optimizationNumber);

        OPT_InitialiserLeSecondMembreDuProblemeLineaire(problemeHebdo,
                                                        PremierPdtDeLIntervalle,
                                                        DernierPdtDeLIntervalle,
                                                        numeroDeLIntervalle,
                                                        optimizationNumber);

        OPT_InitialiserLesCoutsLineaire(problemeHebdo,
                                        PremierPdtDeLIntervalle,
                                        DernierPdtDeLIntervalle);

        // An optimization period represents a sequence as <year>-<week> or <year>-<week>-<day>,
        // depending whether the optimization is daily or weekly.
        // These sequences are used when building the names of MPS or criterion files.
        auto optPeriodStringGenerator = createOptPeriodAsString(
          problemeHebdo->OptimisationAuPasHebdomadaire,
          numeroDeLIntervalle,
          problemeHebdo->weekInTheYear,
          problemeHebdo->year);

        notifyProblemHebdo(problemeHebdo,
                           optimizationNumber,
                           simulationObserver,
                           optPeriodStringGenerator.get());

        if (!OPT_AppelDuSimplexe(options,
                                 problemeHebdo,
                                 numeroDeLIntervalle,
                                 optimizationNumber,
                                 *optPeriodStringGenerator,
                                 writer))
        {
            return false;
        }

        if (problemeHebdo->ExportMPS != Data::mpsExportStatus::NO_EXPORT)
        {
            double optimalSolutionCost = OPT_ObjectiveFunctionResult(problemeHebdo,
                                                                     numeroDeLIntervalle,
                                                                     optimizationNumber);
            OPT_EcrireResultatFonctionObjectiveAuFormatTXT(optimalSolutionCost,
                                                           *optPeriodStringGenerator,
                                                           optimizationNumber,
                                                           writer);
        }
    }
    return true;
}

void runThermalHeuristic(PROBLEME_HEBDO* problemeHebdo)
{
    if (problemeHebdo->OptimisationAvecCoutsDeDemarrage)
    {
        OPT_AjusterLeNombreMinDeGroupesDemarresCoutsDeDemarrage(problemeHebdo);
    }
    else
    {
        OPT_CalculerLesPminThermiquesEnFonctionDeMUTetMDT(problemeHebdo);
    }
}

void resizeProbleme(PROBLEME_ANTARES_A_RESOUDRE* ProblemeAResoudre,
                    unsigned nombreDeVariables,
                    unsigned nombreDeContraintes)
{
    ProblemeAResoudre->CoutQuadratique.resize(nombreDeVariables);
    ProblemeAResoudre->CoutLineaire.resize(nombreDeVariables);
    ProblemeAResoudre->TypeDeVariable.resize(nombreDeVariables);
    ProblemeAResoudre->Xmin.resize(nombreDeVariables);
    ProblemeAResoudre->Xmax.resize(nombreDeVariables);
    ProblemeAResoudre->X.resize(nombreDeVariables);
    ProblemeAResoudre->AdresseOuPlacerLaValeurDesVariablesOptimisees.resize(nombreDeVariables);
    ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsReduits.resize(nombreDeVariables);
    ProblemeAResoudre->PositionDeLaVariable.resize(nombreDeVariables);
    ProblemeAResoudre->NomDesVariables.resize(nombreDeVariables);
    ProblemeAResoudre->VariablesEntieres.resize(nombreDeVariables);

    ProblemeAResoudre->Sens.resize(nombreDeContraintes);
    ProblemeAResoudre->IndicesDebutDeLigne.resize(nombreDeContraintes);
    ProblemeAResoudre->NombreDeTermesDesLignes.resize(nombreDeContraintes);
    ProblemeAResoudre->SecondMembre.resize(nombreDeContraintes);
    ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsMarginaux.resize(nombreDeContraintes);
    ProblemeAResoudre->CoutsMarginauxDesContraintes.resize(nombreDeContraintes);
    ProblemeAResoudre->ComplementDeLaBase.resize(nombreDeContraintes);
    ProblemeAResoudre->NomDesContraintes.resize(nombreDeContraintes);
}
} // namespace

void OPT_ExportRawOptimizationResults(PROBLEME_HEBDO* problemeHebdo,
                                      Solver::IResultWriter& writer,
                                      std::string filename)
{
    if (problemeHebdo->exportRawOptimizationResults)
    {
        uint32_t NombreDePasDeTempsProblemeHebdo = problemeHebdo->NombreDePasDeTemps;
        const std::string fileName = std::string("RawResultsWeek") + std::to_string(problemeHebdo->weekInTheYear) + "_" + filename + ".txt";
        std::string content;
        std::string baseContent;
        content += "year:\t" + problemeHebdo->year;
        content += "weekInTheYear:\t" + problemeHebdo->weekInTheYear;
        content += "OptimisationAuPasHebdomadaire:\t" + problemeHebdo->OptimisationAuPasHebdomadaire;
        content += "TypeDeLissageHydraulique:\t" + problemeHebdo->TypeDeLissageHydraulique;
        content += "WaterValueAccurate:\t" + problemeHebdo->WaterValueAccurate;
        content += "OptimisationAvecCoutsDeDemarrage:\t" + problemeHebdo->OptimisationAvecCoutsDeDemarrage;
        content += "OptimisationAvecVariablesEntieres:\t" + problemeHebdo->OptimisationAvecVariablesEntieres;
        content += "NombreDePays:\t" + problemeHebdo->NombreDePays;
        content += "NombreDePaliersThermiques:\t" + problemeHebdo->NombreDePaliersThermiques;
        content += "NombreDInterconnexions:\t" + problemeHebdo->NombreDInterconnexions;
        content += "NombreDePasDeTemps:\t" + problemeHebdo->NombreDePasDeTemps;
        content += "NombreDePasDeTempsPourUneOptimisation:\t" + problemeHebdo->NombreDePasDeTempsPourUneOptimisation;
        content += "NombreDeJours:\t" + problemeHebdo->NombreDeJours;
        content += "NombreDePasDeTempsDUneJournee:\t" + problemeHebdo->NombreDePasDeTempsDUneJournee;
        content += "NumberOfShortTermStorages:\t" + problemeHebdo->NumberOfShortTermStorages;
        content += "NbTermesContraintesPourLesCoutsDeDemarrage:\t" + problemeHebdo->NbTermesContraintesPourLesCoutsDeDemarrage;
        content += "TypeDOptimisation:\t" + problemeHebdo->TypeDOptimisation;
        content += "NombreDeContraintesCouplantes:\t" + problemeHebdo->NombreDeContraintesCouplantes;
        content += "ReinitOptimisation:\t" + problemeHebdo->ReinitOptimisation;
        content += "exportMPSOnError:\t" + problemeHebdo->exportMPSOnError;
        content += "NamedProblems:\t" + problemeHebdo->NamedProblems;
        content += "exportRawOptimizationResults:\t" + problemeHebdo->exportRawOptimizationResults;
        content += "HeureDansLAnnee:\t" + problemeHebdo->HeureDansLAnnee;
        content += "LeProblemeADejaEteInstancie:\t" + problemeHebdo->LeProblemeADejaEteInstancie;
        content += "firstWeekOfSimulation:\t" + problemeHebdo->firstWeekOfSimulation;
        content += "Expansion:\t" + problemeHebdo->Expansion;
        content += "YaDeLaReserveJmoins1:\t" + problemeHebdo->YaDeLaReserveJmoins1;
        //Interconnexion
        for (uint indexInterco = 0; indexInterco < problemeHebdo->NombreDInterconnexions; indexInterco++)
        {
            content += "Interconnexion<" + std::to_string(indexInterco) + ">:PaysOrigineDeLInterconnexion:\t" + std::string(problemeHebdo->NomsDesPays[problemeHebdo->PaysOrigineDeLInterconnexion[indexInterco]]);
            content += "Interconnexion<" + std::to_string(indexInterco) + ">:PaysExtremiteDeLInterconnexion:\t" + std::string(problemeHebdo->NomsDesPays[problemeHebdo->PaysExtremiteDeLInterconnexion[indexInterco]]);
            baseContent = "Interconnexion<" + std::to_string(indexInterco) + ">:CoutDeTransport:";
            auto coutDeTransport = problemeHebdo->CoutDeTransport[indexInterco];
            content += baseContent + "IntercoGereeAvecDesCouts:\t" + std::to_string(coutDeTransport.IntercoGereeAvecDesCouts);
            content += baseContent + "IntercoGereeAvecLoopFlow:\t" + std::to_string(coutDeTransport.IntercoGereeAvecLoopFlow);
            for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
            {
                content += baseContent + "CoutDeTransportOrigineVersExtremite<" + std::to_string(pdtHebdo)  +">:\t" + std::to_string(coutDeTransport.CoutDeTransportOrigineVersExtremite[pdtHebdo]);
                content += baseContent + "CoutDeTransportExtremiteVersOrigine<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(coutDeTransport.CoutDeTransportExtremiteVersOrigine[pdtHebdo]);
                content += baseContent + "CoutDeTransportOrigineVersExtremiteRef<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(coutDeTransport.CoutDeTransportOrigineVersExtremiteRef[pdtHebdo]);
                content += baseContent + "CoutDeTransportExtremiteVersOrigineRef<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(coutDeTransport.CoutDeTransportExtremiteVersOrigineRef[pdtHebdo]);
            }
            baseContent = "Interconnexion<" + std::to_string(indexInterco) + ">:ValeursDeNTC:";
            for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
            {
                content += baseContent + "ValeurDeNTCOrigineVersExtremite<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ValeursDeNTC[pdtHebdo].ValeurDeNTCOrigineVersExtremite[indexInterco]);
                content += baseContent + "ValeurDeNTCExtremiteVersOrigine<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ValeursDeNTC[pdtHebdo].ValeurDeNTCExtremiteVersOrigine[indexInterco]);
                content += baseContent + "ValeurDeLoopFlowOrigineVersExtremite<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ValeursDeNTC[pdtHebdo].ValeurDeLoopFlowOrigineVersExtremite[indexInterco]);
                content += baseContent + "ResistanceApparente<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ValeursDeNTC[pdtHebdo].ResistanceApparente[indexInterco]);
                content += baseContent + "ValeurDuFlux<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ValeursDeNTC[pdtHebdo].ValeurDuFlux[indexInterco]);
            }
        } 

        for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            std::string areaName = problemeHebdo->NomsDesPays[pays];
            content += "area<" + areaName + ">:CoutDeDefaillancePositive:\t" + std::to_string(problemeHebdo->CoutDeDefaillancePositive[pays]) + "\r\n";
            content += "area<" + areaName + ">:CoutDeDefaillanceNegative:\t" + std::to_string(problemeHebdo->CoutDeDefaillanceNegative[pays]) + "\r\n";
            content += "area<" + areaName + ">:DefaillanceNegativeUtiliserPMinThermique(bool):\t" + std::to_string(problemeHebdo->DefaillanceNegativeUtiliserPMinThermique[pays]) + "\r\n";
            content += "area<" + areaName + ">:DefaillanceNegativeUtiliserHydro(bool):\t" + std::to_string(problemeHebdo->DefaillanceNegativeUtiliserHydro[pays]) + "\r\n";
            content += "area<" + areaName + ">:DefaillanceNegativeUtiliserConsoAbattue(bool):\t" + std::to_string(problemeHebdo->DefaillanceNegativeUtiliserConsoAbattue[pays]) + "\r\n";
            for (int indexBruit = 0; indexBruit < problemeHebdo->BruitSurCoutHydraulique[pays].size(); indexBruit++)
            {
                content += "area<" + areaName + ">:BruitSurCoutHydraulique<" + std::to_string(indexBruit) +">:\t" + std::to_string(problemeHebdo->BruitSurCoutHydraulique[pays][indexBruit]) + "\r\n";
            }
            //Paliers thermiques
            PALIERS_THERMIQUES& PaliersThermiquesDuPays
                = problemeHebdo->PaliersThermiquesDuPays[pays];
            baseContent= "PaliersThermiquesDuPays<" + areaName + ">:";
            content += baseContent + "NombreDePaliersThermiques:\t" + std::to_string(PaliersThermiquesDuPays.NombreDePaliersThermiques);
            for (int indexThermique = 0; indexThermique < PaliersThermiquesDuPays.NombreDePaliersThermiques; indexThermique++)
            {
                std::string clusterName = PaliersThermiquesDuPays.NomsDesPaliersThermiques[indexThermique];
                content += baseContent + "minUpDownTime<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.minUpDownTime[indexThermique]) + "\r\n";
                content += baseContent + "TailleUnitaireDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.TailleUnitaireDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "PminDuPalierThermiquePendantUneHeure<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.PminDuPalierThermiquePendantUneHeure[indexThermique]) + "\r\n";
                content += baseContent + "PminDuPalierThermiquePendantUnJour<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.PminDuPalierThermiquePendantUnJour[indexThermique]) + "\r\n";
                content += baseContent + "NumeroDuPalierDansLEnsembleDesPaliersThermiques<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.NumeroDuPalierDansLEnsembleDesPaliersThermiques[indexThermique]) + "\r\n";
                for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
                {
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:PuissanceDisponibleDuPalierThermique<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].PuissanceDisponibleDuPalierThermique[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:PuissanceDisponibleDuPalierThermiqueRef<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].PuissanceDisponibleDuPalierThermiqueRef[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:PuissanceMinDuPalierThermique<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].PuissanceMinDuPalierThermique[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:PuissanceMinDuPalierThermiqueRef<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].PuissanceMinDuPalierThermiqueRef[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:CoutHoraireDeProductionDuPalierThermique<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].CoutHoraireDeProductionDuPalierThermique[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:NombreMaxDeGroupesEnMarcheDuPalierThermique<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].NombreMaxDeGroupesEnMarcheDuPalierThermique[pdtHebdo]) + "\r\n";
                    content += baseContent + "PuissanceDisponibleEtCout<" + clusterName + ">:NombreMinDeGroupesEnMarcheDuPalierThermique<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(PaliersThermiquesDuPays.PuissanceDisponibleEtCout[indexThermique].NombreMinDeGroupesEnMarcheDuPalierThermique[pdtHebdo]) + "\r\n";

                }
                content += baseContent + "CoutDeDemarrageDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.CoutDeDemarrageDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "CoutDArretDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.CoutDArretDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "CoutFixeDeMarcheDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.CoutFixeDeMarcheDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "pminDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.pminDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "PmaxDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.PmaxDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "PmaxDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.PmaxDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "DureeMinimaleDeMarcheDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.DureeMinimaleDeMarcheDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "DureeMinimaleDArretDUnGroupeDuPalierThermique<" + clusterName + ">:\t" + std::to_string(PaliersThermiquesDuPays.DureeMinimaleDArretDUnGroupeDuPalierThermique[indexThermique]) + "\r\n";
                content += baseContent + "NomsDesPaliersThermiques<" + clusterName + ">:\t" + PaliersThermiquesDuPays.NomsDesPaliersThermiques[indexThermique] + "\r\n";
            }
            //Hydro
            auto& CaracteristiquesHydrauliques
                = problemeHebdo->CaracteristiquesHydrauliques[pays];
            baseContent = "CaracteristiquesHydrauliques<" + areaName + ">:";
            for (int intervalle = 0; intervalle < CaracteristiquesHydrauliques.MinEnergieHydrauParIntervalleOptimise.size() ; intervalle++ )
            {
                content += baseContent + "MinEnergieHydrauParIntervalleOptimise:intervalle<" + std::to_string(intervalle) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.MinEnergieHydrauParIntervalleOptimise[intervalle]) + "\r\n";
                content += baseContent + "MaxEnergieHydrauParIntervalleOptimise:intervalle<" + std::to_string(intervalle) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.MaxEnergieHydrauParIntervalleOptimise[intervalle]) + "\r\n";
                content += baseContent + "CntEnergieH2OParIntervalleOptimise:intervalle<" + std::to_string(intervalle) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.CntEnergieH2OParIntervalleOptimise[intervalle]) + "\r\n";
                content += baseContent + "CntEnergieH2OParJour:intervalle<" + std::to_string(intervalle) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.CntEnergieH2OParJour[intervalle]) + "\r\n";
                content += baseContent + "MaxEnergiePompageParIntervalleOptimise:intervalle<" + std::to_string(intervalle) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.MaxEnergiePompageParIntervalleOptimise[intervalle]) + "\r\n";
            }
            for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
            {
                content += baseContent + "ContrainteDePmaxHydrauliqueHoraire<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.ContrainteDePmaxHydrauliqueHoraire[pdtHebdo]) + "\r\n";
                content += baseContent + "ContrainteDePmaxHydrauliqueHoraireRef<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.ContrainteDePmaxHydrauliqueHoraireRef[pdtHebdo]) + "\r\n";
                content += baseContent + "ContrainteDePmaxPompageHoraire<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.ContrainteDePmaxPompageHoraire[pdtHebdo]) + "\r\n";
                content += baseContent + "NiveauHoraireSup<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.NiveauHoraireSup[pdtHebdo]) + "\r\n";
                content += baseContent + "NiveauHoraireInf<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.NiveauHoraireInf[pdtHebdo]) + "\r\n";
                content += baseContent + "ApportNaturelHoraire<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.ApportNaturelHoraire[pdtHebdo]) + "\r\n";
                content += baseContent + "MingenHoraire<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.MingenHoraire[pdtHebdo]) + "\r\n";
            }
            content += baseContent + "MaxDesPmaxHydrauliques:\t" + std::to_string(CaracteristiquesHydrauliques.MaxDesPmaxHydrauliques) + "\r\n";
            content += baseContent + "PresenceDePompageModulable:\t" + std::to_string(CaracteristiquesHydrauliques.PresenceDePompageModulable) + "\r\n";
            content += baseContent + "PenalisationDeLaVariationDeProductionHydrauliqueSurSommeDesVariations:\t" + std::to_string(CaracteristiquesHydrauliques.PenalisationDeLaVariationDeProductionHydrauliqueSurSommeDesVariations) + "\r\n";
            content += baseContent + "PenalisationDeLaVariationDeProductionHydrauliqueSurVariationMax:\t" + std::to_string(CaracteristiquesHydrauliques.PenalisationDeLaVariationDeProductionHydrauliqueSurVariationMax) + "\r\n";
            content += baseContent + "WeeklyWaterValueStateRegular:\t" + std::to_string(CaracteristiquesHydrauliques.WeeklyWaterValueStateRegular) + "\r\n";
            content += baseContent + "TurbinageEntreBornes(bool):\t" + std::to_string(CaracteristiquesHydrauliques.TurbinageEntreBornes) + "\r\n";
            content += baseContent + "SansHeuristique(bool):\t" + std::to_string(CaracteristiquesHydrauliques.SansHeuristique) + "\r\n";
            content += baseContent + "SuiviNiveauHoraire(bool):\t" + std::to_string(CaracteristiquesHydrauliques.SuiviNiveauHoraire) + "\r\n";
            content += baseContent + "NiveauInitialReservoir:\t" + std::to_string(CaracteristiquesHydrauliques.NiveauInitialReservoir) + "\r\n";
            content += baseContent + "TailleReservoir:\t" + std::to_string(CaracteristiquesHydrauliques.TailleReservoir) + "\r\n";
            content += baseContent + "PumpingRatio:\t" + std::to_string(CaracteristiquesHydrauliques.PumpingRatio) + "\r\n";
            content += baseContent + "WeeklyGeneratingModulation:\t" + std::to_string(CaracteristiquesHydrauliques.WeeklyGeneratingModulation) + "\r\n";
            content += baseContent + "WeeklyPumpingModulation:\t" + std::to_string(CaracteristiquesHydrauliques.WeeklyPumpingModulation) + "\r\n";
            content += baseContent + "DirectLevelAccess(bool):\t" + std::to_string(CaracteristiquesHydrauliques.DirectLevelAccess) + "\r\n";
            content += baseContent + "AccurateWaterValue(bool):\t" + std::to_string(CaracteristiquesHydrauliques.AccurateWaterValue) + "\r\n";
            content += baseContent + "LevelForTimeInterval:\t" + std::to_string(CaracteristiquesHydrauliques.LevelForTimeInterval) + "\r\n";
            for (int layerindex = 0; layerindex < 100; layerindex++)
            {
                content += baseContent + "WaterLayerValues:layer<" + std::to_string(layerindex) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.WaterLayerValues[layerindex]) + "\r\n";
                content += baseContent + "InflowForTimeInterval:layer<" + std::to_string(layerindex) + ">:\t" + std::to_string(CaracteristiquesHydrauliques.InflowForTimeInterval[layerindex]) + "\r\n";
            }

            //STS
            baseContent = "ShortTermStorage<" + areaName + ">:"; 
            //problemeHebdo->ShortTermStorage[pays]
            auto& ShortTermStorage
                = problemeHebdo->ShortTermStorage[pays];
            for (int STSindex = 0; STSindex < ShortTermStorage.size(); STSindex++)
            {
                std::string stsName = ShortTermStorage[STSindex].name;
                content += baseContent + "reservoirCapacity:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].reservoirCapacity) + "\r\n";
                content += baseContent + "injectionNominalCapacity:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].injectionNominalCapacity) + "\r\n";
                content += baseContent + "withdrawalNominalCapacity:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].withdrawalNominalCapacity) + "\r\n";
                content += baseContent + "injectionEfficiency:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].injectionEfficiency) + "\r\n";
                content += baseContent + "withdrawalEfficiency:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].withdrawalEfficiency) + "\r\n";
                content += baseContent + "initialLevel:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].initialLevel) + "\r\n";
                content += baseContent + "initialLevelOptim(bool):<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].initialLevelOptim) + "\r\n";
                content += baseContent + "clusterGlobalIndex:<" + stsName + ">:\t" + std::to_string(ShortTermStorage[STSindex].clusterGlobalIndex) + "\r\n";
            }
            for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
            {
                content += "SoldeMoyenHoraire:<" + areaName + ">:SoldeMoyenDuPays<" +  std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->SoldeMoyenHoraire[pdtHebdo].SoldeMoyenDuPays[pays]) + "\r\n";
            }
        }

        for (uint32_t pdtHebdo = 0; pdtHebdo < NombreDePasDeTempsProblemeHebdo; pdtHebdo++)
        {
            content += "NumeroDeJourDuPasDeTemps<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->NumeroDeJourDuPasDeTemps[pdtHebdo]) + "\r\n";
            content += "NumeroDIntervalleOptimiseDuPasDeTemps<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->NumeroDIntervalleOptimiseDuPasDeTemps[pdtHebdo]) + "\r\n";

            for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
            {
                std::string areaName = problemeHebdo->NomsDesPays[pays];
                RESULTATS_HORAIRES& ResultatsHoraires = problemeHebdo->ResultatsHoraires[pays];
                std::vector<PRODUCTION_THERMIQUE_OPTIMALE>& ProductionThermique = ResultatsHoraires.ProductionThermique;
                PALIERS_THERMIQUES& PaliersThermiquesDuPays
                    = problemeHebdo->PaliersThermiquesDuPays[pays];
                
                content += "area<" + areaName + ">:ConsommationsAbattues:ConsommationAbattueDuPays<" + std::to_string(pdtHebdo) + ">:\t" + std::to_string(problemeHebdo->ConsommationsAbattues[pdtHebdo].ConsommationAbattueDuPays[pays]) + "\r\n";

                for (int index = 0; index < PaliersThermiquesDuPays.NombreDePaliersThermiques; index++)
                {
                    std::string clusterName = PaliersThermiquesDuPays.NomsDesPaliersThermiques[index];
                
                    baseContent = "ResultatsHoraires:ProductionThermique:area<" + areaName + ">:ThermalCluster<" + clusterName + ">:";
                    content += baseContent + "NombreDeGroupesEnMarcheDuPalier<" + std::to_string(pdtHebdo) + ">" + "\t" + std::to_string(ProductionThermique[pdtHebdo].NombreDeGroupesEnMarcheDuPalier[index]) + "\r\n";
                    content += baseContent + "NombreDeGroupesQuiDemarrentDuPalier<" + std::to_string(pdtHebdo) + ">" + "\t" + std::to_string(ProductionThermique[pdtHebdo].NombreDeGroupesQuiDemarrentDuPalier[index]) + "\r\n";
                    content += baseContent + "NombreDeGroupesQuiSArretentDuPalier<" + std::to_string(pdtHebdo) + ">" + "\t" + std::to_string(ProductionThermique[pdtHebdo].NombreDeGroupesQuiSArretentDuPalier[index]) + "\r\n";
                    content += baseContent + "NombreDeGroupesQuiTombentEnPanneDuPalier<" + std::to_string(pdtHebdo) + ">" + "\t" + std::to_string(ProductionThermique[pdtHebdo].NombreDeGroupesQuiTombentEnPanneDuPalier[index]) + "\r\n";
                }
            }
            writer.addEntryFromBuffer(fileName, content);
        }
    }
}

bool OPT_OptimisationLineaire(const OptimizationOptions& options,
                              PROBLEME_HEBDO* problemeHebdo,
                              Solver::IResultWriter& writer,
                              Solver::Simulation::ISimulationObserver& simulationObserver)
{
    if (!problemeHebdo->OptimisationAuPasHebdomadaire)
    {
        problemeHebdo->NombreDePasDeTempsPourUneOptimisation = problemeHebdo
                                                                 ->NombreDePasDeTempsDUneJournee;
    }
    else
    {
        problemeHebdo->NombreDePasDeTempsPourUneOptimisation = problemeHebdo->NombreDePasDeTemps;
    }

    OPT_NumeroDeJourDuPasDeTemps(problemeHebdo);

    OPT_NumeroDIntervalleOptimiseDuPasDeTemps(problemeHebdo);

    OPT_RestaurerLesDonnees(problemeHebdo);

    OPT_ConstruireLaListeDesVariablesOptimiseesDuProblemeLineaire(problemeHebdo);

    auto builder_data = NewGetConstraintBuilderFromProblemHebdo(problemeHebdo);
    ConstraintBuilder builder(builder_data);
    LinearProblemMatrix linearProblemMatrix(problemeHebdo, builder);
    linearProblemMatrix.Run();
    resizeProbleme(problemeHebdo->ProblemeAResoudre.get(),
                   problemeHebdo->ProblemeAResoudre->NombreDeVariables,
                   problemeHebdo->ProblemeAResoudre->NombreDeContraintes);
    if (problemeHebdo->ExportStructure && problemeHebdo->firstWeekOfSimulation)
    {
        OPT_ExportStructures(problemeHebdo, writer);
    }

    bool ret = runWeeklyOptimization(options,
                                     problemeHebdo,
                                     writer,
                                     PREMIERE_OPTIMISATION,
                                     simulationObserver);
    std::string filename = "beforeHeuristic";
    OPT_ExportRawOptimizationResults(problemeHebdo, writer, filename);

    // We only need the 2nd optimization when NOT solving with integer variables
    // We also skip the 2nd optimization in the hidden 'Expansion' mode
    // and if the 1st one failed.
    if (ret && !problemeHebdo->Expansion && !problemeHebdo->OptimisationAvecVariablesEntieres)
    {
        // We need to adjust some stuff before running the 2nd optimisation
        runThermalHeuristic(problemeHebdo);
        ret = runWeeklyOptimization(options,
                                     problemeHebdo,
                                     writer,
                                     DEUXIEME_OPTIMISATION,
                                     simulationObserver);
        filename = "afterHeuristic";
        OPT_ExportRawOptimizationResults(problemeHebdo, writer, filename);
    }
    return ret;
}
