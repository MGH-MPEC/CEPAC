#include "include.h"

/** \brief Constructor takes in the patient object and determines if updates can occur */
EndMonthUpdater::EndMonthUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
EndMonthUpdater::~EndMonthUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void EndMonthUpdater::performInitialUpdates() {
	/** Call the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void EndMonthUpdater::performMonthlyUpdates() {
	/** Uses a half month for applicable costs and survival stats in the month of death */
	double percentOfMonth = 1.0;
	if (!patient->isAlive()) {
		percentOfMonth = 0.5;
	}

	SimContext::PEDS_COST_AGE pedsCostAgeCat = patient->getPedsState()->ageCategoryPedsCost;
	int costAgeCat = patient->getGeneralState()->ageCategoryCost;
	SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
	/** Increment the monthly patient costs and statistics for routine care */
	if(pedsCostAgeCat == SimContext::PEDS_COST_AGE_ADULT){
		double multiplier = simContext->getLTFUInputs()->propGeneralMedicineCost[patient->getMonitoringState()->careState];
		incrementCostsGeneralMedicine(simContext->getCostInputs()->generalMedicineCost[gender][costAgeCat], percentOfMonth, multiplier);
	}

	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
		if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
			if (simContext->getCostInputs()->routineCareCostHIVNegativeStopAge == SimContext::NOT_APPL ||
					patient->getGeneralState()->ageMonths < simContext->getCostInputs()->routineCareCostHIVNegativeStopAge)
				incrementCostsMisc(simContext->getCostInputs()->routineCareCostHIVNegative[gender][costAgeCat], percentOfMonth);
		}
		else{
			incrementCostsMisc(simContext->getPedsCostInputs()->routineCareCostHIVNegative[gender][pedsCostAgeCat], percentOfMonth);
		}
	}
	else if (!patient->getMonitoringState()->isDetectedHIVPositive) {
		//add costs from two different areas
		SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		incrementCostsMisc(simContext->getHIVTestInputs()->monthCostHIVUndetected[cd4Strata], percentOfMonth);
		if (simContext->getCostInputs()->routineCareCostHIVPositiveUndetectedStopAge == SimContext::NOT_APPL ||
				patient->getGeneralState()->ageMonths < simContext->getCostInputs()->routineCareCostHIVPositiveUndetectedStopAge)
			incrementCostsMisc(simContext->getCostInputs()->routineCareCostHIVPositiveUndetected[gender][costAgeCat], percentOfMonth);

	}
	else {
		SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
		SimContext::ART_STATES artState = (patient->getARTState()->isOnART) ? SimContext::ART_ON_STATE : SimContext::ART_OFF_STATE;	
		if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
			incrementCostsRoutineCare(simContext->getCostInputs()->routineCareCostHIVPositive[artState][cd4Strata][gender][costAgeCat], percentOfMonth);
		}
		else{
			incrementCostsRoutineCare(simContext->getPedsCostInputs()->routineCareCostHIVPositive[artState][cd4Strata][gender][pedsCostAgeCat], percentOfMonth);
		}
	}

	/**Increment costs for those on PrEP */
    if(simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP && patient->getMonitoringState()->everPrEP){
		if(patient->getMonitoringState()->hasPrEP){
            SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
			// For now, we are incrementing the full cost in the month of death - may revisit later
			incrementCostsPrEP(simContext->getHIVTestInputs()->costPrEPMonthly[risk], 1.0);
		}	
		// For those who have ever had PrEP and died this month, accumulate their final PrEP costs by PrEP state
		if(!patient->isAlive())
			finalizePrEPCostsByState(patient->getMonitoringState()->isPrEPDropout);
    }

	/** Increment costs for ART treatments */
	if (patient->getARTState()->isOnART) {
		int artLineNum = patient->getARTState()->currRegimenNum;
		double cost = 0.0;
		if (patient->getGeneralState()->isAdolescent){
			int ayaAgeCat = getAgeCategoryAdolescent();
			cost = simContext->getAdolescentARTInputs(artLineNum)->costMonthly[ayaAgeCat];
		}
		else if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT) {
			cost = simContext->getARTInputs(artLineNum)->costMonthly;
		}
		else{
			cost = simContext->getPedsARTInputs(artLineNum)->costMonthly[patient->getPedsState()->ageCategoryPedsARTCost];
		}

		//always incur full cost on month of regimen start
		if (patient->getARTState()->monthOfCurrRegimenStart != patient->getGeneralState()->monthNum){
			/** - Adjust cost for non-responders by the proportion of monthly costs they incur */
			double responseFactor = patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_COST];
			double upperValue = cost;
			double lowerValue = cost * patient->getARTState()->propMthCostNonResponders;
			cost = lowerValue+responseFactor*(upperValue-lowerValue);
		}
		incrementCostsARTMonthly(artLineNum, cost);
	}


	/** Increment costs for OI prophylaxis treatments */
	if (patient->getProphState()->currTotalNumProphsOn > 0) {
		for (int i = 0; i < SimContext::OI_NUM; i++) {
			if (patient->getProphState()->isOnProph[i]) {
				int prophNum = patient->getProphState()->currProphNum[i];
				SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
				double cost;
				if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
					cost=simContext->getProphInputs(prophType,i,prophNum)->costMonthly;
				}
				else{
					cost=simContext->getPedsProphInputs(prophType,i,prophNum)->costMonthly;
				}
				incrementCostsProph((SimContext::OI_TYPE) i, prophNum, cost);
			}
		}
	}

	/** Increment costs for TB proph and treatments */
	if (patient->getTBState()->isOnProph) {
		int prophNum = patient->getTBState()->currProphNum;
		double cost = simContext->getTBInputs()->tbProphInputs[prophNum].costMonthly;
		incrementCostsTBProph(prophNum, cost);
	}

	if (patient->getTBState()->isOnTreatment) {
		int treatNum = patient->getTBState()->currTreatmentNum;
		int stage = 1;
		if((patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart + patient->getTBState()->previousTreatmentDuration) <= simContext->getTBInputs()->TBTreatments[treatNum].stage1Duration)
			stage = 0;
		double cost = simContext->getTBInputs()->TBTreatments[treatNum].costMonthly[stage];
		incrementCostsTBTreatment(cost, treatNum);
	}
	if (patient->getTBState()->isOnEmpiricTreatment) {
		int treatNum = patient->getTBState()->currEmpiricTreatmentNum;
		int stage = 1;
		if((patient->getGeneralState()->monthNum - patient->getTBState()->monthOfEmpiricTreatmentStart + patient->getTBState()->previousEmpiricTreatmentDuration) <= simContext->getTBInputs()->TBTreatments[treatNum].stage1Duration)
			stage = 0;
		double cost = simContext->getTBInputs()->TBTreatments[treatNum].costMonthly[stage];
		incrementCostsTBTreatment(cost, treatNum);
	}

	/** Increment costs of mortality in the month of death */
	if (!patient->isAlive()) {
		SimContext::DTH_CAUSES causeOfDeath = patient->getDiseaseState()->causeOfDeath;
		SimContext::ART_STATES artState = (patient->getARTState()->isOnART) ? SimContext::ART_ON_STATE : SimContext::ART_OFF_STATE;

		if (causeOfDeath < SimContext::OI_NUM) {
			if (patient->getMonitoringState()->clinicVisitType != SimContext::CLINIC_INITIAL) {
				if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
					incrementCostsDeath(simContext->getCostInputs()->deathCostTreated[costAgeCat][artState][causeOfDeath], 1.0);
				}
				else{
					incrementCostsDeath(simContext->getPedsCostInputs()->deathCostTreated[pedsCostAgeCat][artState][causeOfDeath], 1.0);
				}
			}
			else {
				if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
					incrementCostsDeath(simContext->getCostInputs()->deathCostUntreated[costAgeCat][artState][causeOfDeath], 1.0);
				}
				else{
					incrementCostsDeath(simContext->getPedsCostInputs()->deathCostUntreated[pedsCostAgeCat][artState][causeOfDeath], 1.0);
				}
			}
		}
		else if(causeOfDeath == SimContext::DTH_ACTIVE_TB){
			if(pedsCostAgeCat == SimContext::PEDS_COST_AGE_ADULT){
				incrementCostsDeath(simContext->getTBInputs()->costTBDeath, 1.0);
			}
			else{
				incrementCostsDeath(simContext->getTBInputs()->costTBDeathPeds[pedsCostAgeCat], 1.0);
			}
		}
		else if (causeOfDeath == SimContext::DTH_HIV) {
			if (patient->getMonitoringState()->isDetectedHIVPositive) {
				if (patient->getMonitoringState()->clinicVisitType != SimContext::CLINIC_INITIAL) {
					if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
						incrementCostsDeath(simContext->getCostInputs()->deathCostTreated[costAgeCat][artState][SimContext::DTH_HIV], 1.0);
					}
					else{
						incrementCostsDeath(simContext->getPedsCostInputs()->deathCostTreated[pedsCostAgeCat][artState][SimContext::DTH_HIV], 1.0);
					}
				}
				else {
					if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
						incrementCostsDeath(simContext->getCostInputs()->deathCostUntreated[costAgeCat][artState][SimContext::DTH_HIV], 1.0);
					}
					else{
						incrementCostsDeath(simContext->getPedsCostInputs()->deathCostUntreated[pedsCostAgeCat][artState][SimContext::DTH_HIV], 1.0);
					}
				}
			}
			else {
				double cost = simContext->getHIVTestInputs()->HIVDeathCostHIVUndetected;
				incrementCostsMisc(cost, 1.0);
			}
		}
		else if (causeOfDeath == SimContext::DTH_BKGD_MORT) {
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
				double cost = simContext->getHIVTestInputs()->deathCostHIVNegative;
				incrementCostsMisc(cost, 1.0);
			}
			// HIV+ detected
			else if (patient->getMonitoringState()->isDetectedHIVPositive) {
				if (patient->getMonitoringState()->clinicVisitType != SimContext::CLINIC_INITIAL) {
					if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
						incrementCostsDeath(simContext->getCostInputs()->deathCostTreated[costAgeCat][artState][SimContext::DTH_BKGD_MORT], 1.0);
					}
					else{
						incrementCostsDeath(simContext->getPedsCostInputs()->deathCostTreated[pedsCostAgeCat][artState][SimContext::DTH_BKGD_MORT], 1.0);
					}
				}
				else {
					if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
						incrementCostsDeath(simContext->getCostInputs()->deathCostUntreated[costAgeCat][artState][SimContext::DTH_BKGD_MORT], 1.0);
					}
					else{
						incrementCostsDeath(simContext->getPedsCostInputs()->deathCostUntreated[pedsCostAgeCat][artState][SimContext::DTH_BKGD_MORT], 1.0);
					}
				}
			}
			// HIV+ undetected
			else {
				double cost = simContext->getHIVTestInputs()->backgroundMortDeathCostHIVUndetected;
				incrementCostsMisc(cost, 1.0);
			}
		}
		else if ((causeOfDeath >= SimContext::DTH_CHRM_1) && (causeOfDeath < SimContext::DTH_CHRM_1 + SimContext::CHRM_NUM)) {
			double cost = simContext->getCHRMsInputs()->costDeathCHRMs[causeOfDeath - SimContext::DTH_CHRM_1];
			incrementCostsMisc(cost, 1.0);
		}
		/**  - ART toxicity death is incurred in the MortalityUpdater
			 - Other causes of death incur no additional costs */
	}
	/** Assess routine care QOL or QOL in the month of death */
	if (patient->isAlive()) {
		/** If alive, use QOL modifier with either acute OI, undetected HIV, or routine care levels */
		if (patient->getDiseaseState()->hasCurrTrueOI) {
			SimContext::OI_TYPE oiType = patient->getDiseaseState()->typeCurrTrueOI;
			accumulateQOLModifier(simContext->getQOLInputs()->acuteOIQOL[oiType]);
		}
		else if ((!patient->getMonitoringState()->isDetectedHIVPositive)&& !patient->getDiseaseState()->infectedHIVState==SimContext::HIV_INF_NEG) {
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			accumulateQOLModifier(simContext->getHIVTestInputs()->monthQOLHIVUndetected[cd4Strata]);
		}
		else if(!patient->getDiseaseState()->infectedHIVState==SimContext::HIV_INF_NEG){
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			double routineCareQOL = simContext->getQOLInputs()->routineCareQOL[cd4Strata][SimContext::HIST_EXT_N];
			// if they are within the OI history effect duration, use their OI history to determine the QOL modifier
			for (int i = 0; i < SimContext::OI_NUM; i++) {
				if ((patient->getGeneralState()->monthNum - patient->getDiseaseState()->lastMonthSevereOI[i]) <= simContext->getQOLInputs()->routineCareQOLSevereOIHistDuration[cd4Strata]){
					routineCareQOL = simContext->getQOLInputs()->routineCareQOL[cd4Strata][patient->getDiseaseState()->typeTrueOIHistory];
					break;
				}
			}
			accumulateQOLModifier(routineCareQOL);
		}
		if((patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM) || (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)){
			accumulateQOLModifier(simContext->getTBInputs()->QOLModActiveTB);
		}
		/** Accumulate PrEP QoL modifier if applicable */
		if(simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP && patient->getMonitoringState()->hasPrEP){
            SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
            accumulateQOLModifier(simContext->getHIVTestInputs()->PrEPQOL[risk]);
		}
	}
	else {
		/** If month of death, use QOL modifier for the cause of death */
		SimContext::DTH_CAUSES causeOfDeath = patient->getDiseaseState()->causeOfDeath;
		if ((causeOfDeath < SimContext::OI_NUM)) {
			accumulateQOLModifier(simContext->getQOLInputs()->deathBasicQOL[causeOfDeath]);
		}
		else if (causeOfDeath == SimContext::DTH_HIV) {
			if (!patient->getMonitoringState()->isDetectedHIVPositive) {
				accumulateQOLModifier(simContext->getHIVTestInputs()->HIVDeathQOLHIVUndetected);
			}
			else {
				accumulateQOLModifier(simContext->getQOLInputs()->deathBasicQOL[SimContext::DTH_HIV]);
			}
		}
		else if (causeOfDeath == SimContext::DTH_BKGD_MORT) {
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
				accumulateQOLModifier(simContext->getHIVTestInputs()->deathQOLHIVNegative);
			}
			else if (!patient->getMonitoringState()->isDetectedHIVPositive) {
				accumulateQOLModifier(simContext->getHIVTestInputs()->backgroundMortDeathQOLHIVUndetected);
			}
			else {
				accumulateQOLModifier(simContext->getQOLInputs()->deathBasicQOL[SimContext::DTH_BKGD_MORT]);
			}
		}
		else if(causeOfDeath == SimContext::DTH_ACTIVE_TB){
			accumulateQOLModifier(simContext->getTBInputs()->QOLModDeathActiveTB);
		}
		else if ((causeOfDeath >= SimContext::DTH_CHRM_1) && (causeOfDeath < SimContext::DTH_CHRM_1 + SimContext::CHRM_NUM)) {
			accumulateQOLModifier(simContext->getCHRMsInputs()->QOLModDeathCHRMs[causeOfDeath - SimContext::DTH_CHRM_1]);
		}
		/** Other causes of death incur no additional QOL modifiers */
	}
	/** now all the QOL modifiers have been applied, finalize the patient's QOL value */
	finalizeQOLValue();

	/** update statistics for the coststats file */
	updateCostStats(percentOfMonth);

	/** update Pediatric HIV exposure statistics regardless of whether the patient died this month */
	if(patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE && patient->getDiseaseState()->infectedPediatricsHIVState == SimContext::PEDS_HIV_NEG){
		updatePedsHIVExposureStats(patient->getPedsState()->maternalStatus);
	}
	/** Update additional state, statistics and tracing for regular month or month of death */
	if (patient->isAlive()) {
		/** Increment the patient, overall, and longitudinal survival stats */
		updatePatientSurvival(percentOfMonth);
		updateOverallSurvival(percentOfMonth);
		updateLongitSurvival();

		/** update the patients OI history with any new OIs and OI history logging stats */
		if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG) {
			setOIHistory();
			updateOIHistoryLogging();
			/** Update the ART suppression stats */
			updateARTEfficacyStats();
		}

		/** If tracing is enabled, print out either monthly status */
		if (patient->getGeneralState()->tracingEnabled) {
			if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG) {
				if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE) {
					tracer->printTrace(1, "  %d upd: true CD4 perc %1.3f %s, true HVL %s;\n", patient->getGeneralState()->monthNum,
						patient->getDiseaseState()->currTrueCD4Percentage, SimContext::CD4_STRATA_STRS[patient->getDiseaseState()->currTrueCD4Strata],
						SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->currTrueHVLStrata]);
				}
				else {
					tracer->printTrace(1, "  %d upd: true CD4 %1.0f %s, true HVL %s;\n", patient->getGeneralState()->monthNum,
						patient->getDiseaseState()->currTrueCD4, SimContext::CD4_STRATA_STRS[patient->getDiseaseState()->currTrueCD4Strata],
						SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->currTrueHVLStrata]);
				}
			}

			tracer->printTrace(1, "  %d mth: ", patient->getGeneralState()->monthNum);
			if (simContext->getTBInputs()->enableTB)
				tracer->printTrace(1, "TB Symptoms: %s, ", patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]?"Yes":"No");

			tracer->printTrace(1, "LM %1.2lf QA %1.2lf, $ %1.0lf;\n",
				patient->getGeneralState()->LMsDiscounted, patient->getGeneralState()->qualityAdjustLMsDiscounted,
				patient->getGeneralState()->costsDiscounted);

		}

		/** Increment the simulation month number and patient age */
		incrementMonth();
		/** Increment the discount factor */
		incrementDiscountFactor(simContext->getRunSpecsInputs()->discountFactor);
		for(int i = 0; i < simContext->NUM_DISCOUNT_RATES; i++){
			incrementMultDiscountFactor(simContext->getRunSpecsInputs()->multDiscountRatesCost[i], simContext->getRunSpecsInputs()->multDiscountRatesBenefit[i], i);
		}
	}
	else {
		/** Increment the patient and overall survival stats */
		updatePatientSurvival(percentOfMonth);
		updateOverallSurvival(percentOfMonth);

		/** Update population statistics and add the patient summary if death occurred */
		updatePopulationStats();
		addPatientSummary();

		/** If tracing is enabled, print out death tracing */
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d DEATH %s;\n", patient->getGeneralState()->monthNum,
				SimContext::DTH_CAUSES_STRS[patient->getDiseaseState()->causeOfDeath]);
			tracer->printTrace(1, "  LMs %1.2lf QA %1.2lf $ %1.0lf ;\n",
				patient->getGeneralState()->LMsDiscounted,
				patient->getGeneralState()->qualityAdjustLMsDiscounted,
				patient->getGeneralState()->costsDiscounted);
			tracer->printTrace(1, "  END PATIENT\n\n");
		}
	}

} /* end performMonthlyUpdates */

