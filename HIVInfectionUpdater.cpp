#include "include.h"

/** \brief Constructor takes in the patient object */
HIVInfectionUpdater::HIVInfectionUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
HIVInfectionUpdater::~HIVInfectionUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void HIVInfectionUpdater::performInitialUpdates() {
	/** First calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	/** Determine the initial HIV state */
	SimContext::HIV_INF infectedState = SimContext::HIV_INF_NEG;
	SimContext::PEDS_HIV_STATE pedsHIVState = SimContext::PEDS_HIV_NEG;
	SimContext::PEDS_MATERNAL_STATUS momHIVState = SimContext::PEDS_MATERNAL_STATUS_NEG;

	bool isHighRisk = true;
	setHIVIncReducMultiplier(1.0);
	// Start with Pediatric intial breastfeeding, maternal, and HIV states if enabled
	if (simContext->getPedsInputs()->enablePediatricsModel) {
		/** For pediatrics: */
		setInitialMaternalState();

		/** Roll for age at which to stop breastfeeding */
		bool reroll = true;
		int bfStopAge;		

		while(reroll){
			bfStopAge = (int)(CepacUtil::getRandomGaussian(simContext->getPedsInputs()->initialBFStopAgeMean,simContext->getPedsInputs()->initialBFStopAgeStdDev, 90017, patient)+0.5);
			if (bfStopAge <= simContext->getPedsInputs()->initialBFStopAgeMax)
				reroll = false;
		}

		setBreastfeedingStopAge(bfStopAge);

		// Set the breastfeeding status
		SimContext::PEDS_BF_TYPE bfType = SimContext::PEDS_BF_REPL;
		double randNum;

		// Use the initial distribution for exclusive and mixed breastfeeding with the patient's age to determine the appropriate status
		if (patient->getGeneralState()->ageMonths < patient->getPedsState()->breastfeedingStopAge){
			randNum = CepacUtil::getRandomDouble(90015, patient);
			for (int i = 0; i < SimContext::PEDS_BF_COMP; i++) {
				if (randNum < simContext->getPedsInputs()->initialBFDistribution[i]) {
					if (patient->getGeneralState()->ageMonths >= SimContext::PEDS_BF_STOP_EXCL_MIXED)
						bfType = (SimContext::PEDS_BF_COMP);
					else
						bfType = (SimContext::PEDS_BF_TYPE) i;
					break;
				}
				randNum -= simContext->getPedsInputs()->initialBFDistribution[i];
			}	
		}
		setBreastfeedingStatus(bfType);

		/** - Roll for and set the initial pediatrics and maternal HIV state */
		//Set Maternal Status

		bool motherBecameInfected = false;
		randNum = CepacUtil::getRandomDouble(90005, patient);
		for (int i = 0; i < SimContext::PEDS_MATERNAL_STATUS_NUM; i++){
			if (randNum < simContext->getPedsInputs()->maternalStatusDistribution[i]){
				momHIVState = (SimContext::PEDS_MATERNAL_STATUS) i;
				if (momHIVState != SimContext::PEDS_MATERNAL_STATUS_NEG)
					motherBecameInfected = true;
				break;
			}
			randNum -= simContext->getPedsInputs()->maternalStatusDistribution[i];
		}
		setMaternalHIVState(momHIVState, motherBecameInfected, true);

		//Roll for if maternal HIV+ status is known
		randNum = CepacUtil::getRandomDouble(90007, patient);
		bool statusKnown = false;
		if(patient->getPedsState()->maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG){
			if (randNum < simContext->getPedsInputs()->probMotherStatusKnownPregnancy[patient->getPedsState()->maternalStatus])
				statusKnown = true;
		}		
		setMaternalStatusKnown(statusKnown);

		//Roll for if mother on ART
		randNum = CepacUtil::getRandomDouble(90008, patient);
		bool isOnART = false;
		if (patient->getPedsState()->maternalStatusKnown && patient->getPedsState()->maternalStatus!=SimContext::PEDS_MATERNAL_STATUS_NEG){
			if (randNum < simContext->getPedsInputs()->probMotherOnARTInitial[patient->getPedsState()->maternalStatus])
				isOnART = true;
		}

		bool isSuppressed = false;
		bool suppressionKnown = false;
		if (isOnART){
			randNum = CepacUtil::getRandomDouble(90009, patient);
			if (randNum < simContext->getPedsInputs()->probMotherOnSuppressedART[patient->getPedsState()->maternalStatus])
				isSuppressed = true;

			randNum = CepacUtil::getRandomDouble(90009, patient);
			double probKnown = 0.0;
			if(isSuppressed)
				probKnown = simContext->getPedsInputs()->probMotherKnownSuppressed[patient->getPedsState()->maternalStatus];
			else
				probKnown = simContext->getPedsInputs()->probMotherKnownNotSuppressed[patient->getPedsState()->maternalStatus];

			if (randNum < probKnown)
				suppressionKnown = true;
		}
		setMaternalARTStatus(isOnART, isSuppressed, suppressionKnown, true);

		//Roll for mother hvl
		randNum = CepacUtil::getRandomDouble(90009, patient);
		if (randNum < simContext->getPedsInputs()->probMotherLowHVL[patient->getPedsState()->maternalStatus])
			setMaternalHVLStatus(true, true);
		else
			setMaternalHVLStatus(false, true);


		//Roll for stop BF upon status known not supp
		if (isOnART && !isSuppressed && suppressionKnown){
			randNum = CepacUtil::getRandomDouble(90009, patient);
			double prob = 0;
			if (patient->getPedsState()->motherLowHVL)
				prob = simContext->getPedsInputs()->probStopBFNotSuppressedLowHVL;
			else
				prob = simContext->getPedsInputs()->probStopBFNotSuppressedHighHVL;

			if (randNum < prob)
				setBreastfeedingStatus(SimContext::PEDS_BF_REPL);
		}

		//Roll for baby infected status
		randNum = CepacUtil::getRandomDouble(90009, patient);
		bool isBabyInfected = false;
		double probInfected = 0.0;
		if (patient->getPedsState()->motherOnART){
			if (patient->getPedsState()->motherOnSuppressedART)
				probInfected = simContext->getPedsInputs()->earlyVTHIVDistributionMotherOnArtSuppressed[patient->getPedsState()->maternalStatus];
			else{
				if (patient->getPedsState()->motherLowHVL)
					probInfected = simContext->getPedsInputs()->earlyVTHIVDistributionMotherOnArtNotSuppressedLowHVL[patient->getPedsState()->maternalStatus];
				else
					probInfected = simContext->getPedsInputs()->earlyVTHIVDistributionMotherOnArtNotSuppressedHighHVL[patient->getPedsState()->maternalStatus];
			}
		}
		else
			probInfected = simContext->getPedsInputs()->earlyVTHIVDistributionMotherOffArt[patient->getPedsState()->maternalStatus];
		if (randNum < probInfected){
			randNum = CepacUtil::getRandomDouble(90010, patient);
			if (randNum < simContext->getPedsInputs()->propEarlyVTHIVIU)
				pedsHIVState = SimContext::PEDS_HIV_POS_IU;
			else
				pedsHIVState = SimContext::PEDS_HIV_POS_IP;
		}
		else
			pedsHIVState = SimContext::PEDS_HIV_NEG;
		setInfectedPediatricsHIVState(pedsHIVState, true);

		/** - Set the regular HIV infected state  */
		// Set the regular infected state for children who were infected at birth or are HIV negative. There are no initial PP infections so the HIV+ are either IU or IP
		if (patient->getDiseaseState()->infectedPediatricsHIVState != SimContext::PEDS_HIV_NEG){
			// if infected at birth, set the infected state to acute or chronic based on the age category; late childhood draws on the adult CD4 and HVL inputs for chronic HIV
			if(patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE)
				infectedState = SimContext::HIV_INF_ACUTE_SYN;
			else
				infectedState = SimContext::HIV_INF_ASYMP_CHR_POS;
		}
		else {			
			infectedState = SimContext::HIV_INF_NEG;			
			setCareState(SimContext::HIV_CARE_NEG);		
		}
		setInfectedHIVState(infectedState, true, false, true);
		
		//Roll for age of serorversion
		if (simContext->getEIDInputs()->enableHIVTestingEID){
			int ageOfSeroreversion = (int)(CepacUtil::getRandomGaussian(simContext->getEIDInputs()->ageOfSeroreversionMean, simContext->getEIDInputs()->ageOfSeroreversionStdDev, 90018, patient)+0.5);
			setAgeOfSeroreversion(ageOfSeroreversion);
		}
		
		//Set initial prob of infant HIV proph efficacy
		for (int i = 0; i < SimContext::INFANT_HIV_PROPHS_NUM; ++i){
            setInfantHIVProphEffProb(i, 1.0);
            setInfantHIVProph(i, false, false, true);
            setInfantHIVProphMajorToxicity(i,false);
		}	
	} // end of Pediatric initial breastfeeding, maternal, and HIV states
	else {
		if (simContext->getHIVTestInputs()->enableHIVTesting) {
			/** For adults: */
			/** - Roll for initial HIV state from distribution if using HIV testing module */
			double randNum = CepacUtil::getRandomDouble(90020, patient);
			for (int i = 0; i < SimContext::HIV_INF_NUM; i++) {
				if ((simContext->getHIVTestInputs()->initialHIVDistribution[i] != 0) &&
					(randNum < simContext->getHIVTestInputs()->initialHIVDistribution[i])) {
						infectedState = (SimContext::HIV_INF) i;
						break;
				}
				randNum -= simContext->getHIVTestInputs()->initialHIVDistribution[i];
			}

			/** - If patient was preset to not being a prevalent case (likely from transmission model) set them to HIV negative state */
			if (patient->getGeneralState()->predefinedAgeAndGender && !patient->getDiseaseState()->isPrevalentHIVCase){
				//Set non-prevalent cases to HIV Negative
				infectedState = (SimContext::HIV_INF_NEG);
			}
		}
		else {
			/** - Always use chronic HIV if HIV testing module is disabled */
			infectedState = SimContext::HIV_INF_ASYMP_CHR_POS;
			setCareState(SimContext::HIV_CARE_IN_CARE);
		}

		//Roll for risk state
		double randNum = CepacUtil::getRandomDouble(90020, patient);
		if (infectedState == SimContext::HIV_INF_NEG &&
				randNum < simContext->getHIVTestInputs()->initialRiskDistribution[SimContext::HIV_BEHAV_LO])
			isHighRisk = false;

		/** isInitial is true if this is a prevalent case (was true by default, but this changed with transmission model's need to predefine incidence cases) */
		bool isInitial = !(patient->getGeneralState()->predefinedAgeAndGender && !patient->getDiseaseState()->isPrevalentHIVCase);
		setInfectedHIVState(infectedState, isInitial, isHighRisk, true);
		if (infectedState == SimContext::HIV_INF_NEG)
			setCareState(SimContext::HIV_CARE_NEG);

	} // end of adult initial HIV state

	/** Set the initial CD4 percentage for early childhood if HIV positive*/
	if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE) {
		/** Set the initial true CD4 percentage and strata for HIV positive infants */
		/** If HIV-negative, CD4 will be initialized to 0 but should not be used unless they become HIV+ */
		double cd4PercMean = 0;
		double cd4PercStdDev = 0;
		if (pedsHIVState == SimContext::PEDS_HIV_POS_IU) {
			cd4PercMean = simContext->getPedsInputs()->initialCD4PercentageIUMean;
			cd4PercStdDev = simContext->getPedsInputs()->initialCD4PercentageIUStdDev;
		}
		else if (pedsHIVState == SimContext::PEDS_HIV_POS_IP) {
			cd4PercMean = simContext->getPedsInputs()->initialCD4PercentageIPMean;
			cd4PercStdDev = simContext->getPedsInputs()->initialCD4PercentageIPStdDev;
		}
		double cd4PercValue = CepacUtil::getRandomGaussian(cd4PercMean, cd4PercStdDev, 90030, patient);
		setTrueCD4Percentage(cd4PercValue, true);
	}
	/** Set the initial true CD4 value and strata for adults and late childhood if HIV positive*/
	/** If HIV-negative, CD4 will be initialized to 0 but should not be used unless they become HIV+ */
	else {
		double cd4Mean = 0;
		double cd4StdDev = 0;
		double cd4Value = -1;
		if (infectedState == SimContext::HIV_INF_ACUTE_SYN) {
			cd4Mean = simContext->getHIVTestInputs()->initialAcuteCD4DistributionMean;
			cd4StdDev = simContext->getHIVTestInputs()->initialAcuteCD4DistributionStdDev;
		}
		else if (infectedState == SimContext::HIV_INF_ASYMP_CHR_POS) {
			cd4Mean = simContext->getCohortInputs()->initialCD4Mean;
			cd4StdDev = simContext->getCohortInputs()->initialCD4StdDev;
		}
		int count = 0;
		// If we are using the square root transformation the drawn value is squared to get the actual CD4
		if (simContext->getCohortInputs()->enableSquareRootTransform){
			while ((cd4Value < 0) || ((simContext->getRunSpecsInputs()->maxPatientCD4 != SimContext::NOT_APPL) && (cd4Value*cd4Value > simContext->getRunSpecsInputs()->maxPatientCD4))){
				cd4Value = CepacUtil::getRandomGaussian(cd4Mean, cd4StdDev, 90040+count, patient);
				count++;
			}
			cd4Value = cd4Value * cd4Value;
		}
		else{
			while ((cd4Value < 0) || ((simContext->getRunSpecsInputs()->maxPatientCD4 != SimContext::NOT_APPL) && (cd4Value > simContext->getRunSpecsInputs()->maxPatientCD4))){
				cd4Value = CepacUtil::getRandomGaussian(cd4Mean, cd4StdDev, 90040+count, patient);
				count++;
			}
		}

		setTrueCD4(cd4Value, true);
	}

	/** Set the initial HVL strata if HIV positive, get distribution for pediatric early childhood  or adult/pediatric late childhood */
	const double *hvlDist = NULL;
	if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE) {
		if (pedsHIVState == SimContext::PEDS_HIV_POS_IU) {
			hvlDist = simContext->getPedsInputs()->initialHVLDistributionIU;
		}
		else if (pedsHIVState == SimContext::PEDS_HIV_POS_IP) {
			hvlDist = simContext->getPedsInputs()->initialHVLDistributionIP;
		}
	}
	// After early childhood, draw initial HVL from adult inputs
	else {
		if (infectedState == SimContext::HIV_INF_ACUTE_SYN) {
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			hvlDist = simContext->getHIVTestInputs()->initialAcuteHVLDistribution[cd4Strata];
		}
		else if (infectedState == SimContext::HIV_INF_ASYMP_CHR_POS) {
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			hvlDist = simContext->getCohortInputs()->initialHVLDistribution[cd4Strata];
		}
	}
	if (hvlDist != NULL) {
		SimContext::HVL_STRATA hvlStrata = SimContext::HVL__LO;
		double randNum = CepacUtil::getRandomDouble(90050, patient);
		for (int i = SimContext::HVL_NUM_STRATA - 1; i >= 0; i--) {
			if ((hvlDist[i] != 0) && (randNum < hvlDist[i])) {
				hvlStrata = (SimContext::HVL_STRATA) i;
				break;
			}
			randNum -= hvlDist[i];
		}
		setTrueHVLStrata(hvlStrata);
		setSetpointHVLStrata(hvlStrata);
		setTargetHVLStrata(hvlStrata);
	}

	/** Set the patients Transmission risk category */
	int ageCat = getAgeCategoryTransmRisk(patient->getGeneralState()->ageMonths);
	SimContext::TRANSM_RISK transmRisk = SimContext::TRANSM_RISK_OTHER;
	double randNum = CepacUtil::getRandomDouble(60055, patient);
	for (int i = 0; i < SimContext::TRANSM_RISK_NUM; i++){
		double distValue = simContext->getCohortInputs()->transmRiskDistrib[patient->getGeneralState()->gender][ageCat][i];
		if (randNum < distValue){
			transmRisk = (SimContext::TRANSM_RISK) i;
			break;
		}
		randNum -= distValue;
	}
	setTransmRiskCategory(transmRisk);

	setInitialPrEPParams();
	/** Roll for initially on PrEP */
	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG &&
			simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP && !patient->getGeneralState()->isPediatric){
		randNum =  CepacUtil::getRandomDouble(60055, patient);
		SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
		if (randNum < simContext->getHIVTestInputs()->PrEPInitialDistribution[risk]){
			setPrEP(true);
			incrementCostsPrEP(simContext->getHIVTestInputs()->costPrEPInitial[risk], 1.0);
		}
		else{
			setPrEP(false);
		}
	}
	else
		setPrEP(false);
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void HIVInfectionUpdater::performMonthlyUpdates() {
	/** If the patient is pediatric, determine maternal updates and early to late childhood transition by calling HIVInfectionUpdater::performPediatricDiseaseUpdates() */
	if (patient->getGeneralState()->isPediatric) {
		performPediatricDiseaseUpdates();
	}
	/** If the patient is an adolescent, perform updates and age-based transitions specific to adolescents*/
	if (patient->getGeneralState()->isAdolescent){
		performAdolescentDiseaseUpdates();
	}
	/** determines if HIV negative patients become infected  by calling HIVInfectionUpdater::performHIVNegativeUpdates() */
	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
		performHIVNegativeUpdates();
	}

	/** Perform acute to chronic transition if this should occur this month by calling HIVInfectionUpdater::performAcuteToChronicHIVUpdates() */
	if ((patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_ACUTE_SYN) &&
		(patient->getDiseaseState()->monthOfAcuteToChronicHIV == patient->getGeneralState()->monthNum)) {
			performAcuteToChronicHIVUpdates();
	}
} /* end performMonthlyUpdates */

