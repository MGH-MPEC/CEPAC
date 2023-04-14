#include "include.h"

/** \brief Constructor takes in the patient object */
TBDiseaseUpdater::TBDiseaseUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
TBDiseaseUpdater::~TBDiseaseUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void TBDiseaseUpdater::performInitialUpdates() {
	/** First calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	if (!simContext->getTBInputs()->enableTB){
		setTBDiseaseState(SimContext::TB_STATE_UNINFECTED, false, SimContext::TB_INFECT_PREVALENT, SimContext::TB_STATE_UNINFECTED);
		return;
	}

	/** Set the initial TB disease state */
	SimContext::TB_STATE tbState = SimContext::TB_STATE_UNINFECTED;
	SimContext::CD4_STRATA cd4Strata;
	if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG)
		cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;

	double randNum = CepacUtil::getRandomDouble(140010, patient);
	double distProb = 0;
	for (int i = 0; i < SimContext::TB_NUM_STATES; i++) {
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
			distProb = simContext->getTBInputs()->distributionTBStateAtEntryHIVNeg[i];
		}
		else{
			distProb = simContext->getTBInputs()->distributionTBStateAtEntryHIVPos[cd4Strata][i];
		}

		if ((distProb > 0) &&
			(randNum < distProb)) {
				tbState = (SimContext::TB_STATE) i;
				break;
		}
		randNum -= distProb;
	}

	/** Draw the TB Strain */
	if (tbState != SimContext::TB_STATE_UNINFECTED) {
		// Set the initial TB resistance strain
		SimContext::TB_STRAIN tbStrain = SimContext::TB_STRAIN_DS;
		randNum = CepacUtil::getRandomDouble(140020, patient);
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++) {
			if ((simContext->getTBInputs()->distributionTBStrainAtEntry[j] > 0) &&
				(randNum < simContext->getTBInputs()->distributionTBStrainAtEntry[j])) {
					tbStrain = (SimContext::TB_STRAIN) j;
					break;
			}
			randNum -= simContext->getTBInputs()->distributionTBStrainAtEntry[j];
		}
		setTBResistanceStrain(tbStrain);

	}

	setTBDiseaseState(tbState, tbState != SimContext::TB_STATE_UNINFECTED, SimContext::TB_INFECT_PREVALENT, SimContext::TB_STATE_UNINFECTED);
	setTBSelfCure(false);

	/** Draw for the three tracker variables */
	SimContext::TB_TRACKER tbTracker;

	for (int i = 0; i < SimContext::TB_NUM_TRACKER; i++){
		randNum = CepacUtil::getRandomDouble(140025, patient);
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
			distProb = simContext->getTBInputs()->distributionTBTrackerAtEntryHIVNeg[i][patient->getTBState()->currTrueTBDiseaseState];
		}
		else{
			distProb = simContext->getTBInputs()->distributionTBTrackerAtEntryHIVPos[cd4Strata][i][patient->getTBState()->currTrueTBDiseaseState];
		}
		if (randNum < distProb)
			setTBTracker((SimContext::TB_TRACKER) i, true);
		else
			setTBTracker((SimContext::TB_TRACKER) i, false);
	}

	/** Add the initial TB state to the runStats for "at entry" */
	countInitialTBState();

} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month
 *
 * Depending on Patient's current true TB disease state, call one of the following
 * 	- TBDiseaseUpdater::performUninfectedTBUpdates()
 *  - TBDiseaseUpdater::performLatentTBUpdates()
 *  - TBDiseaseUpdater::performActivePulmTBUpdates()
 *  - TBDiseaseUpdater::performActiveExtraPulmTBUpdates()
 *  - TBDiseaseUpdater::performTreatDefaultTBUpdates()
 *  - TBDiseaseUpdater::performPrevTreatTBUpdates()
 **/
