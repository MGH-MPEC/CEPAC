#include "include.h"

/** \brief Constructor takes in the patient as a pointer */
DrugToxicityUpdater::DrugToxicityUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
DrugToxicityUpdater::~DrugToxicityUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void DrugToxicityUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	setHadTBTreatmentMajorTox(false);
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month
 *
 * Calls the following helper functions depending on current drug status:
 * 	- DrugToxicityUpdater::performARTToxicityUpdates()
 * 	- DrugToxicityUpdater::performProphToxicityUpdates()
 * 	- DrugToxicityUpdater::performTBProphToxicityUpdates()
 * 	- DrugToxicityUpdater::performTBTreatmentToxicityUpdates()
 **/
void DrugToxicityUpdater::performMonthlyUpdates() {
	if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		if (patient->getARTState()->hasTakenART)
			performARTToxicityUpdates();

		if (patient->getProphState()->currTotalNumProphsOn > 0)
			performProphToxicityUpdates();
	}
	if (simContext->getTBInputs()->enableTB){
		if (patient->getTBState()->isOnProph)
			performTBProphToxicityUpdates();

		performTBTreatmentToxicityUpdates();
	}

} /* end performMonthlyUpdates */

/** \brief performARTToxicityUpdates handles all toxicity updates for ART regimens */
void DrugToxicityUpdater::performARTToxicityUpdates() {
	/** Loop over all active toxicity effects and process them */
	const list<SimContext::ARTToxicityEffect> &toxicities = patient->getARTState()->activeToxicityEffects;
	list<SimContext::ARTToxicityEffect>::const_iterator currToxIter = toxicities.begin();

	while (currToxIter != toxicities.end()) {

		/** Skip a toxicity effect if the month of tox start has not yet been reached */
		const SimContext::ARTToxicityEffect &toxEffect = *currToxIter;

		if (patient->getGeneralState()->monthNum < toxEffect.monthOfToxStart) {
			currToxIter++;
			continue;
		}

		/** If a toxicity is to begin this month, verify that the patient is still on the
		//	subregimen causing it, remove it and skip to the next tox if not */
		if (patient->getGeneralState()->monthNum == toxEffect.monthOfToxStart) {
			if (!patient->getARTState()->isOnART ||
				(patient->getARTState()->currRegimenNum != toxEffect.ARTRegimenNum) ||
				(patient->getARTState()->currSubRegimenNum != toxEffect.ARTSubRegimenNum)) {
				list<SimContext::ARTToxicityEffect>::const_iterator tempIter = currToxIter;
				tempIter++;
				removeARTToxicityEffect(currToxIter);
				currToxIter = tempIter;
				continue;
			}
		}

		/** Perform special handling if this is the initial month of toxicity */
		const SimContext::ARTInputs::ARTToxicity &toxInputs = simContext->getARTInputs(toxEffect.ARTRegimenNum)->toxicity[toxEffect.ARTSubRegimenNum][toxEffect.toxSeverityType][toxEffect.toxNum];
		if (patient->getGeneralState()->monthNum == toxEffect.monthOfToxStart) {
			// Increment incidence tracker
			updateARTToxIncidenceTracking(toxEffect);
			// Print out tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d %s TOX(%s): ART %d.%d\n", patient->getGeneralState()->monthNum,
					toxInputs.toxicityName.c_str(), SimContext::ART_TOX_SEVERITY_STRS[toxEffect.toxSeverityType],
					toxEffect.ARTRegimenNum + 1, toxEffect.ARTSubRegimenNum);
			}

			/** - Update the state and statistics for the occurrence of the toxicity */
			setARTToxicity(toxEffect);

			/** - Schedule an emergency visit if the toxicity is considered to be severe,
			//	either a major one causing the regimen to stop or any that causes a drug substitution */
			if (patient->getARTState()->hasSevereToxicity)
				scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);

			/** - Add the major tox death rate ratio to the mortality risks if applicable */
			if (toxEffect.toxSeverityType == SimContext::ART_TOX_MAJOR && toxInputs.acuteMajorToxDeathRateRatio > 1.0) {
				addMortalityRisk(SimContext::DTH_TOX_ART, toxInputs.acuteMajorToxDeathRateRatio, toxInputs.costAcuteDeathMajorToxicity);
			}
		}

		/** Evaluate and apply toxicity QOL modifier and cost; add chronic toxicity death rate ratio to the mortality risks if applicable */
		bool useQOL = evaluateARTToxDuration(toxEffect, toxInputs.QOLDuration);
		if (useQOL) {
			accumulateQOLModifier(toxInputs.QOLModifier);
		}
		bool useCost = evaluateARTToxDuration(toxEffect, toxInputs.costDuration);
		if (useCost) {
			incrementCostsToxicity(toxInputs.costAmount);
		}
		bool useChronicDeath = false;
		if (toxEffect.toxSeverityType == SimContext::ART_TOX_CHRONIC) {
			useChronicDeath = evaluateARTToxDuration(toxEffect, toxInputs.chronicDeathDuration);
		}
		if (useChronicDeath) {
			if (patient->getGeneralState()->monthNum >= toxEffect.monthOfToxStart + toxInputs.timeToChronicDeathImpact) {
				updateARTToxPrevalenceTracking(toxEffect);
				if(toxInputs.chronicToxDeathRateRatio > 1.0) {
					addMortalityRisk(SimContext::DTH_TOX_ART, toxInputs.chronicToxDeathRateRatio);
				}	
			}	
		}

		/** If none of the effects are still valid, remove the toxicity structure and process next one */
		if (!useQOL && !useCost && !useChronicDeath) {
			list<SimContext::ARTToxicityEffect>::const_iterator tempIter = currToxIter;
			tempIter++;
			removeARTToxicityEffect(currToxIter);
			currToxIter = tempIter;
		}
		else {
			/** Print toxicity effect tracing if enabled */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "  %d TOX EFFECT %s TOX(%s): ART %d.%d %s %s %s\n",
						patient->getGeneralState()->monthNum,
						toxInputs.toxicityName.c_str(), SimContext::ART_TOX_SEVERITY_STRS[toxEffect.toxSeverityType],
						toxEffect.ARTRegimenNum + 1, toxEffect.ARTSubRegimenNum,
						useQOL ? "useQOL" : "",
						useCost ? "useCost" : "",
						useChronicDeath? "useChronicDeath" : "");
			}
			// Increment iterator to the next toxicity effect
			currToxIter++;
		}
	}
} /* end performARTToxicityUpdates */

