#include "include.h"

/** \brief Constructor takes in the associated patient object */
CHRMsUpdater::CHRMsUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
CHRMsUpdater::~CHRMsUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void CHRMsUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	//initialize the orphans at the start because you will break out of the loop before we initialize the rest
	if (simContext->getCHRMsInputs()->enableOrphans){
		for (int i = 0; i < SimContext::CHRM_NUM; i++) {
			setTrueCHRMsState(i, false);
		}
	}

	/** Roll for prevalent CHRMs at model entry */
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		// Set the base prevalence probability
		double probCHRM = 0.0;
		SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
		int ageCat = getAgeCategoryCHRMs(patient->getGeneralState()->ageMonths, i);
		SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
			probCHRM = simContext->getCHRMsInputs()->probPrevalentCHRMsHIVneg[i][gender][ageCat];
		}
		else {
			probCHRM = simContext->getCHRMsInputs()->probPrevalentCHRMs[i][cd4Strata][gender][ageCat];
		}

		/** Modify probability by generic risk factor logit adjustments */
		double logitCHRM = CepacUtil::probToLogit(probCHRM);
		for (int j = 0; j < SimContext::RISK_FACT_NUM; j++) {
			if (patient->getGeneralState()->hasRiskFactor[j])
				logitCHRM += simContext->getCHRMsInputs()->probPrevalentCHRMsRiskFactorLogit[i][j];
		}
		probCHRM = CepacUtil::logitToProb(logitCHRM);

		/** Determine if CHRM occurs and update state */
		double randNum = CepacUtil::getRandomDouble(150010, patient);
		if (randNum < probCHRM) {
			// Patient has CHRM i at model entry, roll for month of start and update state
			double monthsMean = simContext->getCHRMsInputs()->prevalentCHRMsMonthsSinceStartMean[i];
			double monthsStdDev = simContext->getCHRMsInputs()->prevalentCHRMsMonthsSinceStartStdDev[i];
			int ageMths = patient->getGeneralState()->ageMonths;
			int monthsStart;
			if (simContext->getCHRMsInputs()->enableOrphans){
				//For orphans, monthsStart is the child's age - i.e. the number of months since they were born
				monthsStart = simContext->getCHRMsInputs()->prevalentCHRMsMonthsSinceStartOrphans[i][getAgeCategoryCHRMsMthsStart(ageMths)];
			}	
			else{
				monthsStart = (int) (CepacUtil::getRandomGaussian(monthsMean, monthsStdDev, 150020, patient) + 0.5);
			}
			if(monthsStart < 0){
				monthsStart = 0;
			}	
			setTrueCHRMsState(i, true, true, monthsStart);
		}
		else {
			//If orphans are enabled only allow to roll for the next CHRM type if they have the previous CHRM type; as soon as they roll for no CHRM they break out of the loop
			if (simContext->getCHRMsInputs()->enableOrphans){
				break;
			}
			setTrueCHRMsState(i, false);
		}
	}
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void CHRMsUpdater::performMonthlyUpdates() {
	bool hasChrms = false;
	int monthOfLastCHRMs;
	if (simContext->getCHRMsInputs()->enableOrphans){
		for (int i = 0; i < SimContext::CHRM_NUM; i++) {
			if (patient->getDiseaseState()->hasTrueCHRMs[i]){
				hasChrms = true;
				monthOfLastCHRMs = patient->getDiseaseState()->monthOfCHRMsStageStart[i][0];
			}
		}
	}

	/** Roll for patient developing each CHRM if they don't already have it */
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		if (simContext->getCHRMsInputs()->enableOrphans){
			if(hasChrms && patient->getGeneralState()->monthNum - monthOfLastCHRMs < simContext->getCHRMsInputs()->incidentCHRMsMonthsSincePreviousOrphans )
				break;
		}
		if (!patient->getDiseaseState()->hasTrueCHRMs[i]) {
			/** - Set the base incidence probability */
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
			int ageCat = getAgeCategoryCHRMs(patient->getGeneralState()->ageMonths, i);
			double probCHRM = 0.0;
			if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
				probCHRM = simContext->getCHRMsInputs()->probIncidentCHRMsHIVneg[i][gender][ageCat];
			}
			else {
				probCHRM = simContext->getCHRMsInputs()->probIncidentCHRMs[i][cd4Strata][gender][ageCat];
			}

			/** - If on ART and eligible to benefit from the ART effect, modify probability by on ART rate multiplier */
			if (patient->getARTState()->applyARTEffect) {
				double rateMult = simContext->getCHRMsInputs()->probIncidentCHRMsOnARTMult[i][cd4Strata];
				/** 	- Adjust multiplier between full ART effect and no ART effect according	to the factor from the ART response type */
				rateMult = 1 - (patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_ARTEFFECT_CHRMS] * (1 - rateMult));
				probCHRM = CepacUtil::probRateMultiply(probCHRM, rateMult);
			}

			/** - Modify probability by generic risk factor logit adjustments */
			double logitCHRM = CepacUtil::probToLogit(probCHRM);
			for (int j = 0; j < SimContext::RISK_FACT_NUM; j++) {
				if (patient->getGeneralState()->hasRiskFactor[j])
					logitCHRM += simContext->getCHRMsInputs()->probIncidentCHRMsRiskFactorLogit[i][j];
			}

			/** - Modify probability by history of other CHRMs logit adjustments */
			for (int j = 0; j < SimContext::CHRM_NUM; j++) {
				if (patient->getDiseaseState()->hasTrueCHRMs[j] &&
					(patient->getDiseaseState()->monthOfCHRMsStageStart[j][0] < patient->getGeneralState()->monthNum)) {
						logitCHRM += simContext->getCHRMsInputs()->probIncidentCHRMsPriorHistoryLogit[i][j];
				}
			}
			probCHRM = CepacUtil::logitToProb(logitCHRM);

			/** Determine if CHRM occurs and update state if so */
			double randNum = CepacUtil::getRandomDouble(150030, patient);
			if (randNum < probCHRM) {
				setTrueCHRMsState(i, true);
				/** Print out tracing for the incidence of the CHRM */
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d INCIDENT CHRMs %s;\n",
							patient->getGeneralState()->monthNum, SimContext::CHRM_STRS[i]);
				}
			}

			//If orphans they only have the chance to roll for a single new CHRM type (child) each month
			if (simContext->getCHRMsInputs()->enableOrphans)
				break;
		}
	}
	//Roll for risk factors which can affect their likelihood of new CHRMs starting next month. Also can affect PTR and mortality risk
	for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
		if (!patient->getGeneralState()->hasRiskFactor[i]) {
			double randNum = CepacUtil::getRandomDouble(30010, patient);
			if (randNum < simContext->getCohortInputs()->probRiskFactorIncid[i]) {
				setRiskFactor(i, true);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d INCIDENT RISK FACTOR %s;\n",
					patient->getGeneralState()->monthNum, SimContext::RISK_FACT_STRS[i]);
				}
			}
		}
	}
	/** a counter to track how many CHRMs a patient currently has */
	int numCHRMs = 0;
	/** For each CHRM condition that exists, handle the costs, QOL, and risk of mortality */
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		if (patient->getDiseaseState()->hasTrueCHRMs[i]) {
			/** Get the stage of CHRMs and patient state */
			SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
			int ageCat = getAgeCategoryCHRMs(patient->getGeneralState()->ageMonths, i);
			
			int stage = SimContext::NOT_APPL;
			for (int j = SimContext::CHRM_TIME_PER_NUM - 1; j >= 0; j--) {
				if (patient->getGeneralState()->monthNum >= patient->getDiseaseState()->monthOfCHRMsStageStart[i][j]) {
					stage = j;
					break;
				}
			}
			// Add risk of death if applicable
			if (simContext->getCHRMsInputs()->CHRMsDeathRateRatio[i][stage][gender][ageCat] > 1.0) {
				addMortalityRisk((SimContext::DTH_CAUSES) (SimContext::DTH_CHRM_1 + i),simContext->getCHRMsInputs()->CHRMsDeathRateRatio[i][stage][gender][ageCat]);
			}
			/** Accumulate the costs of each CHRM */
			double costCHRM = simContext->getCHRMsInputs()->costCHRMs[i][stage][gender][ageCat];
			incrementCostsCHRMs((SimContext::CHRM_TYPE)i,costCHRM);

			/** Accumulate the QOL modifiers for each CHRM */
			double qolCHRM = simContext->getCHRMsInputs()->QOLModCHRMs[i][stage][gender][ageCat];
			accumulateQOLModifier(qolCHRM);
			numCHRMs++;
		}
	}	
	// if the calculation type is marginal, accumulate the appropriate QOL modifier for multiple CHRMs based on the number of CHRMs the person has, ranging in single values from 2 to 10
	if ( simContext->getQOLInputs()->QOLCalculationType == SimContext::MARGINAL){
		if (numCHRMs > 1){
			accumulateQOLModifier(simContext->getCHRMsInputs()->QOLModMultipleCHRMs[numCHRMs-2]);
		}
	}
} /* end performMonthlyUpdates */