/** \brief performHIVNegativeUpdates determines if HIV negative patients become infected */
void HIVInfectionUpdater::performHIVNegativeUpdates() {
	//Count number of uninfected
	updateStartMonthHIVNeg();

	//Roll for dropout PrEP - currently, those who drop out cannot rejoin, and patients who are infected on PrEP but not yet detected cannot drop out. Adjustments will be needed to the code related to PrEP dropouts if we begin to allow dropouts to rejoin or HIV+ patients still on PrEP to drop out.
	if (simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP &&
				patient->getMonitoringState()->hasPrEP){
		SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
		double probDropout;
		// the dropout threshold month is relative to a reference month, either Month 0 (simulation start) or the month of PrEP start, based on user input
		if (patient->getGeneralState()->monthNum <= patient->getMonitoringState()->PrEPDropoutThresholdMonth)
			probDropout = simContext->getHIVTestInputs()->PrEPDropoutPreThreshold[risk];

		else
			probDropout = simContext->getHIVTestInputs()->PrEPDropoutPostThreshold[risk];

		double randNum = CepacUtil::getRandomDouble(90060, patient);
		if (randNum < probDropout) {
			setPrEP(false, true);

			/** - Print tracing information for dropout PrEP */
			if (patient->getGeneralState()->tracingEnabled)
				tracer->printTrace(1, "**%d DROPOUT PrEP;\n", patient->getGeneralState()->monthNum);
		}
	}

	//Roll for start PrEP
	SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
	if (simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP && 
	!patient->getMonitoringState()->everPrEP && 
	(simContext->getHIVTestInputs()->PrEPAfterRollout[risk] || patient->getGeneralState()->monthNum < simContext->getHIVTestInputs()->PrEPRolloutDuration[risk])){

		double probPrEP;
		double coverage = simContext->getHIVTestInputs()->PrEPCoverage[risk];
		int duration = simContext->getHIVTestInputs()->PrEPRolloutDuration[risk];
		double shape = simContext->getHIVTestInputs()->PrEPShape[risk];
		int month = patient->getGeneralState()->monthNum;

		//prob from Weibull distribution
		probPrEP = 1-pow(1-coverage,(pow(month+1,shape)-pow(month,shape))/pow(duration,shape));
		//log probability of age-eligible patients joining
        updatePrEPProbLogging(probPrEP, risk);
		if(!patient->getGeneralState()->isPediatric){
			double randNum = CepacUtil::getRandomDouble(90060, patient);
			if (randNum < probPrEP) {
				setPrEP(true);

				incrementCostsPrEP(simContext->getHIVTestInputs()->costPrEPInitial[risk], 1.0);

				/** - Print tracing information for starting PrEP */
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d START PrEP;\n", patient->getGeneralState()->monthNum);
				}//if we're tracing the patient
			} //if we started PrEP
		} //if patient is age-eligible for PrEP	
	}
	/** If Adult or Adolescent, roll for adult HIV infection */
	if ((patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT)|| patient->getGeneralState()->isAdolescent) {
		
		int ageCategory = patient->getGeneralState()->ageCategoryHIVInfection;
		SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
		SimContext::HIV_BEHAV riskType = patient->getMonitoringState()->isHighRiskForHIV ? SimContext::HIV_BEHAV_HI : SimContext::HIV_BEHAV_LO;
		double randNum = CepacUtil::getRandomDouble(90060, patient);
		double probInfect;

		if (patient->getMonitoringState()->hasPrEP)
			probInfect = simContext->getHIVTestInputs()->PrEPIncidence[gender][ageCategory][riskType];
		else
			probInfect = simContext->getHIVTestInputs()->probHIVInfection[gender][ageCategory][riskType];

		//Reduce incidence by global incidence reduction; multipliers accumulate based on 6 stage bounds. First stage bound is always 1. After the 6th bound month, no new multipliers are accumulated

		if (simContext->getHIVTestInputs()->useHIVIncReductionMult){	
			if(patient->getGeneralState()->monthNum <= simContext->getHIVTestInputs()->HIVIncReducStageBounds[SimContext::INC_REDUC_PERIODS_NUM - 1]){	
				// accumulate a new multplier if a new stage bound was reached this month
				double newReducMult = patient->getMonitoringState()->HIVIncReducMultiplier;
				for(int i = 0; i < SimContext::INC_REDUC_PERIODS_NUM; i++){
					if(patient->getGeneralState()->monthNum == simContext->getHIVTestInputs()->HIVIncReducStageBounds[i]){
						newReducMult *= simContext->getHIVTestInputs()->HIVIncReducMultipliers[i];
						setHIVIncReducMultiplier(newReducMult);
						break;
					}
				}
			}
			probInfect *= patient->getMonitoringState()->HIVIncReducMultiplier;
		}
	
		//Roll for infection
		if (randNum < probInfect) {
			/** - If infected, initialize the new infection by calling HIVInfectionUpdater::performHIVNewInfectionUpdates() */
			performHIVNewInfectionUpdates();
		}
	}
	// Pediatric infection
	else {
		/** Roll for infant PP HIV infection (aka PP Vertical Transmission of HIV, or PP VTHIV) which only occurs during bf */
		// No pediatric patients are infected between the end of bf and the transition to either Adolescent or Adult
		SimContext::PEDS_MATERNAL_STATUS momHIVState = patient->getPedsState()->maternalStatus;
		if ((patient->getGeneralState()->ageMonths < patient->getPedsState()->breastfeedingStopAge) &&patient->getPedsState()->isMotherAlive && (momHIVState != SimContext::PEDS_MATERNAL_STATUS_NEG)){
			SimContext::PEDS_BF_TYPE bfType = patient->getPedsState()->breastfeedingStatus;
			double randNum = CepacUtil::getRandomDouble(90065, patient);
			bool isIncident = false;
			if (bfType != SimContext::PEDS_BF_REPL){
				double probVTHIV;
				if (patient->getPedsState()->motherOnART){
					if (patient->getPedsState()->motherOnSuppressedART)
						probVTHIV = simContext->getPedsInputs()->probVTHIVPPMotherOnARTSuppressed[momHIVState];
					else{
						if (patient->getPedsState()->motherLowHVL)
							probVTHIV = simContext->getPedsInputs()->probVTHIVPPMotherOnARTNotSuppressedLowHVL[momHIVState];

						else
							probVTHIV = simContext->getPedsInputs()->probVTHIVPPMotherOnARTNotSuppressedHighHVL[momHIVState];
					}
				}
				else
					probVTHIV = simContext->getPedsInputs()->probVTHIVPPMotherOffART[momHIVState][bfType];

				//Modify by infant proph - when the patient is within the duration for multiple lines of infant prophylaxis, the efficacy multipliers are applied cumulatively to the probability of VTHIV
				for (int i = 0; i < SimContext::INFANT_HIV_PROPHS_NUM; ++i){
					if (patient->getPedsState()->hasEffectiveInfantHIVProph[i]){
						const SimContext::EIDInputs::InfantHIVProph &infantProph = simContext->getEIDInputs()->infantHIVProphs[i];

						int timeSinceProph = patient->getGeneralState()->monthNum - patient->getPedsState()->monthOfEffectiveInfantHIVProph[i];
						//First check if the proph dose has any remaining efficacy; skip if not
						if(timeSinceProph > infantProph.infantProphEffHorizon + infantProph.infantProphDecayTime){
							continue;
						}
						//Find the multiplier defined for the patient age and maternal status
						int period = 0;
						if (patient->getGeneralState()->ageMonths >= infantProph.infantProphVTHIVPPMultAgeThreshold)
							period = 1;
						double VTHIVmult;
						if (patient->getPedsState()->motherOnART){
							if (patient->getPedsState()->motherOnSuppressedART)
								VTHIVmult = infantProph.infantProphVTHIVPPMultMotherOnARTSuppressed[momHIVState][period];
							else{
								if (patient->getPedsState()->motherLowHVL)
									VTHIVmult = infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedLowHVL[momHIVState][period];
								else
									VTHIVmult = infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedHighHVL[momHIVState][period];
							}
						}
						else
							VTHIVmult = infantProph.infantProphVTHIVPPMultMotherOffART[momHIVState][bfType][period];
						//Check if in efficacy time horizon	
						if (timeSinceProph <= infantProph.infantProphEffHorizon)
							probVTHIV = CepacUtil::probRateMultiply(probVTHIV, VTHIVmult);
						else if(timeSinceProph <= infantProph.infantProphEffHorizon + infantProph.infantProphDecayTime){
							int timeSinceDecay = timeSinceProph - infantProph.infantProphEffHorizon;
							double propDecay = (double) timeSinceDecay / infantProph.infantProphDecayTime;
							probVTHIV = CepacUtil::probRateMultiply(probVTHIV, (VTHIVmult + propDecay*(1-VTHIVmult)));
						}
					}
				}
				if (randNum < probVTHIV)
					isIncident=true;
			}//end patient is breastfeeding

			if (isIncident) {
				/** - If infected, initialize the new infection by calling HIVInfectionUpdater::performHIVNewInfectionUpdates() */
				performHIVNewInfectionUpdates();
			}//end we infected a pediatric patient
		}//end mom is HIV positive and patient is of breastfeeding age
	}//end we're dealing with a pediatric patient
} /* end performHIVNegativeUpdates */