/** \brief performProphToxicityUpdates handles all toxicity updates for prophylaxis */
void DrugToxicityUpdater::performProphToxicityUpdates() {

	/** Determine if OI proph toxicity occurs this month by iterating through all possible OI prophs */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		/** Skip this OI if no prophs are taken for it */

		if (!patient->getProphState()->isOnProph[i])
			continue;

		/** Skip this proph if this is not the month of toxicity */
		SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
		int prophNum = patient->getProphState()->currProphNum[i];
		const SimContext::ProphInputs *prophInputs;

		if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
			prophInputs=simContext->getProphInputs(prophType, i, prophNum);
		}
		else{
			prophInputs=simContext->getPedsProphInputs(prophType, i, prophNum);
		}
		assert(prophInputs!=NULL);
		if (patient->getGeneralState()->monthNum - patient->getProphState()->monthOfProphStart[i] != prophInputs->monthsToToxicity){

			continue;
		}

		/** Calculate probability of no toxicity occurring */
		double probMajorTox = prophInputs->probMajorToxicity;
		double probMinorTox = prophInputs->probMinorToxicity;
		double probNoTox = (1 - probMajorTox) * (1 - probMinorTox);

		/** Roll for no toxicity occurring, skip to next proph if none occur */
		double randNum = CepacUtil::getRandomDouble(80010, patient);
		if (randNum < probNoTox)
			continue;

		/** If some toxicity occurred, determine normalized distribution of type of toxicity */
		double probOnlyMajorTox = probMajorTox * (1 - probMinorTox);
		double probOnlyMinorTox = probMinorTox * (1 - probMajorTox);
		double distMajorTox = 1.0;
		if ((probOnlyMajorTox + probOnlyMinorTox) > 0)
			distMajorTox = probOnlyMajorTox / (probOnlyMajorTox + probOnlyMinorTox);

		/** Roll for toxicity being a major one, otherwise its a minor one */
		randNum = CepacUtil::getRandomDouble(80020, patient);
		if (randNum < distMajorTox) {
			// Record the occurrence of toxicity, update costs and QOL
			setProphToxicity(true, (SimContext::OI_TYPE) i);
			accumulateQOLModifier(prophInputs->QOLMajorToxicity);
			incrementCostsToxicity(prophInputs->costMajorToxicity);

			// Print tracing information if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d MAJ TOX: OI %s %s PROPH %d, %1.2lf QAred, $ %1.0lf;\n",
					patient->getGeneralState()->monthNum, SimContext::OI_STRS[i],
					SimContext::PROPH_TYPE_STRS[prophType],
					prophNum + 1, patient->getGeneralState()->QOLValue,
					patient->getGeneralState()->costsDiscounted);
			}
			// Add death rate ratio for prophylaxis toxicity to the mortality risks if applicable
			if (prophInputs->deathRateRatioMajorToxicity > 1.0) {
				addMortalityRisk(SimContext::DTH_TOX_PROPH, prophInputs->deathRateRatioMajorToxicity );
			}	
			/** Schedule an emergency clinic visit if they should be switched on a major toxicity */
			if (prophInputs->switchOnMajorToxicity) {
				scheduleEmergencyClinicVisit(SimContext::EMERGENCY_PROPH, patient->getGeneralState()->monthNum);
			}
		}
		else {
			/** Record the occurrence of the minor toxicity, update cost and QOL */
			setProphToxicity(false, (SimContext::OI_TYPE) i);
			accumulateQOLModifier(prophInputs->QOLMinorToxicity);
			incrementCostsToxicity(prophInputs->costMinorToxicity);

			/** Print tracing information if enabled */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d MIN TOX: OI %s %s PROPH %d, %1.2lf QAred, $ %1.0lf;\n",
					patient->getGeneralState()->monthNum, SimContext::OI_STRS[i],
					SimContext::PROPH_TYPE_STRS[prophType],
					prophNum + 1, patient->getGeneralState()->QOLValue,
					patient->getGeneralState()->costsDiscounted);
			}

			/** Schedule an emergency clinic visit if they should be switched on a minor toxicity */
			if (prophInputs->switchOnMinorToxicity) {
				scheduleEmergencyClinicVisit(SimContext::EMERGENCY_PROPH, patient->getGeneralState()->monthNum);
			}
		}
	}
} /* end performProphToxicityUpdates */