void TBDiseaseUpdater::performMonthlyUpdates() {
	if (!simContext->getTBInputs()->enableTB)
		return;

	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_UNINFECTED)
		performUninfectedTBUpdates();
	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_PREV_TREATED)
		performPrevTreatTBUpdates();
	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_TREAT_DEFAULT)
		performTreatDefaultTBUpdates();
	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_LATENT)
		performLatentTBUpdates();
	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM)
		performActivePulmTBUpdates();
	if (patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
		performActiveExtraPulmTBUpdates();

	if(patient->getGeneralState()->monthNum != 0)
		rollForTBSymptoms();
	//Add mortality risk for death from active TB, modified by time on successful or failed treatment where applicable
	SimContext::TB_STATE currTBState = patient->getTBState()->currTrueTBDiseaseState;
	if( currTBState == SimContext::TB_STATE_ACTIVE_PULM || currTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM){
		double deathRateRatioTB = 1.0;
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
			if (currTBState == SimContext::TB_STATE_ACTIVE_PULM)		
				deathRateRatioTB = simContext->getTBInputs()->TBDeathRateRatioActivePulmHIVneg;
			else
				deathRateRatioTB = simContext->getTBInputs()->TBDeathRateRatioExtraPulmHIVneg;
		}
		// HIV positive patients
		else{
			if (currTBState == SimContext::TB_STATE_ACTIVE_PULM)
				deathRateRatioTB = simContext->getTBInputs()->TBDeathRateRatioActivePulmHIVPos[patient->getDiseaseState()->currTrueCD4Strata];
			else
				deathRateRatioTB = simContext->getTBInputs()->TBDeathRateRatioExtraPulmHIVPos[patient->getDiseaseState()->currTrueCD4Strata];
		}
		// Modify by time on treatment and treatment outcome	
		int timeOnTreatment = SimContext::NOT_APPL;
		bool treatmentSuccess = false;
		// Check if patient recently stopped treatment due to LTFU while in an active state and still has protection from mortality
		if(patient->getTBState()->hasIncompleteTreatment){
			if(patient->getGeneralState()->monthNum <= patient->getTBState()->monthOfMortEfficacyStop){
				timeOnTreatment = patient->getTBState()->previousTreatmentDuration;
				if(patient->getTBState()->treatmentSuccess)
					treatmentSuccess = true;
			}		
		}
		// Check if currently on treatment
		else{
			if (patient->getTBState()->isOnTreatment){
				timeOnTreatment = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart;
				if(patient->getTBState()->treatmentSuccess)
					treatmentSuccess = true;
			}
			else if(patient->getTBState()->isOnEmpiricTreatment){
				timeOnTreatment = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfEmpiricTreatmentStart;
				if(patient->getTBState()->empiricTreatmentSuccess)
					treatmentSuccess = true; 
			}
		}	
		if(timeOnTreatment != SimContext::NOT_APPL){
			double deathRateRatioTx = 1.0;
			// Apply TB treatment death rate ratios based on number of months of treatment completed and treatment outcome
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
				if(treatmentSuccess){
					deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentSuccessHIVNeg[2];
					for (int i = 0; i < 2; i++) {
						if (timeOnTreatment <= simContext->getTBInputs()->TBDeathRateRatioTxSuccessBounds[i]) {
							deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentSuccessHIVNeg[i];;
							break;
						}	
					}
				}	
				else{
					deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentFailureHIVNeg[2];
					for (int i = 0; i < 2; i++) {
						if (timeOnTreatment <= simContext->getTBInputs()->TBDeathRateRatioTxFailureBounds[i]) {
							deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentFailureHIVNeg[i];;
							break;
						}	
					}
				}
			}
			// HIV positive
			else{
				if(treatmentSuccess){
					deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentSuccessHIVPos[2][patient->getDiseaseState()->currTrueCD4Strata];
					for (int i = 0; i < 2; i++) {
						if (timeOnTreatment <= simContext->getTBInputs()->TBDeathRateRatioTxSuccessBounds[i]) {
							deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentSuccessHIVPos[i][patient->getDiseaseState()->currTrueCD4Strata];;
							break;
						}	
					}
				}	
				else{
					deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentFailureHIVPos[2][patient->getDiseaseState()->currTrueCD4Strata];
					for (int i = 0; i < 2; i++) {
						if (timeOnTreatment <= simContext->getTBInputs()->TBDeathRateRatioTxFailureBounds[i]) {
							deathRateRatioTx = simContext->getTBInputs()->TBDeathRateRatioTreatmentFailureHIVPos[i][patient->getDiseaseState()->currTrueCD4Strata];;
							break;
						}	
					}
				}
			}
			deathRateRatioTB *= deathRateRatioTx;
		}
		
		// Modify by calendar month multiplier for the time period if enabled
		if(simContext->getTBInputs()->natHistMultType == SimContext::TB_MULT_MORTALITY){
			int period = 0;
			for (int i = 1; i >= 0; i--) {
				if (patient->getGeneralState()->monthNum > simContext->getTBInputs()->natHistMultTimeBounds[i]) {
					period = i+1;
					break;
				}
			}	
			deathRateRatioTB *= simContext->getTBInputs()->natHistMultTime[period];
		}
		if (deathRateRatioTB > 1.0) {
			addMortalityRisk(SimContext::DTH_ACTIVE_TB, deathRateRatioTB);
		}	
	}
}
 /* end performMonthlyUpdates */			