/** \brief performHIVNewInfectionUpdates handles the new infection of an HIV negative patient
 *
 *  Note: This functions assumes that an HIV infection is scheduled to occur */
void HIVInfectionUpdater::performHIVNewInfectionUpdates(){
	/** Don't do anything if the patient is already infected */
	if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
		return;
	}
	/** Use one set of infection rules for adults and adolescents, and another for Peds early */
	if ((patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT) || patient->getGeneralState()->isAdolescent){
		/** For adults and adolescents:
		/* - Update the Patient's infected state to acute infected */
		setInfectedHIVState(SimContext::HIV_INF_ACUTE_SYN, false, false, true);
		setDetectedHIVState(false);

		/** - Set the CD4 level for an incident case of acute HIV infection */
		double cd4Mean = simContext->getHIVTestInputs()->initialAcuteCD4DistributionMean;
		double cd4StdDev = simContext->getHIVTestInputs()->initialAcuteCD4DistributionStdDev;
		double cd4Value = -1;
		int count = 0;
		if (simContext->getCohortInputs()->enableSquareRootTransform){
			while ((cd4Value < 0) || ((simContext->getRunSpecsInputs()->maxPatientCD4 != SimContext::NOT_APPL) && (cd4Value*cd4Value > simContext->getRunSpecsInputs()->maxPatientCD4))){
				cd4Value = CepacUtil::getRandomGaussian(cd4Mean, cd4StdDev, 90067+count, patient);
				count++;
			}
			cd4Value = cd4Value * cd4Value;
		}
		else{
			while ((cd4Value < 0) || ((simContext->getRunSpecsInputs()->maxPatientCD4 != SimContext::NOT_APPL) && (cd4Value > simContext->getRunSpecsInputs()->maxPatientCD4))){
				cd4Value = CepacUtil::getRandomGaussian(cd4Mean, cd4StdDev, 90068+count, patient);
				count++;
			}
		}
		setTrueCD4(cd4Value, true);

		/** - Set the HVL level for an incident case of acute HIV infection */
		SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
		double randNum = CepacUtil::getRandomDouble(90080, patient);
		for (int i = SimContext::HVL_NUM_STRATA - 1; i >= 0; i--) {
			if ((simContext->getHIVTestInputs()->initialAcuteHVLDistribution[cd4Strata][i] != 0) &&
				(randNum < simContext->getHIVTestInputs()->initialAcuteHVLDistribution[cd4Strata][i])) {
					setTrueHVLStrata((SimContext::HVL_STRATA) i);
					setSetpointHVLStrata((SimContext::HVL_STRATA) i);
					setTargetHVLStrata((SimContext::HVL_STRATA) i);
					break;
			}//If we choose to set to this HVL Strata...
			randNum -= simContext->getHIVTestInputs()->initialAcuteHVLDistribution[cd4Strata][i];
		}//end for loop through all HVL strata

		/** - Update the initial distributions at time of infection statistics */
		updateInitialDistributions();

		/** - Print tracing information for infection and the initial CD4/HVL */
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d HIV INFECTION;\n", patient->getGeneralState()->monthNum);
			tracer->printTrace(1, "  %d init CD4: %1.0f;\n", patient->getGeneralState()->monthNum,
				patient->getDiseaseState()->currTrueCD4);
			tracer->printTrace(1, "  %d init HVL: %s, setpt: %s;\n", patient->getGeneralState()->monthNum,
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->currTrueHVLStrata],
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
		}//end if we're tracing the patient
	}//end if patient is an adult or adolescent
	/** Or process a Pediatric infection: only occurs in early childhood, during bf*/
	else{
		//If they were false positive they are now true positive
		if (patient->getPedsState()->isFalsePositive){
			setFalsePositiveStatus(false, false);
			if (patient->getGeneralState()->tracingEnabled)
				tracer->printTrace(1, "**%d EID patient no longer false positive\n", patient->getGeneralState()->monthNum);
		}

		/** - Update pediatrics and regular HIV state to positive */
		setInfectedPediatricsHIVState(SimContext::PEDS_HIV_POS_PP, false);
		setInfectedHIVState(SimContext::HIV_INF_ACUTE_SYN, false, false, true);
		setDetectedHIVState(false);

		/** - Set the initial CD4 percentage for the newly infected infant */
		int ageCatInfant = getAgeCategoryInfant(patient->getGeneralState()->ageMonths);
		double percCD4Mean = simContext->getPedsInputs()->initialCD4PercentagePPMean[ageCatInfant];
		double percCD4StdDev = simContext->getPedsInputs()->initialCD4PercentagePPStdDev[ageCatInfant];
		double percCD4 = CepacUtil::getRandomGaussian(percCD4Mean, percCD4StdDev, 90066, patient);
		setTrueCD4Percentage(percCD4, true);

		/** - Set the initial childhood HVL setpoint */
		SimContext::HVL_STRATA hvlStrata = SimContext::HVL_VLO;
		double randNum = CepacUtil::getRandomDouble(90067, patient);
		for (int i = SimContext::HVL_NUM_STRATA - 1; i >= 0; i--) {
			if ((simContext->getPedsInputs()->initialHVLDistributionPP[ageCatInfant][i] != 0) &&
				(randNum < simContext->getPedsInputs()->initialHVLDistributionPP[ageCatInfant][i])) {
					hvlStrata = (SimContext::HVL_STRATA) i;
					break;
			}
			randNum -= simContext->getPedsInputs()->initialHVLDistributionPP[ageCatInfant][i];
		}
		setTrueHVLStrata(hvlStrata);
		setSetpointHVLStrata(hvlStrata);
		setTargetHVLStrata(hvlStrata);

		/** - Print tracing information for infection and the initial CD4/HVL */
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d HIV INFECTION;\n", patient->getGeneralState()->monthNum);
			tracer->printTrace(1, "  %d init CD4 perc: %1.3f;\n", patient->getGeneralState()->monthNum,
				patient->getDiseaseState()->currTrueCD4Percentage);
			tracer->printTrace(1, "  %d init HVL: %s, setpt: %s;\n", patient->getGeneralState()->monthNum,
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->currTrueHVLStrata],
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
		}
	}//end if patient is pediatric

}/* end performHIVNewInfectionUpdates*/