/** \brief performTBProphToxicityUpdates handles all toxicity updates for TB prophylaxis */
void DrugToxicityUpdater::performTBProphToxicityUpdates() {
	int prophNum = patient->getTBState()->currProphNum;

	// Roll for TB proph dropout
	double randNum = CepacUtil::getRandomDouble(70070, patient);
	if (randNum < simContext->getTBInputs()->probDropoffProph) {
		stopCurrTBProph();

		//They are not eligible to get proph again if they drop out
		setTBProphEligibility(false, true);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TB PROPH %d DROPOUT;\n",
				patient->getGeneralState()->monthNum, prophNum);
		}
		return;
	}

	// Calculate probability major, minor, or no toxicity occurring
	double probMajorTox;
	double probMinorTox;
	if(patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		if(patient->getARTState()->isOnART){
			double responseFactor = patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_TOX];
			probMajorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMajorToxicityOffART*(1-responseFactor) +
					simContext->getTBInputs()->tbProphInputs[prophNum].probMajorToxicityOnART*responseFactor;
			probMinorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMinorToxicityOffART*(1-responseFactor) +
					simContext->getTBInputs()->tbProphInputs[prophNum].probMinorToxicityOnART*responseFactor;
		}
		else{
			probMajorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMajorToxicityOffART;
			probMinorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMinorToxicityOffART;
		}
	}
	else{
		probMajorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMajorToxicityHIVNeg;
		probMinorTox = simContext->getTBInputs()->tbProphInputs[prophNum].probMinorToxicityHIVNeg;
	}

	double probNoTox = (1 - probMajorTox) * (1 - probMinorTox);

	// Roll for no toxicity occurring, and return if none occur
	randNum = CepacUtil::getRandomDouble(80030, patient);
	if (randNum < probNoTox)
		return;

	// If some toxicity occurred, determine normalized distribution of type of toxicity
	double probOnlyMajorTox = probMajorTox * (1 - probMinorTox);
	double probOnlyMinorTox = probMinorTox * (1 - probMajorTox);
	double distMajorTox = 1.0;
	if ((probOnlyMajorTox + probOnlyMinorTox) > 0)
		distMajorTox = probOnlyMajorTox / (probOnlyMajorTox + probOnlyMinorTox);

	// Roll for toxicity being a major one, otherwise its a minor one
	randNum = CepacUtil::getRandomDouble(80040, patient);
	if (randNum < distMajorTox) {
		// Increment the statistics, costs, and QOL for the TB proph toxicity
		setTBProphToxicity(true);
		accumulateQOLModifier(simContext->getTBInputs()->tbProphInputs[prophNum].QOLModifierMajorToxicity);
		incrementCostsTBToxicity(simContext->getTBInputs()->tbProphInputs[prophNum].costMajorToxicity);

		// Print tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d MAJ TOX: TB PROPH %d, %1.2lf QAred, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, prophNum,
				patient->getGeneralState()->QOLValue, patient->getGeneralState()->costsDiscounted);
		}
	
		// Add the death rate ratio for TB prophylaxis major toxicity to the mortality risks if applicable
		if (simContext->getTBInputs()->tbProphInputs[prophNum].deathRateRatioMajorToxicity > 1.0) {
			addMortalityRisk(SimContext::DTH_TOX_TB_PROPH, simContext->getTBInputs()->tbProphInputs[prophNum].deathRateRatioMajorToxicity);
		}	
	}
	else {
		// Increment the statistics, costs, and QOL for the TB proph toxicity
		setTBProphToxicity(false);
		accumulateQOLModifier(simContext->getTBInputs()->tbProphInputs[prophNum].QOLModifierMinorToxicity);
		incrementCostsTBToxicity(simContext->getTBInputs()->tbProphInputs[prophNum].costMinorToxicity);

		// Print tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d MIN TOX: TB PROPH %d, %1.2lf QAred, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, prophNum,
				patient->getGeneralState()->QOLValue, patient->getGeneralState()->costsDiscounted);
		}
	}
} /* end performTBProphToxicityUpdates */

