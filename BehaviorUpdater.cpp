#include "include.h"

/** \brief Constructor takes in the patient object */
BehaviorUpdater::BehaviorUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
BehaviorUpdater::~BehaviorUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void BehaviorUpdater::performInitialUpdates() {
	/** Call the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
	if (simContext->getLTFUInputs()->useLTFU)
		setPreARTResponseBase(CepacUtil::getRandomGaussian(simContext->getLTFUInputs()->propRespondLTFUPreARTLogitMean, simContext->getLTFUInputs()->propRespondLTFUPreARTLogitStdDev, 30005, patient));
	setAdherenceInterventionState(false);

	int nextIndex = SimContext::NOT_APPL;
	for (int i = 0; i < SimContext::HET_INTV_NUM_PERIODS; i++){
		if (simContext->getHeterogeneityInputs()->useIntervention[i]){
			nextIndex = i;
			break;
		}
	}
	setNextAdherenceIntervention(nextIndex);

} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void BehaviorUpdater::performMonthlyUpdates() {

	/** check to see if should stop adherence intervention */
	if (patient->getGeneralState()->isOnAdherenceIntervention){
		if (patient->getGeneralState()->monthToEndAdherenceIntervention <= patient->getGeneralState()->monthNum){
			//end intervention
			setAdherenceInterventionState(false);

			//check to see if patient is on ART, and if so reset the ART response to remove the adjustment from the intervention
			if (patient->getARTState()->isOnART){
				double onARTLogit;
				//Check to see if ART regimen-specific logit adjustment has a duration; if they exceeded the duration the regimen-specific logit adjustment will no longer be added - these are separate from the intervention's own extra adjustment and its duration 
				if (patient->getARTState()->enableDurationCurrRegimenResponseIncrement &&
						patient->getGeneralState()->monthNum > patient->getARTState()->monthToStopCurrRegimenResponseIncrement)
					onARTLogit = patient->getARTState()->responseLogitCurrRegimenPreIncrement;
				else
					onARTLogit = patient->getARTState()->responseLogitCurrRegimenBase;

				setCurrARTResponse(onARTLogit);
			}
			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d PATIENT STOPPED ADHERENCE INTERVENTION;\n", patient->getGeneralState()->monthNum);
			}
		}
	} //end checking whether to stop adherence intervention

	/** check to see if should start adherence intervention*/
	if (!patient->getGeneralState()->isOnAdherenceIntervention && patient->getGeneralState()->nextAdherenceIntervention != SimContext::NOT_APPL){
		int intvIndex = patient->getGeneralState()->nextAdherenceIntervention;
		/** Roll for duration of event */
		double duration = CepacUtil::getRandomGaussian(simContext->getHeterogeneityInputs()->interventionDurationMean[intvIndex],
				simContext->getHeterogeneityInputs()->interventionDurationSD[intvIndex],30000, patient);
		duration = (int)(duration + 0.5);
		setAdherenceInterventionState(true, intvIndex, duration);

		/** roll for coefficient adjustment */
		double adjustment = -1;
		double responseMean = simContext->getHeterogeneityInputs()->interventionAdjustmentMean[intvIndex];
		double responseSD = simContext->getHeterogeneityInputs()->interventionAdjustmentSD[intvIndex];
		SimContext::HET_ART_LOGIT_DISTRIBUTION responseDist = simContext->getHeterogeneityInputs()->interventionAdjustmentDistribution[intvIndex];
		if (responseDist == SimContext::HET_ART_LOGIT_DISTRIBUTION_NORMAL)
			adjustment = CepacUtil::getRandomGaussian(responseMean, responseSD, 30100, patient);
		else if (responseDist == SimContext::HET_ART_LOGIT_DISTRIBUTION_TRUNC_NORMAL){
			while (adjustment < 0)
				adjustment = CepacUtil::getRandomGaussian(responseMean, responseSD, 30120, patient);
		}
		else{
			while (adjustment < 0)
				adjustment = CepacUtil::getRandomGaussian(responseMean, responseSD, 30125, patient);
			adjustment = adjustment * adjustment;
		}

		setAdherenceInterventionResponseAdjustment(adjustment);

		if (patient->getARTState()->isOnART){
			double onARTLogit;
			//Check to see if ART regimen-specific logit adjustment has a duration; if they exceeded that duration the regimen-specific logit adjustment will no longer be added, but the intervention adjustment will be added 
			if (patient->getARTState()->enableDurationCurrRegimenResponseIncrement &&
					patient->getGeneralState()->monthNum > patient->getARTState()->monthToStopCurrRegimenResponseIncrement)
				onARTLogit = patient->getARTState()->responseLogitCurrRegimenPreIncrement;
			else
				onARTLogit = patient->getARTState()->responseLogitCurrRegimenBase;

			setCurrARTResponse(onARTLogit + adjustment);
		}

		//add cost of intervention startup
		incrementCostsInterventionInit(simContext->getHeterogeneityInputs()->interventionCostInit[intvIndex]);

		//Set next adherence intervention
		int nextIndex = SimContext::NOT_APPL;
		for (int i = intvIndex+1; i < SimContext::HET_INTV_NUM_PERIODS; i++){
			if (simContext->getHeterogeneityInputs()->useIntervention[i]){
				nextIndex = i;
				break;
			}
		}
		setNextAdherenceIntervention(nextIndex);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d PATIENT STARTED ADHERENCE INTERVENTION PERIOD %d; Coefficient adjustment:%lf, $ %1.0lf;\n", patient->getGeneralState()->monthNum, intvIndex, adjustment, patient->getGeneralState()->costsDiscounted);
		}
	} //end checking whether to start adherence intervention

	//add monthly costs for adherence intervention
	if (patient->getGeneralState()->isOnAdherenceIntervention){
		incrementCostsInterventionMonthly(simContext->getHeterogeneityInputs()->interventionCostMonthly[patient->getGeneralState()->currAdherenceIntervention]*
				simContext->getLTFUInputs()->propInterventionCost[patient->getMonitoringState()->careState]);
	}

	/** Check if this is the month to end regimen specific logit adjustment has a duration; adjustments from interventions will still be made to their response on the current regimen */
	if (patient->getARTState()->isOnART && patient->getARTState()->enableDurationCurrRegimenResponseIncrement && patient->getGeneralState()->monthNum == patient->getARTState()->monthToStopCurrRegimenResponseIncrement){
		double interventionAdjust = 0;
		if (patient->getGeneralState()->isOnAdherenceIntervention)
			interventionAdjust = patient->getGeneralState()->responseLogitAdherenceInterventionIncrement;
		setCurrARTResponse(patient->getARTState()->responseLogitCurrRegimenPreIncrement + interventionAdjust);
	}

	/** Return if LTFU is not enabled */
	if (!simContext->getLTFUInputs()->useLTFU)
		return;

	/** If patient is LTFU, roll for return to care */
	if (patient->getMonitoringState()->currLTFUState == SimContext::LTFU_STATE_LOST) {
		/** Return if we have not reached min months and there is not an acute OI */
		int monthsLost = patient->getGeneralState()->monthNum - patient->getMonitoringState()->monthOfLTFUStateChange;
		if ((monthsLost < simContext->getLTFUInputs()->minMonthsRemainLost) &&
			!patient->getDiseaseState()->hasCurrTrueOI)
			return;

		/** Calculate the probability of return to care */
		double logitRTC = simContext->getLTFUInputs()->regressionCoefficientsRTC[SimContext::RTC_BACKGROUND];
		if ((patient->getPedsState()->ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE) && (patient->getDiseaseState()->currTrueCD4 < simContext->getLTFUInputs()->CD4ThresholdRTC))
			logitRTC += simContext->getLTFUInputs()->regressionCoefficientsRTC[SimContext::RTC_CD4];
		// logit adjustment in month of acute OI
		if (patient->getDiseaseState()->hasCurrTrueOI){
			if (simContext->getLTFUInputs()->severeOIsRTC[patient->getDiseaseState()->typeCurrTrueOI])
				logitRTC += simContext->getLTFUInputs()->regressionCoefficientsRTC[SimContext::RTC_ACUTESEVEREOI];
			else
				logitRTC += simContext->getLTFUInputs()->regressionCoefficientsRTC[SimContext::RTC_ACUTEMILDOI];
		}
		// logit adjustment in the month after a TB positive diagnosis
		if(patient->getTBState()->monthOfTBPosDiagnosis != SimContext::NOT_APPL && patient->getGeneralState()->monthNum == patient->getTBState()->monthOfTBPosDiagnosis + 1){
			logitRTC += simContext->getLTFUInputs()->regressionCoefficientsRTC[SimContext::RTC_TBPOS];
		}
		double probRTC = pow(1 + exp(0 - logitRTC), -1);

		/** Roll for return to care and update state if it occurs */
		double randNum = CepacUtil::getRandomDouble(30020, patient);
		if (randNum < probRTC) {
			setCurrLTFUState(SimContext::LTFU_STATE_RETURNED);
			/** Set TB RTC if TB module and TB LTFU are enabled and patient is currently TB LTFU from integrated HIV/TB clinic*/
			if ((patient->getTBState()->careState == SimContext::TB_CARE_LTFU) && simContext->getTBInputs()->isIntegrated) {
				setTBRTC();
			}	
			/** If RTC, set the month of next clinic visit to the current month */
			scheduleRegularClinicVisit(true, patient->getGeneralState()->monthNum);

			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d PATIENT RETURNED TO CARE;\n", patient->getGeneralState()->monthNum);
			}

			//Check to see if should resume intervention if active
			if (patient->getGeneralState()->isOnAdherenceIntervention){
				//roll to end intervention
				randNum = CepacUtil::getRandomDouble(30025, patient);
				if (randNum < simContext->getLTFUInputs()->probResumeInterventionRTC){
					//resume intervention

					//add cost of intervention startup
					incrementCostsInterventionInit(simContext->getLTFUInputs()->costResumeInterventionRTC);

					// Output tracing if enabled
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d PATIENT RESUMED ADHERENCE INTERVENTION; $ %1.0lf;\n", patient->getGeneralState()->monthNum,  patient->getGeneralState()->costsDiscounted);
					}
				}
				else {
					//end intervention
					setAdherenceInterventionState(false);

					// Output tracing if enabled
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d PATIENT STOPPED ADHERENCE INTERVENTION;\n", patient->getGeneralState()->monthNum);
					}
				}
			}
		}

		return;
	} //end if patient is LTFU

	/** If not already LTFU, calculate the probability of LTFU using user specified logits*/
	if(!patient->getMonitoringState()->hadPrevClinicVisit)
		return;

	bool isOnART = false;
	if (patient->getARTState()->isOnART)
		isOnART = true;
	double logitLTFU = 0.0;
	if (isOnART) {
		logitLTFU = patient->getARTState()->responseLogitCurrRegimen;
	}
	else {
		logitLTFU = patient->getGeneralState()->responseBaselineLogit;
		int ageCat = patient->getGeneralState()->ageCategoryHeterogeneity;
		SimContext::PEDS_AGE_CAT pedsAgeCat = patient->getPedsState()->ageCategoryPediatrics;

		if(pedsAgeCat==SimContext::PEDS_AGE_ADULT){
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondAge[ageCat];
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondCD4[cd4Strata];
		}
		else if(pedsAgeCat==SimContext::PEDS_AGE_LATE){
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondAgeLate;
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondCD4[cd4Strata];
		}
		else{
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondAgeEarly;
		}

		if (patient->getGeneralState()->gender == SimContext::GENDER_FEMALE)
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondFemale;
		if (patient->getDiseaseState()->typeTrueOIHistory != SimContext::HIST_EXT_N)
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondHistoryOIs;
		if (patient->getARTState()->hadPrevToxicity)
			logitLTFU += simContext->getHeterogeneityInputs()->propRespondPriorARTToxicity;
		//Risk factors can also affect CHRM prevalence and incidence and have mortality risks associated with them. See CHRMsUpdater for their incidence, which occurs after CHRM incidence
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
			if (patient->getGeneralState()->hasRiskFactor[i])
				logitLTFU += simContext->getHeterogeneityInputs()->propRespondRiskFactor[i];
		}
		logitLTFU+=patient->getGeneralState()->responseLogitPreARTBase;
	} //end if patient is not on ART

	if (patient->getGeneralState()->isOnAdherenceIntervention)
		logitLTFU += patient->getGeneralState()->responseLogitAdherenceInterventionIncrement;

	//calculate prob of LTFU from outcome function
	double propRespondLTFU = pow(1 + exp(0 - logitLTFU), -1);
	double L1, L2;
	if (simContext->getLTFUInputs()->useInterventionLTFU){
		if (patient->getGeneralState()->isOnAdherenceIntervention){

			L1 = simContext->getLTFUInputs()->responseThresholdPeriodLTFU[patient->getGeneralState()->currAdherenceIntervention][0];
			L2 = simContext->getLTFUInputs()->responseThresholdPeriodLTFU[patient->getGeneralState()->currAdherenceIntervention][1];
		}
		else{
			L1 = simContext->getLTFUInputs()->responseThresholdLTFUOffIntervention[0];
			L2 = simContext->getLTFUInputs()->responseThresholdLTFUOffIntervention[1];
		}
	}
	else{
		L1 = simContext->getLTFUInputs()->responseThresholdLTFU[0];
		L2 = simContext->getLTFUInputs()->responseThresholdLTFU[1];
	}
	double respFactor;

	if (propRespondLTFU > L2)
		respFactor = 1.0;
	else if (propRespondLTFU > L1)
		respFactor = (propRespondLTFU - L1) / (L2 - L1);
	else
		respFactor = 0.0;

	double lowerValue;
	double upperValue;

	if (simContext->getLTFUInputs()->useInterventionLTFU){
		if(patient->getGeneralState()->isOnAdherenceIntervention){
			lowerValue = simContext->getLTFUInputs()->responseValuePeriodLTFU[patient->getGeneralState()->currAdherenceIntervention][0];
			upperValue = simContext->getLTFUInputs()->responseValuePeriodLTFU[patient->getGeneralState()->currAdherenceIntervention][1];
		}
		else{
			lowerValue = simContext->getLTFUInputs()->responseValueLTFUOffIntervention[0];
			upperValue = simContext->getLTFUInputs()->responseValueLTFUOffIntervention[1];
		}
	}
	else{
		lowerValue = simContext->getLTFUInputs()->responseValueLTFU[0];
		upperValue = simContext->getLTFUInputs()->responseValueLTFU[1];
	}
	double probLTFU = lowerValue+respFactor*(upperValue-lowerValue);

	/** Roll for LTFU and update state if so */
	double randNum = CepacUtil::getRandomDouble(30030, patient);
	if (randNum < probLTFU) {
		setCurrLTFUState(SimContext::LTFU_STATE_LOST);

		/** Check whether changes are needed to the patient's TB care state at an integrated HIV/TB clinic */
		if (simContext->getTBInputs()->enableTB && simContext->getTBInputs()->useTBLTFU && simContext->getTBInputs()->isIntegrated){
			if (patient->getTBState()->careState == SimContext::TB_CARE_IN_CARE){
				setTBLTFU();
			}
			// if patient is not in TB care or in TB care but still on proph roll for proph stop
			if(patient->getTBState()->isOnProph){
				randNum = CepacUtil::getRandomDouble(30031, patient);
				if(randNum < simContext->getTBInputs()->probStopTBProphAtHIVLTFU){
					stopCurrTBProph();
					// check first whether they are now ineligible to resume TB proph due to major toxicity
					if(patient->getTBState()->hasMajorProphToxicity && !simContext->getTBInputs()->moveToNextProphAfterTox){
						setNextTBProph(false, SimContext::NOT_APPL, SimContext::NOT_APPL);
					}
					//if they are eligible to resume TB proph, check whether to go on the same line upon RTC or a subsequent one if available
					else{
						bool hasNext = false;
						int currProphIndex = patient->getTBState()->currProphIndex;
						int nextProphIndex = SimContext::NOT_APPL;
						int nextProphNum = SimContext::NOT_APPL;
							//if they don't have toxicity and have not exceeded the maximum restarts they can resume this line upon return to HIV care
							if(!patient->getTBState()->hasMajorProphToxicity && (patient->getTBState()->numProphStarts[currProphIndex] < simContext->getTBInputs()->maxRestarts[currProphIndex]+1)){
								hasNext = true;
								nextProphIndex = currProphIndex;
								nextProphNum = patient->getTBState()->currProphNum;	
							}
							else {
								for (int i = currProphIndex + 1; i < SimContext::TB_NUM_PROPHS; i++) {
								if (simContext->getTBInputs()->prophOrder[i] != SimContext::NOT_APPL) {
									if(patient->getTBState()->numProphStarts[i] < simContext->getTBInputs()->maxRestarts[i]+1){
										hasNext = true;
										nextProphIndex = i;
										nextProphNum = simContext->getTBInputs()->prophOrder[i];
										break;
									}
								}
							}
						}
						setNextTBProph(hasNext, nextProphIndex, nextProphNum);
					}	
				}
			}
			// if they were scheduled to start proph later this month or in a subsequent month then unschedule it
			else if(patient->getTBState()->isScheduledForProph && !simContext->getTBInputs()->allowTBProphStartWhileHIVLTFU && (patient->getGeneralState()->monthNum <= patient->getTBState()->monthOfProphStart)){
				unscheduleNextTBProph();
			}	
		}
		/** If newly LTFU, stop the current ART and determine if previous ART regimen should be restarted at return
			instead of proceeding to the next one (default) */
		if (patient->getARTState()->isOnART) {
			if (patient->getARTState()->currSTIState != SimContext::STI_STATE_NONE) {
				setCurrSTIState(SimContext::STI_STATE_NONE);
			}
			stopCurrARTRegimen(SimContext::ART_STOP_LTFU);
			/** Set the target HVL back to the setpoint */
			setTargetHVLStrata(patient->getDiseaseState()->setpointHVLStrata);

			/** Determine if patient should go back to current regimen; default to yes if there is no next regimen */
			if (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_FAILURE && patient->getARTState()->hasNextRegimenAvailable) {
				if (patient->getARTState()->hasObservedFailure) {
					int monthsFail = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfObservedFailure;
					if (monthsFail < simContext->getLTFUInputs()->maxMonthsAfterObservedFailureToRestartRegimen) {
						setNextARTRegimen(true, patient->getARTState()->prevRegimenNum);
					}
				}
				else {
					randNum = CepacUtil::getRandomDouble(30040, patient);
					if (randNum < simContext->getLTFUInputs()->probRestartRegimenWithoutObsvervedFailure) {
						setNextARTRegimen(true, patient->getARTState()->prevRegimenNum);
					}
				}
			}
			else {
				setNextARTRegimen(true, patient->getARTState()->prevRegimenNum);
			}

			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d TAKEN OFF ART %d by %s;\n", patient->getGeneralState()->monthNum,
					patient->getARTState()->prevRegimenNum + 1,
					SimContext::ART_STOP_TYPE_STRS[patient->getARTState()->typeCurrStop]);
			}
		}

		/** Roll for and stop the current OI prophs */
		randNum = CepacUtil::getRandomDouble(30050, patient);
		if (randNum >= simContext->getLTFUInputs()->probRemainOnOIProph) {
			for (int i = 0; i < SimContext::OI_NUM; i++) {
				if (patient->getProphState()->isOnProph[i]) {
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d STOP %s PROPH %d for OI %s;\n", patient->getGeneralState()->monthNum,
							SimContext::PROPH_TYPE_STRS[patient->getProphState()->currProphType[i]],
							patient->getProphState()->currProphNum[i] + 1, SimContext::OI_STRS[i]);
					}
					stopCurrProph((SimContext::OI_TYPE) i);
				}
			}
		}

		/** Stop all subsequent clinic visits until patient returns to care */
		scheduleRegularClinicVisit(false);
		scheduleEmergencyClinicVisit(SimContext::EMERGENCY_NONE);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d PATIENT LOST TO FOLLOW UP;\n", patient->getGeneralState()->monthNum);
		}
	}
} /* end performMonthlyUpdates */