/** \brief performAcuteToChronicHIVUpdates handles the transition from acute to chronic HIV */
void HIVInfectionUpdater::performAcuteToChronicHIVUpdates() {
	/** Set the infection state based on the prior OI history (symptomatic or asymptomatic)*/
	if (patient->getDiseaseState()->typeTrueOIHistory == SimContext::HIST_EXT_N) {
		setInfectedHIVState(SimContext::HIV_INF_ASYMP_CHR_POS, false, false, false);
	}
	else {
		setInfectedHIVState(SimContext::HIV_INF_SYMP_CHR_POS, false, false, false);
	}

	if (!patient->getGeneralState()->isPediatric){
		if(simContext->getHIVTestInputs()->enableHIVTesting){
			SimContext::HVL_STRATA hvlStrata = patient->getDiseaseState()->currTrueHVLStrata;
			/** Update the true CD4 levels according to the chronic transition parameters */
			double cd4ChangeMean = simContext->getHIVTestInputs()->CD4ChangeAtChronicHIVMean[hvlStrata];
			double cd4ChangeStdDev = simContext->getHIVTestInputs()->CD4ChangeAtChronicHIVStdDev[hvlStrata];
			double cd4Change = CepacUtil::getRandomGaussian(cd4ChangeMean, cd4ChangeStdDev, 90090, patient);
			setTrueCD4(patient->getDiseaseState()->currTrueCD4 + cd4Change);
			
			/** Update setpoint HVL level based on the specified distribution of current to new levels */
			SimContext::HVL_STRATA hvlSetpoint = patient->getDiseaseState()->setpointHVLStrata;
			double randNum = CepacUtil::getRandomDouble(90100, patient);
			for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
				if ((simContext->getHIVTestInputs()->HVLDistributionAtChronicHIV[hvlSetpoint][i] > 0) &&
					(randNum < simContext->getHIVTestInputs()->HVLDistributionAtChronicHIV[hvlSetpoint][i])) {
						hvlSetpoint = (SimContext::HVL_STRATA) i;
						setSetpointHVLStrata(hvlSetpoint);
						break;
				}
				randNum -= simContext->getHIVTestInputs()->HVLDistributionAtChronicHIV[hvlSetpoint][i];
			}

			/** If patient is not on ART or is on failed ART, set true and target HVL to the setpoint level */
			if (!patient->getARTState()->isOnART || (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_FAILURE)) {
				setTrueHVLStrata(hvlSetpoint);
				setTargetHVLStrata(hvlSetpoint);
			}
			/** Print out trace information about transition for those in adult age category*/
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d HIV ACUTE TO CHR: CD4 %1.0f, HVLsetpt %s;\n",
					patient->getGeneralState()->monthNum, patient->getDiseaseState()->currTrueCD4,
					SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
			}
		}
	}	
	/** Print out separate trace information about transition for those in late childhood or early childhood age categories, for whom the transition has no clinical meaning*/
	else if (patient->getGeneralState()->tracingEnabled){
		if(patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			tracer->printTrace(1, "**%d PEDIATRIC HIV ACUTE TO CHR: CD4 %1.0f, HVLsetpt %s;\n",
				patient->getGeneralState()->monthNum, patient->getDiseaseState()->currTrueCD4,
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
		}
		else{ 
			tracer->printTrace(1, "**%d PEDIATRIC HIV ACUTE TO CHR: HVLsetpt %s;\n",
				patient->getGeneralState()->monthNum,
				SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
		}
	}
} /* end performAcuteToChronicHIVUpdates */

/** \brief performPediatricDiseaseUpdates determines maternal updates, early to late childhood transition, and late childhood to adolescent or adult transitions */
void HIVInfectionUpdater::performPediatricDiseaseUpdates() {	
	if (patient->getPedsState()->isMotherAlive) {
		/** Update breastfeeding status if the age at which to stop breastfeeding or age of exclusive/mixed to complementary transition is reached */
		SimContext::PEDS_BF_TYPE bfType = patient->getPedsState()->breastfeedingStatus;
		if (patient->getGeneralState()->ageMonths == patient->getPedsState()->breastfeedingStopAge && bfType != SimContext::PEDS_BF_REPL) {
			setBreastfeedingStatus(SimContext::PEDS_BF_REPL);
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d PEDS BREASTFEEDING STATUS: %s\n",  patient->getGeneralState()->monthNum, SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus]);
			}
		}
		else if ((bfType == SimContext::PEDS_BF_EXCL || bfType == SimContext::PEDS_BF_MIXED) && (patient->getGeneralState()->ageMonths == SimContext::PEDS_BF_STOP_EXCL_MIXED)) {
			setBreastfeedingStatus(SimContext::PEDS_BF_COMP);
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d PEDS BREASTFEEDING STATUS: %s\n",  patient->getGeneralState()->monthNum, SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus]);
			}
		}

		/** Handle transitions in maternal HIV infection state */
		SimContext::PEDS_MATERNAL_STATUS momHIVState = patient->getPedsState()->maternalStatus;
		if (momHIVState == SimContext::PEDS_MATERNAL_STATUS_NEG) {
			/** Roll for incident maternal infection if HIV-negative */
			rollForMaternalInfection();
		}
		if ((momHIVState == SimContext::PEDS_MATERNAL_STATUS_ACUTE) &&
			(patient->getGeneralState()->monthNum == patient->getPedsState()->monthOfMaternalHIVInfection + SimContext::PEDS_MOM_MTHS_ACUTE)) {
				/** If mom's acute time period is reached, transition to chronic CD4>350 (chronic high)*/
				setMaternalHIVState(SimContext::PEDS_MATERNAL_STATUS_CHR_HIGH, false, false);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d MATERNAL TRANSITION CHRONIC HIV: %s\n",  patient->getGeneralState()->monthNum, SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus]);
			}
		}

		/** Roll for maternal mortality */
		momHIVState = patient->getPedsState()->maternalStatus;
		double randNum = CepacUtil::getRandomDouble(90106, patient);
		if (randNum < simContext->getPedsInputs()->probMaternalDeath[momHIVState]) {
			setMaternalDeath();
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d MATERNAL DEATH\n", patient->getGeneralState()->monthNum);
			}
		}
		// Update HIV never exposed status based on the updated maternal and patient status
		if(patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE &&patient->getDiseaseState()->infectedPediatricsHIVState == SimContext::PEDS_HIV_NEG){
			updatePedsNeverExposed();
		}	
		//Roll for status becoming known during BF
		if (patient->getPedsState()->isMotherAlive && (patient->getPedsState()->maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG)
			&& (patient->getPedsState()->breastfeedingStatus != SimContext::PEDS_BF_REPL)){
			double randNum = CepacUtil::getRandomDouble(90108, patient);
			if (!patient->getPedsState()->maternalStatusKnown && randNum < simContext->getPedsInputs()->probMotherStatusBecomeKnown[momHIVState]){
				setMaternalStatusKnown(true);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d MATERNAL HIV Status Known\n", patient->getGeneralState()->monthNum);
				}
			}
		}

		//Roll for Maternal ART status among breastfeeding women if known
		if (simContext->getPedsInputs()->usePPMaternalARTStatus &&
				patient->getPedsState()->isMotherAlive &&
				(patient->getPedsState()->maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG) &&
				(patient->getPedsState()->breastfeedingStatus != SimContext::PEDS_BF_REPL) &&
				patient->getPedsState()->maternalStatusKnown){

			int ageCatPP = patient->getGeneralState()->ageMonths;
			if (ageCatPP >= SimContext::PEDS_PP_MATERNAL_ART_STATUS_NUM)
				ageCatPP = SimContext::PEDS_PP_MATERNAL_ART_STATUS_NUM -1;

			//Roll for maternal suppression status known
			randNum = CepacUtil::getRandomDouble(90109, patient);
			bool suppressionKnown = false;
			if (randNum < simContext->getPedsInputs()->ppMaternalARTStatusProbSuppressionKnown[ageCatPP])
				suppressionKnown = true;

			randNum = CepacUtil::getRandomDouble(90109, patient);
			//Roll for mother being on suppressed ART
			if (randNum < simContext->getPedsInputs()->ppMaternalARTStatusProbSuppressed[ageCatPP]){
				setMaternalARTStatus(true, true, suppressionKnown, false);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d MOTHER ON SUPPRESSED ART\n", patient->getGeneralState()->monthNum);
				}
			}
			//If mother is not on suppressed ART, the other options are on ART unsuppressed with either Low or High HVL and off ART
			else{
				randNum -= simContext->getPedsInputs()->ppMaternalARTStatusProbSuppressed[ageCatPP];
				if(randNum < simContext->getPedsInputs()->ppMaternalARTStatusProbNotSuppressedLowHVL[ageCatPP]){
					setMaternalARTStatus(true, false, suppressionKnown,false);
					setMaternalHVLStatus(true);
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d MOTHER ON NON SUPPRESSED ART LOW HVL\n", patient->getGeneralState()->monthNum);
					}
				}
				else{
					randNum -=simContext->getPedsInputs()->ppMaternalARTStatusProbNotSuppressedLowHVL[ageCatPP];
					if (randNum < simContext->getPedsInputs()->ppMaternalARTStatusProbNotSuppressedHighHVL[ageCatPP]){
						setMaternalARTStatus(true, false, suppressionKnown,false);
						setMaternalHVLStatus(false);
						if (patient->getGeneralState()->tracingEnabled) {
							tracer->printTrace(1, "**%d MOTHER ON NON SUPPRESSED ART HIGH HVL\n", patient->getGeneralState()->monthNum);
						}
					}
					else{
						setMaternalARTStatus(false, false, false, false);
						if (patient->getGeneralState()->tracingEnabled) {
							tracer->printTrace(1, "**%d MOTHER OFF ART\n", patient->getGeneralState()->monthNum);
						}
					}
				}
			}

			//Roll for stop BF upon status known not supp
			if (patient->getPedsState()->motherOnART && !patient->getPedsState()->motherOnSuppressedART && patient->getPedsState()->motherSuppressionKnown){
				randNum = CepacUtil::getRandomDouble(90009, patient);
				double prob = 0;
				if (patient->getPedsState()->motherLowHVL)
					prob = simContext->getPedsInputs()->probStopBFNotSuppressedLowHVL;
				else
					prob = simContext->getPedsInputs()->probStopBFNotSuppressedHighHVL;

				if (randNum < prob){
					setBreastfeedingStatus(SimContext::PEDS_BF_REPL);
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d PEDS BREASTFEEDING STATUS: %s\n",  patient->getGeneralState()->monthNum, SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus]);
					}
				}
			} 
		} // end of updates to maternal ART and breastfeeding status if using PP maternal ART status and mother HIV+
	} //end if mother is alive at the beginning of the month

	/** Check for receiving HIV infant proph if not linked to care - can't receive it if they have switched to adult HIV tests due to the effects on sensitivity and specificity of HIV tests */
	bool ageEligible = false;
	if ((!simContext->getHIVTestInputs()->enableHIVTesting || patient->getGeneralState()->ageMonths < simContext->getHIVTestInputs()->HIVRegularTestingStartAge))
		ageEligible = true;
	if (simContext->getEIDInputs()->enableHIVTestingEID && ageEligible && (patient->getMonitoringState()->careState < SimContext::HIV_CARE_IN_CARE)){
		// For each Infant HIV Prophylaxis regimen
        for (int i = 0; i < SimContext::INFANT_HIV_PROPHS_NUM; ++i){
           	const SimContext::EIDInputs::InfantHIVProph &infantProph = simContext->getEIDInputs()->infantHIVProphs[i];
			//don't do anything if proph not enabled
            if (!infantProph.infantProphEnabled) 
				continue;
			// Check for receiving a dose of infant HIV prophylaxis if patient is breastfeeding
			if(patient->getPedsState()->isMotherAlive &&
			patient->getPedsState()->breastfeedingStatus != SimContext::PEDS_BF_REPL){
				int ageCatProph = patient->getGeneralState()->ageMonths;
				if (ageCatProph >= SimContext::INFANT_PROPH_AGES_NUM)
					ageCatProph = SimContext::INFANT_PROPH_AGES_NUM-1;

				double randNum = CepacUtil::getRandomDouble(90009, patient);
				//Roll for getting infant proph, if so check EID policy
				if (randNum < infantProph.infantProphProb[ageCatProph]){
					int negEIDMonths = infantProph.infantProphNegEIDMonths[ageCatProph];
					bool passesEID = false;
					// Check test requirements, starting with whether a positive EID test result will disqualify them, then whether a negative result is required
                	if (!infantProph.infantProphEligibilityStopOnPosEID || !patient->getPedsState()->hasMostRecentPositiveEIDTest){
                   		if (negEIDMonths==SimContext::NOT_APPL)
                        	passesEID = true;
                   		else{
							if(patient->getPedsState()->hasMostRecentNegativeEIDTest){
								int timeSinceNegEID = patient->getGeneralState()->monthNum - patient->getPedsState()->monthOfMostRecentNegativeEIDTest;
								if (timeSinceNegEID <= negEIDMonths)
									passesEID = true;
							}
						}
					}
					
					// Check major tox eligibility
					if (passesEID && evaluatePedsHIVProphPolicy(infantProph) &&
						!(infantProph.infantProphMajorToxStopOnTox && patient->getPedsState()->hasInfantHIVProphMajorToxicity[i])){
						//Add cost
						int costAgeCat = getAgeCategoryInfantHIVProphCost(patient->getGeneralState()->ageMonths);
						if (!patient->getPedsState()->everInfantHIVProph[i])
							incrementCostsInfantHIVProphDirect(infantProph.infantProphStartupCost[costAgeCat]);
						incrementCostsInfantHIVProphDirect(infantProph.infantProphDoseCost[costAgeCat]);
	
						//Tox only happens on the first dose, but the death rate ratio has a duration
						if (!patient->getPedsState()->everInfantHIVProph[i]){

                        	/** Infant Proph Toxicities - both major and minor may occur */
							double probMajorTox = infantProph.infantProphMajorToxProb;
							double probMinorTox = infantProph.infantProphMinorToxProb;
	
							/** Roll for major toxicity occurring */
							double randNum = CepacUtil::getRandomDouble(90009, patient);
							if (randNum < probMajorTox) {
								setInfantHIVProphMajorToxicity(i,true);
								accumulateQOLModifier(infantProph.infantProphMajorToxQOLMod);
								incrementCostsInfantHIVProphTox(infantProph.infantProphMajorToxCost);
								// Print tracing information if enabled
								if (patient->getGeneralState()->tracingEnabled) {
									tracer->printTrace(1, "**%d MAJ TOX: INFANT HIV PROPH, %1.2lf	 QAred, $ %1.0lf;\n",
									patient->getGeneralState()->monthNum, patient->getGeneralState()->QOLValue,
									patient->getGeneralState()->costsDiscounted);
								}
							}
							/** Roll for minor toxicity occurring */
                        	randNum = CepacUtil::getRandomDouble(90011, patient);
							if(randNum < probMinorTox){
								accumulateQOLModifier(infantProph.infantProphMinorToxQOLMod); 
								incrementCostsInfantHIVProphTox(infantProph.infantProphMinorToxCost);
								// Print tracing information if enabled
								if (patient->getGeneralState()->tracingEnabled) {
									tracer->printTrace(1, "**%d MIN TOX: INFANT HIV PROPH %d, %1.2lf QAred, $ %1.0lf;\n",
									patient->getGeneralState()->monthNum, i, patient->getGeneralState()->QOLValue,
									patient->getGeneralState()->costsDiscounted);
								}
							}
						}

						//Roll for dose efficacy
						bool isEff = false;
						randNum = CepacUtil::getRandomDouble(90009, patient);
						if (randNum < patient->getPedsState()->probHIVProphEffective[i])
							isEff = true;

						setInfantHIVProph(i, true, isEff);

						if (isEff){
							//Set next dose efficacy
							setInfantHIVProphEffProb(i, patient->getPedsState()->probHIVProphEffective[i] * infantProph.infantProphProbEff);
						}

						if (patient->getGeneralState()->tracingEnabled) {
                        tracer->printTrace(1, "**%d INFANT HIV PROPH %d Administered: Dose %s\n", patient->getGeneralState()->monthNum, i, isEff?"Succeeds":"Fails");
						}
					} //end of administering infant HIV proph dose
				} // end of rolling for whether to get an infant HIV proph dose
			} // end if patient is breastfeeding
			/** Check whether the risk of death from HIV infant proph from a past dose is still active and add the death rate ratio if applicable*/
			if(patient->getPedsState()->hasInfantHIVProphMajorToxicity[i] && patient->getGeneralState()->monthNum - patient->getPedsState()->monthOfInfantHIVProphMajorToxicity[i] < infantProph.infantProphMajorToxDeathRateRatioDuration){
				if(infantProph.infantProphMajorToxDeathRateRatio > 1.0) {
					addMortalityRisk(SimContext::DTH_TOX_INFANT_HIV_PROPH, infantProph.infantProphMajorToxDeathRateRatio, infantProph.infantProphMajorToxDeathCost);
				}	
			} 
		} // end code for this proph line and move on to next
	} // end if EID is enabled 
	/** Handle conversions if this is the month of transition from early to late childhood */
	if ((patient->getDiseaseState()->infectedPediatricsHIVState != SimContext::PEDS_HIV_NEG) &&
		(patient->getGeneralState()->ageMonths == (SimContext::PEDS_YEAR_EARLY * 12))) {
		// If they started at the age category boundary of age 5, there is no need to transition between early and late childhood and it's too soon to transition to adult, so return 
		if(patient->getGeneralState()->monthNum == patient->getGeneralState()->initialMonthNum){
			return;
		}	

		/** - Convert the CD4 percentage to an absolute CD4 value */
		SimContext::PEDS_CD4_PERC cd4PercStrata = patient->getDiseaseState()->currTrueCD4PercentageStrata;
		double cd4Mean = simContext->getPedsInputs()->absoluteCD4TransitionMean[cd4PercStrata];
		double cd4StdDev = simContext->getPedsInputs()->absoluteCD4TransitionStdDev[cd4PercStrata];
		double cd4Value = CepacUtil::getRandomGaussian(cd4Mean, cd4StdDev, 90110, patient);
		setTrueCD4(cd4Value, true);
		// Update CD4 envelope types from CD4 percentage to absolute CD4 count if they are active
		if (patient->getARTState()->overallCD4PercentageEnvelope.isActive){
			setCD4EnvelopeRegimen(SimContext::ENVL_CD4_OVERALL,patient->getARTState()->overallCD4PercentageEnvelope.regimenNum, patient->getARTState()->overallCD4PercentageEnvelope.monthOfStart);
		}
		if (patient->getARTState()->indivCD4PercentageEnvelope.isActive){
			setCD4EnvelopeRegimen(SimContext::ENVL_CD4_INDIV,patient->getARTState()->indivCD4PercentageEnvelope.regimenNum, patient->getARTState()->indivCD4PercentageEnvelope.monthOfStart);
		}
		/** If patient is on ART and suppressed, update the actual CD4 slope */
		if (patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS)) {
			//draw for a CD4 slope
			int monthsSinceRegimenStart = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;

			double cd4Slope = drawCD4Slope(patient->getARTState()->currRegimenNum, monthsSinceRegimenStart);
			setCurrRegimenCD4Slope(cd4Slope);
			}

		// Update the CD4 envelope slopes if active - the individual envelope cannot be active without the overall envelope being active 
		if(patient->getARTState()->overallCD4Envelope.isActive)		
			performCD4EnvelopeAgeTransition(true, patient->getARTState()->indivCD4Envelope.isActive);

		/** - Convert the childhood HVL setpoint to the adult setpoint HVL value */
		SimContext::HVL_STRATA currSetpoint = patient->getDiseaseState()->setpointHVLStrata;
		SimContext::HVL_STRATA newSetpoint = currSetpoint;
		double randNum = CepacUtil::getRandomDouble(90120, patient);
		for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
			if ((simContext->getPedsInputs()->setpointHVLTransition[currSetpoint][i] > 0) &&
				(randNum < simContext->getPedsInputs()->setpointHVLTransition[currSetpoint][i])) {
					newSetpoint = (SimContext::HVL_STRATA) i;
					break;
			}
			randNum -= simContext->getPedsInputs()->setpointHVLTransition[currSetpoint][i];
		}
		setSetpointHVLStrata(newSetpoint);
		/** - If not on ART or on failed ART, also set the true HVL to the new setpoint */
		if (!patient->getARTState()->isOnART || (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_FAILURE)) {
			setTrueHVLStrata(newSetpoint);
			setTargetHVLStrata(newSetpoint);
		}

		/** - Reset observed CD4 percentage and HVL values, the early childhood ones are no longer relevant */
		setObservedCD4Percentage(false);
		setObservedHVLStrata(false);
	}

	/** Handle conversions if this is the month of transition to adolescent model*/
	if (simContext->getAdolescentInputs()->enableAdolescent &&
		(patient->getGeneralState()->ageMonths == simContext->getAdolescentInputs()->ageTransitionFromPeds)){
		// Change their patient status from Pediatric to Adolescent
		setPediatricState(false);
		setAdolescentState(true);

		if(patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d TRANSITION TO ADOLESCENT MODEL: age %d mths (%1.2lf yrs)\n", patient->getGeneralState()->monthNum, patient->getGeneralState()->ageMonths, patient->getGeneralState()->ageMonths / 12.0 );
		/** - Roll for new CD4 slope if on suppressive ART */
		if((patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG) && patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS)) {
			int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
			int artLineNum = patient->getARTState()->currRegimenNum;
			double cd4Slope = drawCD4Slope(artLineNum, monthsOnART);
			setCurrRegimenCD4Slope(cd4Slope);
		}
		// Update CD4 envelope slope(s) if active
		if(patient->getARTState()->overallCD4Envelope.isActive)
			performCD4EnvelopeAgeTransition(true, patient->getARTState()->indivCD4Envelope.isActive);
	}

	/**Handle conversions if this is the month of transition from late childhood to adult and they did not just transition to Adolescent*/
	if (patient->getGeneralState()->ageMonths == (SimContext::PEDS_YEAR_LATE * 12) && !patient->getGeneralState()->isAdolescent){
		// Update their Pediatric patient status to false
		setPediatricState(false);
		if(patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d TRANSITION TO ADULT MODEL: age %d mths (%1.2lf yrs)\n", patient->getGeneralState()->monthNum, patient->getGeneralState()->ageMonths, patient->getGeneralState()->ageMonths / 12.0  );	
		/** - Roll for new CD4 slope if on suppressive ART */
		if((patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG) &&	patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS)) {
			int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
			int artLineNum = patient->getARTState()->currRegimenNum;

			double cd4Slope = drawCD4Slope(artLineNum, monthsOnART);
			setCurrRegimenCD4Slope(cd4Slope);
		}
		// Update the CD4 envelope slopes if active 
		if(patient->getARTState()->overallCD4Envelope.isActive)		
			performCD4EnvelopeAgeTransition(true, patient->getARTState()->indivCD4Envelope.isActive);
	}
} /* end performPediatricDiseaseUpdates */