/** \brief performUninfectedTBUpdates determines if a first TB infection occurs from the no history state*/
void TBDiseaseUpdater::performUninfectedTBUpdates() {
	//Roll for TB infection, or reinfection if they previously had Latent TB which was treated with prophylaxis before it activated
	rollForInfection(SimContext::TB_STATE_UNINFECTED);
} /* end performUninfectedTBUpdates */

/** \brief performLatentTBUpdates determines if TB reactivates or a reinfection occurs from the latent state */
void TBDiseaseUpdater::performLatentTBUpdates() {
	//Roll for TB reinfection
	rollForInfection(SimContext::TB_STATE_LATENT);
	//If a patient with Latent TB gets reinfected, their TB state will still be Latent, but they may have a different TB drug resistance strain. They can be reinfected and have their TB activated in the same month

	const SimContext::TBInputs *tbInputs = simContext->getTBInputs();

	// Calculate probability of activation
	double probActivate = 0;
	SimContext::CD4_STRATA cd4Strata;
	double fCD4 = 0;
	int stage = 0;
	int timeSinceInfection = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTBInfection;
	if (timeSinceInfection > tbInputs->probActivateMthThreshold)
		stage = 1;
	if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		probActivate = tbInputs->probActivateHIVPos[stage][cd4Strata];
	}
	else
		probActivate = tbInputs->probActivateHIVNeg[stage];

	// Modify by calendar multplier if enabled 
	if(simContext->getTBInputs()->natHistMultType == SimContext::TB_MULT_ACTIVATION){
		int period = 0;
		for (int i = 1; i >= 0; i--) {
			if (patient->getGeneralState()->monthNum > simContext->getTBInputs()->natHistMultTimeBounds[i]) {
				period = i+1;
				break;
			}
		}
		probActivate = CepacUtil::probRateMultiply(probActivate, simContext->getTBInputs()->natHistMultTime[period]);
	}	

	// Modify prob by proph efficacy if on TB proph or recently completed a full course
	SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
	if (patient->getTBState()->isOnProph){
		double efficacy;
		int prophNum = patient->getTBState()->currProphNum;
		if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			efficacy = tbInputs->tbProphInputs[prophNum].efficacyActivationHIVNeg[tbStrain][0];
		else
			efficacy = tbInputs->tbProphInputs[prophNum].efficacyActivationHIVPos[tbStrain][cd4Strata][0];

		probActivate = CepacUtil::probRateMultiply(probActivate, 1 - efficacy);
	}
	else if (patient->getTBState()->hasCompletedProph){
		int recentProphNum = patient->getTBState()->mostRecentProphNum;
		int effHorizon = tbInputs->tbProphInputs[recentProphNum].monthsOfEfficacyActivation[tbStrain];
		int decayPeriod = tbInputs->tbProphInputs[recentProphNum].decayPeriodActivation[tbStrain];

	 	if(patient->getGeneralState()->monthNum <= patient->getTBState()->monthOfProphStop + effHorizon + decayPeriod ){
			double efficacy;
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyActivationHIVNeg[tbStrain][1];
			else
				efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyActivationHIVPos[tbStrain][cd4Strata][1];

			if (patient->getGeneralState()->monthNum > patient->getTBState()->monthOfProphStop + effHorizon){
				int monthsSinceDecay = patient->getGeneralState()->monthNum - (patient->getTBState()->monthOfProphStop + effHorizon);
				double propDecay = (double) monthsSinceDecay / decayPeriod;
				efficacy *= (1-propDecay);
			}
			probActivate = CepacUtil::probRateMultiply(probActivate, 1 - efficacy);
		}
	}

	// Modify prob by treat efficacy if on TB treat or recently stopped
	int effectiveTreatNum = SimContext::NOT_APPL;
	if (patient->getTBState()->isOnTreatment || patient->getTBState()->isOnEmpiricTreatment){
		// mostRecentTreatNum is the same as the current TB treatment number, but without the flag indicating whether the treatment is empiric 
		effectiveTreatNum = patient->getTBState()->mostRecentTreatNum;
	}	
	else if (patient->getTBState()->hasStoppedTreatmentOrEmpiric){
		int recentTreatNum = patient->getTBState()->mostRecentTreatNum;
		if(patient->getGeneralState()->monthNum <=
			patient->getTBState()->monthOfTreatmentOrEmpiricStop+
			tbInputs->TBTreatments[recentTreatNum].monthsOfEfficacyActivation[tbStrain]){
				effectiveTreatNum = recentTreatNum;
		}	
	}
	else if(patient->getTBState()->observedHistActiveTBAtEntry){
		int recentTreatNum = patient->getTBState()->mostRecentTreatNum;
		if(patient->getGeneralState()->monthNum + patient->getTBState()->monthsSinceInitTBTreatStopAtEntry <= tbInputs->TBTreatments[recentTreatNum].monthsOfEfficacyActivation[tbStrain]){
			effectiveTreatNum = recentTreatNum;
		}	
	}

	if (effectiveTreatNum != SimContext::NOT_APPL) {
		double efficacy;

		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyActivationHIVNeg[tbStrain];
		else
			efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyActivationHIVPos[tbStrain][cd4Strata];

		probActivate = CepacUtil::probRateMultiply(probActivate, 1 - efficacy);
	}

	//Roll for activation
	double randNum = CepacUtil::getRandomDouble(140140, patient);
	if(randNum < probActivate){
		//Roll for pulm vs extra pulm
		randNum =  CepacUtil::getRandomDouble(140150, patient);
		double probPulm = 0;
		SimContext::TB_STATE newTBState;
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			probPulm = tbInputs->probPulmonaryOnActivationHIVNeg;
		else
			probPulm = tbInputs->probPulmonaryOnActivationHIVPos[cd4Strata];

		if (randNum < probPulm){
			newTBState = SimContext::TB_STATE_ACTIVE_PULM;
			//roll for prob sputum bacillary load hi
			double probSputum = 0;
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				probSputum = tbInputs->probSputumHiOnActivationPulmHIVNeg;
			else
				probSputum = tbInputs->probSputumHiOnActivationPulmHIVPos[cd4Strata];
			randNum =  CepacUtil::getRandomDouble(140160, patient);
			if (randNum < probSputum  && !patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]){
					setTBTracker(SimContext::TB_TRACKER_SPUTUM_HI, true);
			}
		}
		else{
			newTBState = SimContext::TB_STATE_ACTIVE_EXTRAPULM;
		}

		setTBDiseaseState(newTBState, false, SimContext::TB_INFECT_REACTIVATE, SimContext::TB_STATE_LATENT);

		// If patient is on proph and activated, roll for prob of increased resistance
		if (patient->getTBState()->isOnProph) {
			int prophNum = patient->getTBState()->currProphNum;
			SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
			double randNum = CepacUtil::getRandomDouble(140070, patient);
			if ((randNum < tbInputs->tbProphInputs[prophNum].probResistanceInActiveStates) &&
				(tbStrain < SimContext::TB_STRAIN_XDR)) {
					increaseTBDrugResistance(false);
					SimContext::TB_STRAIN newTBStrain = patient->getTBState()->currTrueTBResistanceStrain;
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d TB PROPH INCR RESIST %s;\n",
							patient->getGeneralState()->monthNum, SimContext::TB_STRAIN_STRS[newTBStrain]);
					}
			}
		}

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TB Activation %s %s, Sputum: %s, Immune Reactive:%s, TB Symptoms:%s ;\n", patient->getGeneralState()->monthNum,
				SimContext::TB_STRAIN_STRS[patient->getTBState()->currTrueTBResistanceStrain],
				SimContext::TB_STATE_STRS[patient->getTBState()->currTrueTBDiseaseState],
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]?"Yes":"No");
		}
	}
} /* end performLatentTBUpdates */

