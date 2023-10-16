#include "include.h"

/** \brief Constructor takes in the patient object */
BeginMonthUpdater::BeginMonthUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
BeginMonthUpdater::~BeginMonthUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void BeginMonthUpdater::performInitialUpdates() {

	/** First call the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	/** Initialize the general patient state */
	int patientNum = runStats->getPopulationSummary()->numCohorts + 1;
	/** Increase numCohorts here so that the next created patient has a different patientNum, even if this patient isn't dead (i.e. in the transmission model) */
	this->incrementCohortSize();
	bool tracingEnabled = (patientNum <= SimContext::numPatientsToTrace);
	initializePatient(patientNum, tracingEnabled);

	//Reset seed for patient if using fixed seed
	if (!simContext->getRunSpecsInputs()->randomSeedByTime)
		CepacUtil::setFixedSeed(patient);

	/** Determine the patients gender */
	double randNum = CepacUtil::getRandomDouble(20010, patient);
	SimContext::GENDER_TYPE gender = SimContext::GENDER_FEMALE;
	if (randNum < simContext->getCohortInputs()->maleGenderDistribution)
		gender = SimContext::GENDER_MALE;

	/** Determine the patients age */
	int ageMonths;
    // Custom age input overrides pediatrics age input!
    if (simContext->getCohortInputs()->useCustomAgeDist) {
        const int numStrata = SimContext::INIT_AGE_NUM_STRATA;
        const double *ageProbs = simContext->getCohortInputs()->ageProbs;
        const double *ageStrata = simContext->getCohortInputs()->ageStrata;

        double randNum = CepacUtil::getRandomDouble(20015, patient);
		// Determine the index for the patient's age stratum by setting upper and lower bounds around the random number based on the age strata probabilities and narrowing the bounds on either side  until the stratum is found for which the random number is closest to the associated probability while still being smaller than it
		// The age strata proability inputs are read into a cumulative distribution function, so the first stratum always has the lowest probability and the last defined stratum always has a probability of 1. We start with stratum 0 (lowest probability) on the left hand side of the inequality with the random number and the number of strata (1 past the highest possible index to be the first with a probability of 1) on the right side of the inequality with the random number, then loop through and take averages to adjust the bounds until the index for the appropriate stratum is reached
        int left = 0;
        int right = numStrata;
        int strat = 0;
		int numAttempts = 0;
        while (true){
            if (randNum < ageProbs[strat]){
                if (strat == 0 || ageProbs[strat - 1] <= randNum)
                    break;
                right = strat;
            }
            else{
                left = strat;
				numAttempts++;
				// It should not take more than 5 attempts to find the right stratum out of 32
				if(numAttempts > 5){
					printf("\nWARNING: unable to find age stratum for Patient %d. Need a cumulative probability greater than %lf. Defaulting to stratum %d", patient->getGeneralState()->patientNum, randNum, strat+1);
					break;
				}
			}	
            strat = (left+right)/2;
        }
		// Once the index is found, we use the bounds of this stratum and another random number to determine the patient's initial age
        ageMonths = (int) ((CepacUtil::getRandomDouble(20016, patient) * (ageStrata[strat + numStrata]-ageStrata[strat])) + ageStrata[strat] + .5);
    }

    // If no custom age distribution, get params and draw from normal dist.
    else {
        double ageMonthsMean;
        double ageMonthsStdDev;
        // Use Peds Tab age input if Peds model is active
        if (simContext->getPedsInputs()->enablePediatricsModel){
			ageMonthsMean = simContext->getPedsInputs()->initialAgeMean;
			ageMonthsStdDev = simContext->getPedsInputs()->initialAgeStdDev;
		}
		else {
			ageMonthsMean = simContext->getCohortInputs()->initialAgeMean;
			ageMonthsStdDev = simContext->getCohortInputs()->initialAgeStdDev;
		}
		ageMonths = -1;
		while ((ageMonths < 0) || (ageMonths > 1200)){
        	ageMonths = (int) (CepacUtil::getRandomGaussian(ageMonthsMean, ageMonthsStdDev, 20020, patient) + 0.5);
    	}
	}

	/** If we want the pre-assigned age and gender (i.e. transmission model), set that here */
	if (this->patient->getGeneralState()->predefinedAgeAndGender){
		gender = this->patient->getGeneralState()->gender;
		ageMonths = this->patient->getGeneralState()->ageMonths;
	}

	/** Set the age and gender of the patient */
	setPatientAgeGender(gender, ageMonths);
	// Determine whether the patient is an Adolescent or Pediatric one rather than an Adult
	bool isAdolescent = false;
	bool isPediatric = false;
	
	if(simContext->getAdolescentInputs()->enableAdolescent){
		if(simContext->getPedsInputs()->enablePediatricsModel && 
		 (patient->getGeneralState()->ageMonths < simContext->getAdolescentInputs()->ageTransitionFromPeds))
			isPediatric = true;
		else if(simContext->getAdolescentInputs()->transitionToAdult){
			if(patient->getGeneralState()->ageMonths  < simContext->getAdolescentInputs()->ageTransitionToAdult)
				isAdolescent = true;
		} 	
		else
			isAdolescent = true;	
	}
	else if(patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_ADULT)
		isPediatric = true;
	setAdolescentState(isAdolescent);
	setPediatricState(isPediatric);

	/** Set the patients prevalence of risk factors */
	for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
		bool hasRisk = false;
		randNum = CepacUtil::getRandomDouble(60050, patient);
		if (randNum < simContext->getCohortInputs()->probRiskFactorPrev[i])
			hasRisk = true;
		setRiskFactor(i, hasRisk, true);
	}
	/** initialize patient cost subgroups to false */
	setCostSubgroups(true);
} /* end performInitialUpdates */
//

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void BeginMonthUpdater::performMonthlyUpdates() {
	/** Set the discount factor, reset the QOL value to the background value, and reset the occurence of an acute OI and the mortality risks*/
	resetQOL();
	setCurrTrueOI(SimContext::OI_NONE);
	setCurrObservedOI(false);
	clearMortalityRisks();

	/** Do special processing for the initial month: */
	if (patient->getGeneralState()->monthNum == patient->getGeneralState()->initialMonthNum) {
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "\n\nBEGIN PATIENT %d\n", patient->getGeneralState()->patientNum);
		}

		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG) {
			/** Print out initial patient tracing for HIV negative patients */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "  gender: %s, init age: %d mths (%1.2lf yrs)\n",
					(patient->getGeneralState()->gender == SimContext::GENDER_MALE) ? "male" : "female",
					patient->getGeneralState()->ageMonths, patient->getGeneralState()->ageMonths / 12.0 );
				tracer->printTrace(1, "  CD4 response: %s, init VisitType %s, Implement proph %s art %s;\n",
					SimContext::CD4_RESPONSE_STRS[patient->getARTState()->CD4ResponseType],
					SimContext::CLINIC_VISITS_STRS[patient->getMonitoringState()->clinicVisitType],
					(patient->getProphState()->mayReceiveProph) ? "YES" : "NO",
					(patient->getARTState()->mayReceiveART) ? "YES" : "NO" );
				if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_ADULT) {
					tracer->printTrace(1, "  Peds HIV state: %s\n",
						SimContext::PEDS_HIV_STATE_STRS[patient->getDiseaseState()->infectedPediatricsHIVState]);
					if (simContext->getEIDInputs()->enableHIVTestingEID)
						tracer->printTrace(1, "  Age of Seroreversion: %d\n", patient->getPedsState()->ageOfSeroreversion);

					tracer->printTrace(1, "  Maternal State:%s,%s,%s,%s,%s\n", SimContext::PEDS_MATERNAL_STATUS_STRS[patient->getPedsState()->maternalStatus],
							SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus],
							patient->getPedsState()->maternalStatusKnown? "Status Known" : "Status Unknown",
							patient->getPedsState()->motherOnART? "On ART" : "Not On ART",
							patient->getPedsState()->motherOnSuppressedART? "Suppressed" : "Not Suppressed");

				}
				tracer->printTrace(1, "  HIV state: HIVneg, HIV risk level: %s\n", patient->getMonitoringState()->isHighRiskForHIV?"High":"Low");

				tracer->printTrace(1, "  On PrEP: %s\n", patient->getMonitoringState()->hasPrEP?"Yes":"No");	
			} //end if tracing is enabled
		} //end if patient is HIV-negative
		else {
			/** If patient is initially HIV-positive, update the initial distribution at time of infection statistics */
			updateInitialDistributions();

			/** Print out initial patient tracing for prevalent HIV positive patients */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "  gender: %s, init age: %d mths (%1.2lf yrs)\n",
					(patient->getGeneralState()->gender == SimContext::GENDER_MALE) ? "male" : "female",
					patient->getGeneralState()->ageMonths, patient->getGeneralState()->ageMonths / 12.0 );
				if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE) {
					tracer->printTrace(1, "  init CD4 perc: %1.3f %s;\n", patient->getDiseaseState()->currTrueCD4Percentage,
						SimContext::CD4_STRATA_STRS[patient->getDiseaseState()->currTrueCD4Strata]);
				}
				else {
					tracer->printTrace(1, "  init CD4: %1.0f %s;\n", patient->getDiseaseState()->currTrueCD4,
						SimContext::CD4_STRATA_STRS[patient->getDiseaseState()->currTrueCD4Strata]);
				}
				tracer->printTrace(1, "  init HVL: %s, setpt: %s;\n",
					SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->currTrueHVLStrata],
					SimContext::HVL_STRATA_STRS[patient->getDiseaseState()->setpointHVLStrata]);
				tracer->printTrace(1, "  CD4 response: %s, init VisitType %s, Implement proph %s art %s;\n",
					SimContext::CD4_RESPONSE_STRS[patient->getARTState()->CD4ResponseType],
					SimContext::CLINIC_VISITS_STRS[patient->getMonitoringState()->clinicVisitType],
					(patient->getProphState()->mayReceiveProph) ? "YES" : "NO",
					(patient->getARTState()->mayReceiveART) ? "YES" : "NO" );
				if (patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_ADULT) {
					tracer->printTrace(1, "  Peds HIV state: %s\n",
						SimContext::PEDS_HIV_STATE_STRS[patient->getDiseaseState()->infectedPediatricsHIVState]);
					if (simContext->getEIDInputs()->enableHIVTestingEID)
						tracer->printTrace(1, "  Age of Seroreversion: %d\n", patient->getPedsState()->ageOfSeroreversion);
					tracer->printTrace(1, "  Maternal State:%s,%s,%s,%s,%s\n", SimContext::PEDS_MATERNAL_STATUS_STRS[patient->getPedsState()->maternalStatus],
							SimContext::PEDS_BF_TYPE_STRS[patient->getPedsState()->breastfeedingStatus],
							patient->getPedsState()->maternalStatusKnown? "Status Known" : "Status Unknown",
							patient->getPedsState()->motherOnART? "On ART" : "Not On ART",
							patient->getPedsState()->motherOnSuppressedART? "Suppressed" : "Not Suppressed");
				}
				tracer->printTrace(1, "  HIV state: %s, %s\n",
					SimContext::HIV_POS_STRS[patient->getDiseaseState()->infectedHIVPosState],
					(patient->getMonitoringState()->isDetectedHIVPositive) ? "detected" : "undetected");
				for (int i = 0; i < SimContext::OI_NUM; i++) {
					if (patient->getDiseaseState()->hasTrueOIHistory[i]) {
						tracer->printTrace(1, "  init OI history: %s\n", SimContext::OI_STRS[i]);
					}
				}
			} //end if tracing is enabled
		} //end if patient is HIV-positive	
		// more initial states independent of HIV status
		if(patient->getGeneralState()->tracingEnabled){
			for (int i = 0; i < SimContext::CHRM_NUM; i++) {
				if (patient->getDiseaseState()->hasTrueCHRMs[i]) {
					int stage = SimContext::NOT_APPL;
					for (int j = SimContext::CHRM_TIME_PER_NUM - 1; j >= 0; j--) {
						if (patient->getGeneralState()->monthNum >= patient->getDiseaseState()->monthOfCHRMsStageStart[i][j]) {
							stage = j+1;
							break;
						}
					}
					tracer->printTrace(1, "  init prevalent CHRMs: %s (%d m old, stage %d)\n", SimContext::CHRM_STRS[i], patient->getGeneralState()->monthNum - patient->getDiseaseState()->monthOfCHRMsStageStart[i][0], stage);
				}
			}
			for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
				if (patient->getGeneralState()->hasRiskFactor[i]) {
					tracer->printTrace(1, "  init prevalent Risk Factors: %s\n", SimContext::RISK_FACT_STRS[i]);
				}
			}

			if (simContext->getTBInputs()->enableTB){
				tracer->printTrace(1, "  TB Init State: %s;\n",
					SimContext::TB_STATE_STRS[patient->getTBState()->currTrueTBDiseaseState]);
				
				if (patient->getTBState()->currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED){
					tracer->printTrace(1, "  TB Init Strain: %s;\n",
						SimContext::TB_STRAIN_STRS[patient->getTBState()->currTrueTBResistanceStrain]);

					tracer->printTrace(1, "  TB Init Trackers: Sputum:%s, Immune Reactive:%s, TB Symptoms:%s;\n",
						patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SPUTUM_HI]?"Yes":"No",
						patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE]?"Yes":"No",
						patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]?"Yes":"No");
					
					if(patient->getTBState()->observedHistActiveTBAtEntry){
						tracer->printTrace(1, "  TB Treatment %d stopped %d m ago;\n", 
							patient->getTBState()->mostRecentTreatNum, (-1 * patient->getTBState()->monthOfInitialTreatmentStop));
					}	
				}
			}
		} //end if tracing is enabled
	} //end if this is the initial month 

	/** set patient costs subgroups */
	setCostSubgroups();
} /* end performMonthlyUpdates */