/** \brief performAdolescentDiseaseUpdates determines adolescent transitions */
void HIVInfectionUpdater::performAdolescentDiseaseUpdates() {
	//Update Response for regimen
	if (patient->getARTState()->isOnART){
		/** Get the current ART regimen information */
		int artLineNum = patient->getARTState()->currRegimenNum;
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(artLineNum);

		//Month of age cat change for AYA ART
		for (int ayaAgeCat = 0; ayaAgeCat < SimContext::ADOLESCENT_NUM_ART_AGES-1; ayaAgeCat++){
			if (patient->getGeneralState()->ageMonths == ayaART->ageBounds[ayaAgeCat]*12){
				/** Determine and set the ART response type for this regimen */
				double responseLogit = patient->getGeneralState()->responseBaselineLogit;
				int hetAgeCat = patient->getGeneralState()->ageCategoryHeterogeneity;

				responseLogit += simContext->getHeterogeneityInputs()->propRespondAge[hetAgeCat];
				SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
				responseLogit += simContext->getHeterogeneityInputs()->propRespondCD4[cd4Strata];

				if (patient->getGeneralState()->gender == SimContext::GENDER_FEMALE)
					responseLogit += simContext->getHeterogeneityInputs()->propRespondFemale;
				if (patient->getDiseaseState()->typeTrueOIHistory != SimContext::HIST_EXT_N)
					responseLogit += simContext->getHeterogeneityInputs()->propRespondHistoryOIs;
				if (patient->getARTState()->hadPrevToxicity)
					responseLogit += simContext->getHeterogeneityInputs()->propRespondPriorARTToxicity;
				for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
					if (patient->getGeneralState()->hasRiskFactor[i])
						responseLogit += simContext->getHeterogeneityInputs()->propRespondRiskFactor[i];
				}
				double responseStdDev;
				double responseIncrRegimenMean;
				SimContext::HET_ART_LOGIT_DISTRIBUTION responseDist = SimContext::HET_ART_LOGIT_DISTRIBUTION_NORMAL;
				responseIncrRegimenMean = simContext->getAdolescentARTInputs(artLineNum)->propRespondARTRegimenLogitMean[ayaAgeCat+1];
				responseStdDev = simContext->getAdolescentARTInputs(artLineNum)->propRespondARTRegimenLogitStdDev[ayaAgeCat+1];

				double responseLogitDraw = -1;
				responseLogitDraw = CepacUtil::getRandomGaussian(responseIncrRegimenMean, responseStdDev, 60100, patient);

				double preIncrResponseLogit = responseLogit;
				responseLogit += responseLogitDraw;

				setARTResponseCurrRegimenBase(responseLogit, responseLogitDraw, preIncrResponseLogit, false, 1);

				if (patient->getGeneralState()->isOnAdherenceIntervention)
					responseLogit += patient->getGeneralState()->responseLogitAdherenceInterventionIncrement;
				setCurrARTResponse(responseLogit);
				/** Check if patient is suppressed on the regimen, if so redraw slope for the new AYA art age category */
				if (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS){
					int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
					double cd4Slope = drawCD4Slope(artLineNum, monthsOnART);
					setCurrRegimenCD4Slope(cd4Slope);
				}
				break;
			}  // end of checking if this is the month of age category change for adolescent ART
		} // end of looping through the AYA ART age categories for the current regimen
	} // end if the patient is on ART

	// Check whether the CD4 envelope slopes need to be updated for a new AYA ART age category
	bool setOverallEnvSlope = false;
	bool setIndivEnvSlope = false;
	if(patient->getARTState()->overallCD4Envelope.isActive){
		/** Get the overall CD4 envelope ART regimen information */
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(patient->getARTState()->overallCD4Envelope.regimenNum);
		//Month of AYA ART age cat change for overall envelope 
		for (int ayaAgeCat = 0; ayaAgeCat < SimContext::ADOLESCENT_NUM_ART_AGES-1; ayaAgeCat++){
			if (patient->getGeneralState()->ageMonths == ayaART->ageBounds[ayaAgeCat]*12){
				setOverallEnvSlope = true;
				break;
			}	
		}	
	}
	if(patient->getARTState()->indivCD4Envelope.isActive){
		/** Get the individual envelope ART regimen information */
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(patient->getARTState()->indivCD4Envelope.regimenNum);
		//Month of AYA ART age cat change for individual regimen CD4 envelope 
		for (int ayaAgeCat = 0; ayaAgeCat < SimContext::ADOLESCENT_NUM_ART_AGES-1; ayaAgeCat++){
			if (patient->getGeneralState()->ageMonths == ayaART->ageBounds[ayaAgeCat]*12){
				setIndivEnvSlope = true;
				break;
			}	
		}	
	}
	// Note that the AYA age categories may be different for the overall and individual envelope regimens, so in this case the individual envelope may need updating at a time when the overall envelope does not need to be updated
	if(setOverallEnvSlope || setIndivEnvSlope)
		performCD4EnvelopeAgeTransition(setOverallEnvSlope, setIndivEnvSlope);

	/**Handle conversions if this is the month of transition from adolescent to adult*/
	if (simContext->getAdolescentInputs()->transitionToAdult &&
		(patient->getGeneralState()->ageMonths == simContext->getAdolescentInputs()->ageTransitionToAdult)) {	
		// Update their Adolescent status to false
		setAdolescentState(false);	
		if(patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d TRANSITION TO ADULT MODEL: age %d mths (%1.2lf yrs)\n",patient->getGeneralState()->monthNum, patient->getGeneralState()->ageMonths, patient->getGeneralState()->ageMonths / 12.0 );	
		/** - Roll for new CD4 slope if on suppressive ART */
		if((patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG) &&	patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS)) {
			int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
			int artLineNum = patient->getARTState()->currRegimenNum;
			double cd4Slope = drawCD4Slope(artLineNum, monthsOnART);
			setCurrRegimenCD4Slope(cd4Slope);
		}
		// Update CD4 envelope(s) if active - individual CD4 envelope cannot be active unless the overall CD4 envelope is active
		if(patient->getARTState()->overallCD4Envelope.isActive)
			performCD4EnvelopeAgeTransition(true, patient->getARTState()->indivCD4Envelope.isActive);
	}
} /* end performAdolescentDiseaseUpdates */