/** \brief performActivePulmTBUpdates handles transitions from the active pulm tb state */
void TBDiseaseUpdater::performActivePulmTBUpdates() {
	//roll for self-cure
	if(simContext->getTBInputs()->enableSelfCure){
		int monthsSinceAct = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTBStateChange;
		if (monthsSinceAct == simContext->getTBInputs()->selfCureTime)
			setTBSelfCure(true);
	}
} /* end performActivePulmTBUpdates */

/** \brief performActiveExtraPulmTBUpdates handles transitions from the active extra pulm TB state */
void TBDiseaseUpdater::performActiveExtraPulmTBUpdates() {
	//roll for self-cure
	if(simContext->getTBInputs()->enableSelfCure){
		int monthsSinceAct = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTBStateChange;
		if (monthsSinceAct == simContext->getTBInputs()->selfCureTime)
			setTBSelfCure(true);
	}
} /* end performActiveExtraPulmTBUpdates */

/** \brief performPrevTreatTBUpdates handles transitions from the previously treated (successful) TB state */
void TBDiseaseUpdater::performPrevTreatTBUpdates() {
	//Roll for TB reinfection
	rollForInfection(SimContext::TB_STATE_PREV_TREATED);
	//If reinfected, TB state changes to Latent
	if (patient->getTBState()->currTrueTBDiseaseState != SimContext::TB_STATE_PREV_TREATED)
		return;

	// patients who experienced self-cure have strong immune systems and will not relapse
	if(!patient->getTBState()->isSelfCured){
		rollForRelapse(SimContext::TB_STATE_PREV_TREATED);
	}	
} /* end performPrevTreatTBUpdates */