/** \brief performTBTreatmentToxicityUpdates handles all toxicity updates for TB treatment */
void DrugToxicityUpdater::performTBTreatmentToxicityUpdates() {
	bool isOnTreatment = false;
	int treatNum;
	int stage = 1;
	//Due to the timing of treatment start within the simulation month, the first TB treatment stage is calculated differently depending on whether it is at the end of the month (LTFU and costs: threshold month excluded) or the beginning of the month (toxicity: threshold month included)

	SimContext::TBInputs::TBTreatment tbTreat;
	if (patient->getTBState()->isOnTreatment){
		isOnTreatment = true;
		treatNum = patient->getTBState()->currTreatmentNum;
		tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];
		if((patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart + patient->getTBState()->previousTreatmentDuration) <= tbTreat.stage1Duration)
			stage = 0;
	}
	else if (patient->getTBState()->isOnEmpiricTreatment){
		isOnTreatment = true;
		treatNum = patient->getTBState()->currEmpiricTreatmentNum;
		tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];
		if((patient->getGeneralState()->monthNum - patient->getTBState()->monthOfEmpiricTreatmentStart + patient->getTBState()->previousEmpiricTreatmentDuration) <= tbTreat.stage1Duration)
			stage = 0;
	}
	if (!isOnTreatment)
		return;

	// Calculate probability major, minor, or no toxicity occurring
	double probMajorTox;
	double probMinorTox;
	double probDthMajor;
	double minorCost;
	double minorQOL;
	double majorCost;
	double majorQOL;

	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
		probMajorTox = tbTreat.probMajorToxHIVNeg[stage];
		probMinorTox = tbTreat.probMinorToxHIVNeg[stage];

		majorCost = tbTreat.costMajorToxHIVNeg;
		majorQOL = tbTreat.QOLModMajorToxHIVNeg;
		minorCost = tbTreat.costMinorToxHIVNeg;
		minorQOL = tbTreat.QOLModMinorToxHIVNeg;
	}
	else{
		if (patient->getARTState()->isOnART){
			double responseFactor = patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_TOX];
			probMajorTox = tbTreat.probMajorToxOffARTHIVPos[stage]*(1-responseFactor) +
				tbTreat.probMajorToxOnARTHIVPos[stage]*responseFactor;
			probMinorTox = tbTreat.probMinorToxOffARTHIVPos[stage]*(1-responseFactor) + 
				tbTreat.probMinorToxOnARTHIVPos[stage]*responseFactor;
		}
		else{
			probMajorTox = tbTreat.probMajorToxOffARTHIVPos[stage];
			probMinorTox = tbTreat.probMinorToxOffARTHIVPos[stage];
		}
		majorCost = tbTreat.costMajorToxHIVPos;
		majorQOL = tbTreat.QOLModMajorToxHIVPos;
		minorCost = tbTreat.costMinorToxHIVPos;
		minorQOL = tbTreat.QOLModMinorToxHIVPos;
	}

	double probNoTox = (1 - probMajorTox) * (1 - probMinorTox);

	// Roll for no toxicity occurring, and return if none occur
	double randNum = CepacUtil::getRandomDouble(80050, patient);
	if (randNum < probNoTox)
		return;

	// If some toxicity occurred, determine normalized distribution of type of toxicity
	double probOnlyMajorTox = probMajorTox * (1 - probMinorTox);
	double probOnlyMinorTox = probMinorTox * (1 - probMajorTox);
	double distMajorTox = 1.0;
	if ((probOnlyMajorTox + probOnlyMinorTox) > 0)
		distMajorTox = probOnlyMajorTox / (probOnlyMajorTox + probOnlyMinorTox);

	// Roll for toxicity being a major one, otherwise its a minor one
	randNum = CepacUtil::getRandomDouble(80060, patient);
	if (randNum < distMajorTox) {
		// Increment the statistics, costs, and QOL for the major TB treatment toxicity
		setTBTreatmentToxicity(true, treatNum);
		accumulateQOLModifier(majorQOL);
		incrementCostsTBToxicity(majorCost, treatNum);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d MAJ TOX: TB TREAT, %1.2lf QAred, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->QOLValue,
				patient->getGeneralState()->costsDiscounted);
		}

		// Add the death rate ratio for TB treatment major toxicity to the mortality risks if applicable
		if (tbTreat.deathRateRatioMajorToxicity > 1.0) {
			addMortalityRisk(SimContext::DTH_TOX_TB_TREATM, tbTreat.deathRateRatioMajorToxicity );
		}
		
		//Stop current empiric treatment
		if (patient->getTBState()->isOnEmpiricTreatment)
			stopEmpiricTBTreatment();
	}
	else {
		// Increment the statistics, costs, and QOL for the minor TB treatment toxicity
		setTBTreatmentToxicity(false, treatNum);
		accumulateQOLModifier(minorQOL);
		incrementCostsTBToxicity(minorCost, treatNum);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d MIN TOX: TB TREAT, %1.2lf QAred, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->QOLValue,
				patient->getGeneralState()->costsDiscounted);
		}
	}
} /* end performTBTreatmentToxicityUpdates */

/** \brief evaluateARTToxDuration evaluates if the given ART toxicity duration criteria are still met */
bool DrugToxicityUpdater::evaluateARTToxDuration(const SimContext::ARTToxicityEffect &toxEffect, SimContext::ART_TOX_DUR duration) {
	if ((duration == SimContext::ART_TOX_DUR_MONTH) &&
		(patient->getGeneralState()->monthNum == toxEffect.monthOfToxStart))
		return true;
	if ((duration == SimContext::ART_TOX_DUR_SUBREG) &&
		(patient->getARTState()->currRegimenNum == toxEffect.ARTRegimenNum) &&
		(patient->getARTState()->currSubRegimenNum == toxEffect.ARTSubRegimenNum))
		return true;
	if ((duration == SimContext::ART_TOX_DUR_REG) &&
		(patient->getARTState()->currRegimenNum == toxEffect.ARTRegimenNum))
		return true;
	if (duration == SimContext::ART_TOX_DUR_DEATH)
		return true;
	return false;
} /* end evaluateARTToxDuration */