/** \brief drawCD4Slope finds the mean and SD for the monthly CD4 change on suppressive ART and returns the new slope value
 * 
 * \param artLineNum the regimen number
 * \param monthsSinceStart the number of months since the regimen (actual or envelope) started
 * 
 * \return the new CD4 slope value
 **/
double HIVInfectionUpdater::drawCD4Slope(int artLineNum, int monthsSinceStart) {
	int stage = 2;
	double cd4SlopeMean;
	double cd4SlopeStdDev;
	double cd4Slope;
	if(patient->getGeneralState()->isAdolescent){
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(artLineNum);
		int ayaARTAgeCat = getAgeCategoryAdolescentART(artLineNum);
		if (monthsSinceStart <= ayaART->stageBoundsCD4ChangeOnSuppART[0])
			stage = 0;
		else if (monthsSinceStart <= ayaART->stageBoundsCD4ChangeOnSuppART[1])
			stage = 1;

		cd4SlopeMean = ayaART->CD4ChangeOnSuppARTMean[stage][ayaARTAgeCat];
		cd4SlopeStdDev = ayaART->CD4ChangeOnSuppARTStdDev[stage][ayaARTAgeCat];
	}
	else if(patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
		SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
		const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(artLineNum);
		if (monthsSinceStart <= pedsART->stageBoundsCD4ChangeOnSuppARTLate[0])
			stage = 0;
		else if (monthsSinceStart <= pedsART->stageBoundsCD4ChangeOnSuppARTLate[1])
			stage = 1;
		cd4SlopeMean = pedsART->CD4ChangeOnSuppARTMeanLate[cd4Response][stage];
		cd4SlopeStdDev = pedsART->CD4ChangeOnSuppARTStdDevLate[cd4Response][stage];
	}
	else if(patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
		SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
		const SimContext::ARTInputs *ART = simContext->getARTInputs(artLineNum);
		if (monthsSinceStart <= ART->stageBoundsCD4ChangeOnSuppART[0])
			stage = 0;
		else if (monthsSinceStart <= ART->stageBoundsCD4ChangeOnSuppART[1])
			stage = 1;
		cd4SlopeMean = ART->CD4ChangeOnSuppARTMean[cd4Response][stage];
		cd4SlopeStdDev = ART->CD4ChangeOnSuppARTStdDev[cd4Response][stage];
		}
	cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 90112, patient);

	return cd4Slope;

} /* end drawCD4Slope */