/** \brief performTreatDefaultTBUpdates handles transitions from the previously treated but defaulted TB state*/
void TBDiseaseUpdater::performTreatDefaultTBUpdates() {
	//Roll for TB reinfection
	rollForInfection(SimContext::TB_STATE_TREAT_DEFAULT);
	//If reinfected, TB state changes to Latent
	if (patient->getTBState()->currTrueTBDiseaseState != SimContext::TB_STATE_TREAT_DEFAULT)
		return;

	rollForRelapse(SimContext::TB_STATE_TREAT_DEFAULT);
} /* end performTreatDefaultTBUpdates */



/** \brief rollForTBSymptoms checks to see if patients aquire TB symptoms that month and also clears TB symptoms from the previous month (active TB states do not clear symptoms
 */
void TBDiseaseUpdater::rollForTBSymptoms() {

	//Clear TB symptoms if not in active TB state
	SimContext::TB_STATE tbState = patient->getTBState()->currTrueTBDiseaseState;
	if (tbState != SimContext::TB_STATE_ACTIVE_PULM && tbState != SimContext::TB_STATE_ACTIVE_EXTRAPULM)
		setTBTracker(SimContext::TB_TRACKER_SYMPTOMS, false);

	//Roll for TB symptoms
	double probSymptoms;

	if(patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		probSymptoms = simContext->getTBInputs()->probTBSymptomsMonthHIVPos[cd4Strata][patient->getTBState()->currTrueTBDiseaseState];
	}
	else{
		probSymptoms = simContext->getTBInputs()->probTBSymptomsMonthHIVNeg[patient->getTBState()->currTrueTBDiseaseState];
	}
	double randNum = CepacUtil::getRandomDouble(140065, patient);

	if (randNum < probSymptoms)
		setTBTracker(SimContext::TB_TRACKER_SYMPTOMS, true);

} /* end rollForTBSymptoms */

/** \brief rollForInfection handles Infection and Reinfection from the uninfected, latent, prev treated or default tb states
 * \param tbState is the state from which infection is rolled from
 */