/** \brief performCD4EnvelopeAgeTransition updates the overall and individual CD4 envelope slopes during age category transitions
 * 
 * \param setOverallEnvelopeSlope a bool indicating whether the overall envelope slope needs to be updated 
 * \param setIndivEnvelopeSlope a bool indicating whether the individual envelope slope needs to be updated
 **/
void HIVInfectionUpdater::performCD4EnvelopeAgeTransition(bool setOverallEnvelopeSlope, bool setIndivEnvelopeSlope){
	int overallEnvLineNum = patient->getARTState()->overallCD4Envelope.regimenNum;
	int indivEnvLineNum = patient->getARTState()->indivCD4Envelope.regimenNum;
	if(setOverallEnvelopeSlope){
		/** If patient is still suppressed on the regimen that is setting the overall envelope,
		//	use the new current CD4 slope as the new overall CD4 envelope slope */
		if (patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) && (patient->getARTState()->currRegimenNum == patient->getARTState()->overallCD4Envelope.regimenNum) &&
		(patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->overallCD4Envelope.monthOfStart)) {
			setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, patient->getARTState()->currRegimenCD4Slope);
	}
		// Otherwise, they need a new slope
		else{
			int monthsSinceOverallEnv = patient->getGeneralState()->monthNum - patient->getARTState()->overallCD4Envelope.monthOfStart;

			double cd4Slope = drawCD4Slope(overallEnvLineNum, monthsSinceOverallEnv);
			setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
		}	
		// If the patient has only ever been successful on one ART regimen, the envelopes are identical, so we set the individual envelope slope to the newly updated overall one
		if (overallEnvLineNum == indivEnvLineNum){
			setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->overallCD4Envelope.slope);
			// You've now updated both slopes, so return
			return;
		}	
	}
	// Updates for individual envelopes which are not identical to the overall envelope
	if (setIndivEnvelopeSlope){
		// If they are still suppressed on the regimen that is setting the individual envelope, use the current CD4 slope as the new individual CD4 envelope slope
		if (patient->getARTState()->isOnART && (patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) && (patient->getARTState()->currRegimenNum == patient->getARTState()->indivCD4Envelope.regimenNum) && (patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->indivCD4Envelope.monthOfStart)){
			setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
		}
		// Otherwise, they need new a new slope
		else{
			int monthsSinceIndivEnv = patient->getGeneralState()->monthNum - patient->getARTState()->indivCD4Envelope.monthOfStart;
			double cd4Slope = drawCD4Slope(indivEnvLineNum, monthsSinceIndivEnv);
			setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
		}	
	}
	// Debugging trace - HIVInf stands for the file name, HIVInfectionUpdater
	if (overallEnvLineNum < 0 || overallEnvLineNum > SimContext::ART_NUM_LINES){
		tracer->printTrace(1, "Patient Num:%d, Month:%d, ART:%d;\n, HIVInf", patient->getGeneralState()->patientNum, patient->getGeneralState()->monthNum, overallEnvLineNum);
	}
} /* end performCD4EnvelopeAgeTransition */

/** \brief evaluatePedsHIVProphPolicy determines if the starting criteria for Infant HIV Proph have been met
 *
 * \param infantHIVProph is reference to a SimContext::EIDInputs::InfantHIVProph to be evaluated
 * \return true if the Patient meets criteria
 **/
bool HIVInfectionUpdater::evaluatePedsHIVProphPolicy(const SimContext::EIDInputs::InfantHIVProph &infantHIVProph) {
	/** return false if over max age */
	if (patient->getGeneralState()->ageMonths > infantHIVProph.infantProphMaxAge)
		return false;

	/** maternal HIV status */
	if (infantHIVProph.infantProphEligibilityMaternalStatusKnown != SimContext::NOT_APPL &&
			patient->getPedsState()->maternalStatusKnown != (bool) infantHIVProph.infantProphEligibilityMaternalStatusKnown)
		return false;

	/** maternal status pos */
	if (infantHIVProph.infantProphEligibilityMaternalStatusPos != SimContext::NOT_APPL &&
			(patient->getPedsState()->maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG)!= (bool) infantHIVProph.infantProphEligibilityMaternalStatusPos)
		return false;

	/** mother on ART */
	if (infantHIVProph.infantProphEligibilityMotherOnART != SimContext::NOT_APPL &&
			patient->getPedsState()->motherOnART != (bool) infantHIVProph.infantProphEligibilityMotherOnART)
		return false;

	/** mother viral status known at delivery */
	if (infantHIVProph.infantProphEligibilityMaternalVSKnownDelivery != SimContext::NOT_APPL &&
			patient->getPedsState()->motherSuppressionKnownInitially != (bool) infantHIVProph.infantProphEligibilityMaternalVSKnownDelivery)
		return false;

	/** mother viral status at delivery */
	if (infantHIVProph.infantProphEligibilityMotherSuppressedDelivery != SimContext::NOT_APPL &&
			patient->getPedsState()->motherOnSuppressedARTInitially != (bool) infantHIVProph.infantProphEligibilityMotherSuppressedDelivery)
		return false;

	/** mother HIV+ with HVL known to be high at delivery - the HVL can only be known if the mother was on ART at delivery. Maternal HVL is not observed when off ART. */
	if(patient->getPedsState()->motherOnARTInitially && !patient->getPedsState()->motherOnSuppressedARTInitially && patient->getPedsState()->motherSuppressionKnownInitially){
		if (infantHIVProph.infantProphEligibilityMotherHVLHighDelivery != SimContext::NOT_APPL &&
				patient->getPedsState()->motherLowHVLInitially == (bool) infantHIVProph.infantProphEligibilityMotherHVLHighDelivery)
			return false;
	}		

	/** mother viral status known */
	if (infantHIVProph.infantProphEligibilityMaternalVSKnown != SimContext::NOT_APPL &&
			patient->getPedsState()->motherSuppressionKnown != (bool) infantHIVProph.infantProphEligibilityMaternalVSKnown)
		return false;

	/** mother viral status */
	if (infantHIVProph.infantProphEligibilityMotherSuppressed != SimContext::NOT_APPL &&
			patient->getPedsState()->motherOnSuppressedART != (bool) infantHIVProph.infantProphEligibilityMotherSuppressed)
		return false;

	/** mother HIV+ with HVL known to be high - only if mother is on ART. Otherwise maternal HVL will not be observed. */
	if(patient->getPedsState()->motherOnART && !patient->getPedsState()->motherOnSuppressedART && patient->getPedsState()->motherSuppressionKnown){
		if (infantHIVProph.infantProphEligibilityMotherHVLHigh != SimContext::NOT_APPL &&
				patient->getPedsState()->motherLowHVL == (bool) infantHIVProph.infantProphEligibilityMotherHVLHigh)
			return false;
	}		

	return true;
} /* end evaluatePedsHIVProphPolicy */