void TBDiseaseUpdater::rollForInfection(SimContext::TB_STATE tbState) {
	const SimContext::TBInputs *tbInputs = simContext->getTBInputs();

	// Calculate the probability of a first infection
	double probInfect;
	SimContext::CD4_STRATA cd4Strata;
	if(patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG)
		cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;

	int ageCat = getAgeCategoryTBInfection(patient->getGeneralState()->ageMonths);
	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
		probInfect = tbInputs->probInfectionHIVNeg[ageCat];
	}
	else{
		probInfect = tbInputs->probInfectionHIVPos[cd4Strata][ageCat];
	}

	// Apply rate multiplier for TB state
	probInfect = CepacUtil::probRateMultiply(probInfect, tbInputs->infectionMultiplier[tbState]);

	// Modify prob by proph efficacy if on TB proph or recently completed a course
	if (patient->getTBState()->isOnProph){
		double efficacy;
		int prophNum = patient->getTBState()->currProphNum;
		if(!patient->getTBState()->hasTrueHistoryTB){
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				efficacy = tbInputs->tbProphInputs[prophNum].efficacyInfectionHIVNeg[0];
			else
				efficacy = tbInputs->tbProphInputs[prophNum].efficacyInfectionHIVPos[cd4Strata][0];
		}
		else{
			SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				efficacy = tbInputs->tbProphInputs[prophNum].efficacyReinfectionHIVNeg[tbStrain][0];
			else
				efficacy = tbInputs->tbProphInputs[prophNum].efficacyReinfectionHIVPos[tbStrain][cd4Strata][0];
		}		
		probInfect = CepacUtil::probRateMultiply(probInfect, 1 - efficacy);
	}
	else if (patient->getTBState()->hasCompletedProph){
		int effHorizon;
		int decayPeriod;
		int recentProphNum = patient->getTBState()->mostRecentProphNum;
		
		if(!patient->getTBState()->hasTrueHistoryTB){
			effHorizon = tbInputs->tbProphInputs[recentProphNum].monthsOfEfficacyInfection;
			decayPeriod = tbInputs->tbProphInputs[recentProphNum].decayPeriodInfection;
		}
		else{
			SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
			effHorizon = tbInputs->tbProphInputs[recentProphNum].monthsOfEfficacyReinfection[tbStrain];
			decayPeriod = tbInputs->tbProphInputs[recentProphNum].decayPeriodReinfection[tbStrain];
		}
	 	if(patient->getGeneralState()->monthNum <= patient->getTBState()->monthOfProphStop + effHorizon + decayPeriod ){
			double efficacy;
			if(!patient->getTBState()->hasTrueHistoryTB){
				if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
					efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyInfectionHIVNeg[1];
				else
					efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyInfectionHIVPos[cd4Strata][1];
			}
			else{
				//use reinfection values
				SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
				if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
					efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyReinfectionHIVNeg[tbStrain][1];
				else
					efficacy = tbInputs->tbProphInputs[recentProphNum].efficacyReinfectionHIVPos[tbStrain][cd4Strata][1];
			}

			if(patient->getGeneralState()->monthNum > patient->getTBState()->monthOfProphStop + effHorizon){
				int monthsSinceDecay = patient->getGeneralState()->monthNum - (patient->getTBState()->monthOfProphStop + effHorizon);
				double propDecay = (double) monthsSinceDecay / decayPeriod;
				efficacy *= (1-propDecay);
			}
			probInfect = CepacUtil::probRateMultiply(probInfect, 1 - efficacy);
		}
	}

	// Modify prob by treat efficacy if on TB treat
	int effectiveTreatNum = SimContext::NOT_APPL;
	int monthsOfEfficacy;

	if (patient->getTBState()->isOnTreatment || patient->getTBState()->isOnEmpiricTreatment){
		// mostRecentTreatNum is the same as the current TB treatment nubmer but without the flag indicating whether the treatment is empiric
		effectiveTreatNum = patient->getTBState()->mostRecentTreatNum;
	}	
	else if (patient->getTBState()->hasStoppedTreatmentOrEmpiric){
		int recentTreatNum = patient->getTBState()->mostRecentTreatNum;
		if (!patient->getTBState()->hasTrueHistoryTB){
			monthsOfEfficacy = tbInputs->TBTreatments[recentTreatNum].monthsOfEfficacyInfection;
		}
		else{
			SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
			monthsOfEfficacy = tbInputs->TBTreatments[recentTreatNum].monthsOfEfficacyReinfection[tbStrain];
		}

		if(patient->getGeneralState()->monthNum <=patient->getTBState()->monthOfTreatmentOrEmpiricStop + monthsOfEfficacy){
			effectiveTreatNum = recentTreatNum;
		}	
	}
	else if (patient->getTBState()->observedHistActiveTBAtEntry){
		int recentTreatNum = patient->getTBState()->mostRecentTreatNum;
		SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
		if(patient->getGeneralState()->monthNum + patient->getTBState()->monthsSinceInitTBTreatStopAtEntry <= tbInputs->TBTreatments[recentTreatNum].monthsOfEfficacyReinfection[tbStrain]){
			effectiveTreatNum = recentTreatNum;
		}
	}

	if (effectiveTreatNum != SimContext::NOT_APPL) {
		double efficacy;

		//Use infection values
		if (!patient->getTBState()->hasTrueHistoryTB){
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyInfectionHIVNeg;
			else
				efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyInfectionHIVPos[cd4Strata];
		}
		else{
			//use reinfection values
			SimContext::TB_STRAIN tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyReinfectionHIVNeg[tbStrain];
			else
				efficacy = tbInputs->TBTreatments[effectiveTreatNum].efficacyReinfectionHIVPos[tbStrain][cd4Strata];
		}
		probInfect = CepacUtil::probRateMultiply(probInfect, 1 - efficacy);
	}

	// Determine if infection/reinfection occurs
	double randNum = CepacUtil::getRandomDouble(140090, patient);
	if (randNum < probInfect) {
		// Roll for resistance strain of infection
		SimContext::TB_STRAIN newTBStrain = SimContext::TB_STRAIN_DS;
		randNum = CepacUtil::getRandomDouble(140100, patient);
		for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++) {
			if (randNum < tbInputs->distributionTBStrainAtEntry[i]) {
				newTBStrain = (SimContext::TB_STRAIN) i;
				break;
			}
			randNum -= tbInputs->distributionTBStrainAtEntry[i];
		}

		// Update TB state for infection or reinfection
		setTBResistanceStrain(newTBStrain);

		if(!patient->getTBState()->hasTrueHistoryTB){
			setTBDiseaseState(SimContext::TB_STATE_LATENT, true, SimContext::TB_INFECT_INITIAL, tbState);
		}
		//Note that, although these TB states are all being reinfected, if the original tbState is Latent TB their TB state will not actually change. They may, however, have drawn a different drug resistance strain. 
		else{
			setTBDiseaseState(SimContext::TB_STATE_LATENT, true, SimContext::TB_INFECT_REINFECT, tbState);
		}
		setTBSelfCure(false);

		//Roll for getting immune reactive tracker status if not already
		double probReact;
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			probReact = tbInputs->probImmuneReactiveOnInfectionHIVNeg;
		else
			probReact = tbInputs->probImmuneReactiveOnInfectionHIVPos[cd4Strata];

		randNum = CepacUtil::getRandomDouble(140105, patient);

		if (randNum < probReact && !patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE])
			setTBTracker(SimContext::TB_TRACKER_IMMUNE_REACTIVE, true);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TB INFECTION %s %s, Sputum: %s, Immune Reactive:%s, TB Symptoms:%s ;\n", patient->getGeneralState()->monthNum,
				SimContext::TB_STRAIN_STRS[patient->getTBState()->currTrueTBResistanceStrain],
				SimContext::TB_STATE_STRS[patient->getTBState()->currTrueTBDiseaseState],
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]?"Yes":"No");
		}
	}
} /* end rollForInfection */



/** \brief rollForRelapse handles Relapse from the treated or default states to the active pulm or active extra pulm states
 * \param tbState is the state from which infection is rolled from
 */
void TBDiseaseUpdater::rollForRelapse(SimContext::TB_STATE tbState) {

	const SimContext::TBInputs *tbInputs = simContext->getTBInputs();

	// Calculate probability of Relapse
	double probRelapse = 0;
	SimContext::CD4_STRATA cd4Strata;
	double fCD4 = 0;
	int timeSinceTreatment;
	int timeSinceInitTreatment;
	float treatMult = 1.0;
	if (patient->getTBState()->hasStoppedTreatmentOrEmpiric){
		timeSinceTreatment = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentOrEmpiricStop;

		if(patient->getTBState()->observedHistActiveTBAtEntry){
			timeSinceInitTreatment = patient->getTBState()->monthsSinceInitTBTreatStopAtEntry + patient->getGeneralState()->monthNum;
		}
		else{
			timeSinceInitTreatment = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfInitialTreatmentStop;
		}
	}
	// Since self-cures cannot relapse, if they have not stopped treatment since model start then they must have stopped treatment before model entry
	else {
		timeSinceInitTreatment = patient->getTBState()->monthsSinceInitTBTreatStopAtEntry + patient->getGeneralState()->monthNum;
		timeSinceTreatment = timeSinceInitTreatment;
	}
	// Check whether they are within the duration for the treatment relapse rate multiplier
	int treatNum = patient->getTBState()->mostRecentTreatNum;
	if(timeSinceTreatment <= tbInputs->TBTreatments[treatNum].relapseMultDuration){
		treatMult = tbInputs->TBTreatments[treatNum].relapseMult;
	}

	if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		fCD4 = tbInputs->probRelapseTtoAFCD4[cd4Strata];
	}
	else
		fCD4 = 1;

	if (timeSinceTreatment < tbInputs->probRelapseEffHorizon){
		probRelapse = 0;
	}	
	else{
		probRelapse = fCD4 * tbInputs->probRelapseTtoARateMultiplier * exp(-1*min(timeSinceTreatment, tbInputs->probRelapseTtoAThreshold) * tbInputs->probRelapseTtoAExponent);
		if(tbState == SimContext::TB_STATE_TREAT_DEFAULT){
			probRelapse = CepacUtil::probRateMultiply(probRelapse, simContext->getTBInputs()->relapseRateMultTBTreatDefault);
		}
		probRelapse = CepacUtil::probRateMultiply(probRelapse, treatMult);
	}
	//Roll for Relapse
	double randNum = CepacUtil::getRandomDouble(140180, patient);
	if(randNum < probRelapse){
		//Roll for pulm vs extra pulm
		randNum =  CepacUtil::getRandomDouble(140190, patient);
		double probPulm = 0;
		SimContext::TB_STATE newTBState;
		if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			probPulm = tbInputs->probPulmonaryOnRelapseHIVNeg;
		else
			probPulm = tbInputs->probPulmonaryOnRelapseHIVPos[cd4Strata];

		if (randNum < probPulm){
			newTBState = SimContext::TB_STATE_ACTIVE_PULM;
			//roll for prob sputum bacillary load hi
			double probSputum = 0;
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
				probSputum = tbInputs->probSputumHiOnRelapsePulmHIVNeg;
			else
				probSputum = tbInputs->probSputumHiOnRelapsePulmHIVPos[cd4Strata];
			randNum =  CepacUtil::getRandomDouble(140160, patient);
			if (randNum < probSputum  && !patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]){
					setTBTracker(SimContext::TB_TRACKER_SPUTUM_HI, true);
			}
		}
		else{
			newTBState = SimContext::TB_STATE_ACTIVE_EXTRAPULM;
		}

		setTBDiseaseState(newTBState, false, SimContext::TB_INFECT_RELAPSE, tbState);

		if(timeSinceInitTreatment <= tbInputs->monthRelapseUnfavorableOutcome)
			setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_RELAPSE);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TB Relapse %s %s, Sputum: %s, Immune Reactive:%s, TB Symptoms:%s ;\n", patient->getGeneralState()->monthNum,
				SimContext::TB_STRAIN_STRS[patient->getTBState()->currTrueTBResistanceStrain],
				SimContext::TB_STATE_STRS[patient->getTBState()->currTrueTBDiseaseState],
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE]?"Yes":"No",
				patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]?"Yes":"No");
		}
	}
} /* end rollForRelapse */
