#include "include.h"

/** \brief Constructor takes in the patient object and determines if updatesCanOccur */
MortalityUpdater::MortalityUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
MortalityUpdater::~MortalityUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void MortalityUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void MortalityUpdater::performMonthlyUpdates() {
	const SimContext::NatHistInputs *natHist = simContext->getNatHistInputs();
	SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
	SimContext::PEDS_AGE_CAT pedsAgeCat = patient->getPedsState()->ageCategoryPediatrics;
	int ageYears = patient->getGeneralState()->ageMonths / 12;
	int monthNum = patient->getGeneralState()->monthNum;
	double backgroundDeathRate;
	double deathRateRatioHIV = 1.0;
	double OIHistDeathRateRatio = 1.0;
	double maxOIHistDeathRateRatio = 1.0;
	
	// if the patient's age is greater than the maximum allowed, death occurs and the cause is set as background mortality 
	if (ageYears > SimContext::AGE_MAXIMUM){
		/* set unfavorable outcome for TB if death occurs during initial treatment*/
		if (simContext->getTBInputs()->enableTB){
			if ((patient->getTBState()->isOnTreatment || patient->getTBState()->isOnEmpiricTreatment) && !patient->getTBState()->everHadNonInitialTreatmentOrEmpiric)
				setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_DEATH);
		}
		setCauseOfDeath(SimContext::DTH_BKGD_MORT);
		return;
	}

	// Add mortality risks for adolescents
	if (patient->getGeneralState()->isAdolescent){
		backgroundDeathRate = natHist->monthlyBackgroundDeathRate[gender][ageYears];
		// Adjust background mortality by overall modifier if applicable
		if (simContext->getNatHistInputs()->backgroundMortModifierType== SimContext::MORT_MOD_INCREMENTAL){
			//Incremental method increments probability of death in proportion to the probability of not dying to ensure it stays less than or equal to 1
			double extraBackgroundMortRisk = simContext->getNatHistInputs()->backgroundMortModifier;
			//Convert rate to prob first
			double probDeathBackgroundMort = CepacUtil::rateToProb(backgroundDeathRate);
			probDeathBackgroundMort =probDeathBackgroundMort + extraBackgroundMortRisk - (probDeathBackgroundMort*extraBackgroundMortRisk);
			probDeathBackgroundMort = max(probDeathBackgroundMort, 0.0);
			// Do data validation check here since it can't be done in the input sheet
			if(probDeathBackgroundMort < 1.0){
				backgroundDeathRate = CepacUtil::probToRate(probDeathBackgroundMort);
			}
		}
		else if (simContext->getNatHistInputs()->backgroundMortModifierType == SimContext::MORT_MOD_MULT){
			backgroundDeathRate *= simContext->getNatHistInputs()->backgroundMortModifier;
		}	
		// Add background mortality, using adult inputs from the NatHist tab
		addMortalityRisk(SimContext::DTH_BKGD_MORT, backgroundDeathRate);
		// Add risk of death from the generic risk factors if applicable
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++){
			if (patient->getGeneralState()->hasRiskFactor[i] && simContext->getAdolescentInputs()->genericRiskDeathRateRatio[i][getAgeCategoryAdolescent()] > 1.0) {
				addMortalityRisk((SimContext::DTH_CAUSES)(SimContext::DTH_RISK_1 + i),simContext->getAdolescentInputs()->genericRiskDeathRateRatio[i][getAgeCategoryAdolescent()]);
			}
		}
		// If the adolescent is HIV positive, add the risk of death from HIV
		if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
			SimContext::CD4_STRATA currCD4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			deathRateRatioHIV = simContext->getAdolescentInputs()->HIVDeathRateRatio[currCD4Strata][getAgeCategoryAdolescent()];
			//  Modify by ART death rate ratio if applicable, adjusting for adherence first
			if (patient->getARTState()->applyARTEffect){
				double ARTDeathRateRatio = simContext->getAdolescentInputs()->ARTDeathRateRatio[getAgeCategoryAdolescent()];
				ARTDeathRateRatio = 1 - (patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_ARTEFFECT_MORT] * (1 - ARTDeathRateRatio));
				deathRateRatioHIV *= ARTDeathRateRatio;
			}
			// Check for severe OI history in the HIV positive adolescents and modify the HIV death rate ratio by the maximum of any applicable severe OI history death rate ratios
			for(int oiType = 0; oiType < SimContext::OI_NUM; oiType++){
				// Check for a history of the OI and whether the OI is designated as severe
				if(patient->getDiseaseState()->hasTrueOIHistory[oiType] &&  simContext->getRunSpecsInputs()->severeOIs[oiType]){
					int monthsSince = monthNum - patient->getDiseaseState()->lastMonthSevereOI[oiType];
					// Check if this is TB being modeled as a severe OI when the TB tab is not enabled 
					if(!simContext->getTBInputs()->enableTB && simContext->getRunSpecsInputs()->OIsIncludeTB && oiType==0){
						if(monthNum > 0 && monthsSince <=  simContext->getAdolescentInputs()->TB_OIHistEffectDuration){
							OIHistDeathRateRatio = simContext->getAdolescentInputs()->TB_OIHistDeathRateRatio[getAgeCategoryAdolescent()];
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);
						}
					}	
					// Otherwise, it is a regular severe OI
					else{	
						if(monthNum > 0 && monthsSince <=  simContext->getAdolescentInputs()->severeOIHistEffectDuration){				
							OIHistDeathRateRatio = simContext->getAdolescentInputs()->severeOIHistDeathRateRatio[getAgeCategoryAdolescent()];
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);		
						}
					}
				}
			}
			deathRateRatioHIV *= maxOIHistDeathRateRatio;
			if (deathRateRatioHIV > 1.0) {
				addMortalityRisk(SimContext::DTH_HIV, deathRateRatioHIV);
			}	
		}	
	}	
	// Add mortality risks for adults
	else if(pedsAgeCat == SimContext::PEDS_AGE_ADULT){
		backgroundDeathRate = natHist->monthlyBackgroundDeathRate[gender][ageYears];
		// Adjust background mortality by overall modifier if applicable
		if (simContext->getNatHistInputs()->backgroundMortModifierType== SimContext::MORT_MOD_INCREMENTAL){
			//Incremental method increments probability of death in proportion to the probability of not dying to ensure it stays less than or equal to 1
			double extraBackgroundMortRisk = simContext->getNatHistInputs()->backgroundMortModifier;
			//Convert rate to prob first
			double probDeathBackgroundMort = CepacUtil::rateToProb(backgroundDeathRate);
			probDeathBackgroundMort =probDeathBackgroundMort + extraBackgroundMortRisk - (probDeathBackgroundMort*extraBackgroundMortRisk);
			probDeathBackgroundMort = max(probDeathBackgroundMort, 0.0);
			// Do data validation check here since it can't be done in the input sheet
			if(probDeathBackgroundMort < 1.0){
				backgroundDeathRate = CepacUtil::probToRate(probDeathBackgroundMort);
			}
		}
		else if (simContext->getNatHistInputs()->backgroundMortModifierType == SimContext::MORT_MOD_MULT){
			backgroundDeathRate *= simContext->getNatHistInputs()->backgroundMortModifier;
		}	
		// Add background mortality 
		addMortalityRisk(SimContext::DTH_BKGD_MORT, backgroundDeathRate);
		// Add risk of death from the generic risk factors if applicable
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++){
			if (patient->getGeneralState()->hasRiskFactor[i] && natHist->genericRiskDeathRateRatio[i] > 1.0) {
				addMortalityRisk((SimContext::DTH_CAUSES)(SimContext::DTH_RISK_1 + i), natHist->genericRiskDeathRateRatio[i]);
			}
		}
		// If the patient is HIV positive, add the risk of death from HIV 
		if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
			SimContext::CD4_STRATA currCD4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			deathRateRatioHIV = natHist->HIVDeathRateRatio[currCD4Strata];
			//  Modify by ART death rate ratio if applicable
			if (patient->getARTState()->applyARTEffect){
				double ARTDeathRateRatio = 1 - (patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_ARTEFFECT_MORT] * (1 - natHist->ARTDeathRateRatio));
				deathRateRatioHIV *= ARTDeathRateRatio;
			}
			// Check for severe OI history in the HIV positive patients and modify the HIV death rate ratio by the maximum of any applicable severe OI history death rate ratios
			for(int oiType=0; oiType <SimContext::OI_NUM; oiType++){
				// Check for a history of the OI and whether the OI is designated as severe
				if(patient->getDiseaseState()->hasTrueOIHistory[oiType] && simContext->getRunSpecsInputs()->severeOIs[oiType]){
					int monthsSince = monthNum - patient->getDiseaseState()->lastMonthSevereOI[oiType];
					// Check if this is TB being modeled as a severe OI when the TB tab is not enabled 
					if(!simContext->getTBInputs()->enableTB && simContext->getRunSpecsInputs()->OIsIncludeTB && oiType==0){
						if(monthNum > 0 && monthsSince <=natHist->TB_OIHistEffectDuration){
							OIHistDeathRateRatio = natHist->TB_OIHistDeathRateRatio;
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);
						}
					}
					// Otherwise, it is a regular severe OI
					else{
						if(monthNum > 0 && monthsSince <=natHist->severeOIHistEffectDuration){					
							OIHistDeathRateRatio = natHist->severeOIHistDeathRateRatio;
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);
						}
					}	
				}	
			}
			deathRateRatioHIV *= maxOIHistDeathRateRatio;	
			if(deathRateRatioHIV > 1.0) {
				addMortalityRisk(SimContext::DTH_HIV, deathRateRatioHIV);	
			}	
		}
	}		
	// Add mortality risks for late childhood
	else if (pedsAgeCat == SimContext::PEDS_AGE_LATE){
		// Add background mortality; start with the adult background death rates from the NatHist tab
		backgroundDeathRate = natHist->monthlyBackgroundDeathRate[gender][ageYears];
		// Modify by maternal mortality death rate ratio for late childhood if applicable
		if (!patient->getPedsState()->isMotherAlive && monthNum < patient->getPedsState()->monthOfMaternalDeath + simContext->getPedsInputs()->durationMaternalMortDeathRateRatioLate){
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
				backgroundDeathRate *= simContext->getPedsInputs()->maternalMortDeathRateRatioLateHIVNeg;	
			}	
			else{
				backgroundDeathRate *= simContext->getPedsInputs()->maternalMortDeathRateRatioLateHIVPos;
			}
		}	
		
		// Adjust background mortality by overall modifier if applicable
		if (simContext->getNatHistInputs()->backgroundMortModifierType== SimContext::MORT_MOD_INCREMENTAL){
			//Incremental method increments probability of death in proportion to the probability of not dying to ensure it stays less than or equal to 1
			double extraBackgroundMortRisk = simContext->getNatHistInputs()->backgroundMortModifier;
			//Convert rate to prob first
			double probDeathBackgroundMort = CepacUtil::rateToProb(backgroundDeathRate);
			probDeathBackgroundMort =probDeathBackgroundMort + extraBackgroundMortRisk - (probDeathBackgroundMort*extraBackgroundMortRisk);
			probDeathBackgroundMort = max(probDeathBackgroundMort, 0.0);
			// Do data validation check here since it can't be done in the input sheet
			if(probDeathBackgroundMort < 1.0){
				backgroundDeathRate = CepacUtil::probToRate(probDeathBackgroundMort);
			}
		}
		else if (simContext->getNatHistInputs()->backgroundMortModifierType == SimContext::MORT_MOD_MULT){
			backgroundDeathRate *= simContext->getNatHistInputs()->backgroundMortModifier;
		}	
		
		addMortalityRisk(SimContext::DTH_BKGD_MORT, backgroundDeathRate);
		// Add risk of death from the generic risk factors if applicable
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++){
			if (patient->getGeneralState()->hasRiskFactor[i] &&simContext->getPedsInputs()->genericRiskDeathRateRatio[i][pedsAgeCat] > 1.0) {
				addMortalityRisk((SimContext::DTH_CAUSES)(SimContext::DTH_RISK_1 + i),simContext->getPedsInputs()->genericRiskDeathRateRatio[i][pedsAgeCat]);
			}
		}	
		// If the child is HIV positive, add the risk of death from HIV
		if(patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
			SimContext::CD4_STRATA currCD4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			deathRateRatioHIV = simContext->getPedsInputs()->HIVDeathRateRatioPedsLate[currCD4Strata];
			//  Modify by ART death rate ratio if applicable, using stage bounds based on how long they have been on the current regimen
			if (patient->getARTState()->applyARTEffect) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				int stage = 2;
				for (int i = 0; i < 2; i++) {
					if (monthsOnART <= simContext->getPedsInputs()->stageBoundsARTDeathRateRatioPeds[i]) {
					stage = i;
					break;
					}
				}
				double ARTDeathRateRatio = simContext->getPedsInputs()->ARTDeathRateRatio[pedsAgeCat][stage];
				ARTDeathRateRatio = 1 - (patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_ARTEFFECT_MORT] * (1 - ARTDeathRateRatio));
				deathRateRatioHIV *= ARTDeathRateRatio;
			}
			// Check for severe OI history in the HIV positive children and modify the HIV death rate ratio by the maximum of any applicable severe OI history death rate ratios
			for(int oiType=0; oiType <SimContext::OI_NUM; oiType++){
				// Check for a history of the OI and whether the OI is designated as severe
				if(patient->getDiseaseState()->hasTrueOIHistory[oiType] && simContext->getRunSpecsInputs()->severeOIs[oiType]){
					int monthsSince = monthNum - patient->getDiseaseState()->lastMonthSevereOI[oiType];
					// Check if this is TB being modeled as a severe OI when the TB tab is not enabled 
					if(!simContext->getTBInputs()->enableTB && simContext->getRunSpecsInputs()->OIsIncludeTB && oiType==0){
						if(monthNum > 0 && monthsSince <= simContext->getPedsInputs()->TB_OIHistEffectDurationPedsLate){
							OIHistDeathRateRatio = simContext->getPedsInputs()->TB_OIHistDeathRateRatioPedsLate;
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);
						}
					}
					// Otherwise, it is a regular severe OI 
					else{
						if(monthNum > 0 && monthsSince <= simContext->getPedsInputs()->severeOIHistEffectDurationPedsLate){
							OIHistDeathRateRatio = simContext->getPedsInputs()->severeOIHistDeathRateRatioPedsLate;
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);
						}
					}	
				}
			}
			deathRateRatioHIV *= maxOIHistDeathRateRatio;	
			if (deathRateRatioHIV > 1.0) {
				addMortalityRisk(SimContext::DTH_HIV, deathRateRatioHIV);	
			}		
		}		
	}	
	// Add mortality risks for early childhood
	else {		
		// Add background mortality; start with the pediatric background death rates
		if (simContext->getPedsInputs()->useExposedUninfectedDefs && patient->getDiseaseState()->useHEUMortality){	
			backgroundDeathRate = simContext->getPedsInputs()->backgroundDeathRateExposedUninfectedEarly[gender][pedsAgeCat];
		}
		else{
			backgroundDeathRate = simContext->getPedsInputs()->backgroundDeathRateEarly[gender][pedsAgeCat];
		}	
		// Modify by maternal mortality death rate ratios for early childhood if applicable 
		if (!patient->getPedsState()->isMotherAlive && monthNum < patient->getPedsState()->monthOfMaternalDeath + simContext->getPedsInputs()->durationMaternalMortDeathRateRatioEarly){
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
				backgroundDeathRate *= simContext->getPedsInputs()->maternalMortDeathRateRatioEarlyHIVNeg;
			}	
			else {
				backgroundDeathRate *= simContext->getPedsInputs()->maternalMortDeathRateRatioEarlyHIVPos;
			}	
		}
		// Modify by replacement feeding death rate ratios if applicable - only if the mother is alive, to avoid the potential for applying both this and the maternal mortality death rate ratio in the same month
		if (patient->getPedsState()->isMotherAlive && patient->getPedsState()->breastfeedingStoppedEarly && (monthNum < patient->getPedsState()->monthOfReplacementFeedingStart + simContext->getPedsInputs()->durationReplacementFedDeathRateRatioEarly)){
			if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
				backgroundDeathRate *= simContext->getPedsInputs()->replacementFedDeathRateRatioEarlyHIVNeg;
			}	
			else{
				backgroundDeathRate *= simContext->getPedsInputs()->replacementFedDeathRateRatioEarlyHIVPos;	
			}	
		}
		// Adjust background mortality by overall modifier if applicable
		if (simContext->getNatHistInputs()->backgroundMortModifierType== SimContext::MORT_MOD_INCREMENTAL){
			//Incremental method increments probability of death in proportion to the probability of not dying to ensure it stays less than or equal to 1
			double extraBackgroundMortRisk = simContext->getNatHistInputs()->backgroundMortModifier;
			//Convert rate to prob first
			double probDeathBackgroundMort = CepacUtil::rateToProb(backgroundDeathRate);
			probDeathBackgroundMort =probDeathBackgroundMort + extraBackgroundMortRisk - (probDeathBackgroundMort*extraBackgroundMortRisk);
			probDeathBackgroundMort = max(probDeathBackgroundMort, 0.0);
			// Do data validation check here since it can't be done in the input sheet
			if(probDeathBackgroundMort < 1.0){
				backgroundDeathRate = CepacUtil::probToRate(probDeathBackgroundMort);
			}
		}
		else if (simContext->getNatHistInputs()->backgroundMortModifierType == SimContext::MORT_MOD_MULT){
			backgroundDeathRate *= simContext->getNatHistInputs()->backgroundMortModifier;
		}	
		addMortalityRisk(SimContext::DTH_BKGD_MORT, backgroundDeathRate);
		// Add risk of death from the generic risk factors if applicable
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++){
			if (patient->getGeneralState()->hasRiskFactor[i] && simContext->getPedsInputs()->genericRiskDeathRateRatio[i][pedsAgeCat] > 1.0) {
				addMortalityRisk((SimContext::DTH_CAUSES)(SimContext::DTH_RISK_1 + i),simContext->getPedsInputs()->genericRiskDeathRateRatio[i][pedsAgeCat]);
			}
		}
		// If the child is HIV positive, add the risk of death from HIV
		if(patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
			SimContext::PEDS_CD4_PERC cd4PercStrata = patient->getDiseaseState()->currTrueCD4PercentageStrata;
			deathRateRatioHIV = simContext->getPedsInputs()->HIVDeathRateRatioPedsEarly[pedsAgeCat][cd4PercStrata];
			//  Modify by ART death rate ratio if applicable, using stage bounds based on how long they have been on the current regimen
			if (patient->getARTState()->applyARTEffect) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				int stage = 2;
				for (int i = 0; i < 2; i++) {
					if (monthsOnART <= simContext->getPedsInputs()->stageBoundsARTDeathRateRatioPeds[i]) {
						stage = i;
						break;
					}
				}
				double ARTDeathRateRatio = simContext->getPedsInputs()->ARTDeathRateRatio[pedsAgeCat][stage];
				ARTDeathRateRatio = 1 - (patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_ARTEFFECT_MORT] * (1 - ARTDeathRateRatio));
				deathRateRatioHIV *= ARTDeathRateRatio;
			}
			// Check for severe OI history in the HIV positive children and modify the HIV death rate ratio by the maximum of any applicable severe OI history death rate ratios
			for(int oiType=0; oiType <SimContext::OI_NUM; oiType++){
				// Check for a history of the OI and whether the OI is designated as severe
				if(patient->getDiseaseState()->hasTrueOIHistory[oiType] && simContext->getRunSpecsInputs()->severeOIs[oiType]){
					int monthsSince = monthNum - patient->getDiseaseState()->lastMonthSevereOI[oiType];
					// Check if this is TB being modeled as a severe OI when the TB tab is not enabled 
					if(!simContext->getTBInputs()->enableTB && simContext->getRunSpecsInputs()->OIsIncludeTB && oiType==0){
						if(monthNum > 0 && monthsSince <= simContext->getPedsInputs()->TB_OIHistEffectDurationPedsEarly){
							OIHistDeathRateRatio = simContext->getPedsInputs()->TB_OIHistDeathRateRatioPedsEarly[pedsAgeCat];
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);	
						}
					}
					// Otherwise, it is a regular severe OI
					else{
						if(monthNum > 0 && monthsSince <= simContext->getPedsInputs()->severeOIHistEffectDurationPedsEarly){
							OIHistDeathRateRatio = simContext->getPedsInputs()->severeOIHistDeathRateRatioPedsEarly[pedsAgeCat];
							maxOIHistDeathRateRatio = max(maxOIHistDeathRateRatio, OIHistDeathRateRatio);						
						}
					}
				}	
			}
			deathRateRatioHIV *= maxOIHistDeathRateRatio;	
			if (deathRateRatioHIV > 1.0) {
				addMortalityRisk(SimContext::DTH_HIV, deathRateRatioHIV);
			}	
		}
	}		
	/** Calculate the probability of the patient dying this month by first calculating the death rate based on current risks and then converting to a probability */
	bool deathOccurs = false;
	int causeOfDeathId = 0;
	double probDeath = 0.0;

	const vector<SimContext::MortalityRisk> &mortalityRisks = patient->getDiseaseState()->mortalityRisks;
	int numRisks = mortalityRisks.size();
	double rateRatioProduct = 1.0;
	for(int i=0;i< numRisks;i++){
		rateRatioProduct *= mortalityRisks[i].deathRateRatio;
	}
	probDeath = CepacUtil::rateToProb(rateRatioProduct);
	double randNum = CepacUtil::getRandomDouble(120010, patient);
	if (randNum < probDeath)
		deathOccurs = true;
	if (deathOccurs) {
		double *indivDeathRate = new double[numRisks];
		double sumOfRates = 0.0;
		for (int i = 0; i < numRisks; i++){
			if (mortalityRisks[i].causeOfDeath == SimContext::DTH_BKGD_MORT){
				indivDeathRate[i] = natHist->monthlyBackgroundDeathRate[gender][ageYears];
			}
			else{
				indivDeathRate[i] = natHist->monthlyBackgroundDeathRate[gender][ageYears] * mortalityRisks[i].deathRateRatio;
			}
			sumOfRates += indivDeathRate[i];
		}
		randNum = CepacUtil::getRandomDouble(120020, patient);
		for (int i = 0; i < numRisks; i++){
			indivDeathRate[i] = indivDeathRate[i] / sumOfRates;
			if ((indivDeathRate[i] > 0) && (randNum < indivDeathRate[i])){
				causeOfDeathId = i;
				break;
			}
			randNum -= indivDeathRate[i];
		}
		delete[] indivDeathRate;

		/** Perform death-related updates */
		/* Set unfavorable outcome for TB if death occurs during initial treatment*/
		if (simContext->getTBInputs()->enableTB){
			if ((patient->getTBState()->isOnTreatment || patient->getTBState()->isOnEmpiricTreatment) && !patient->getTBState()->everHadNonInitialTreatmentOrEmpiric)
				setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_DEATH);
		}
		/** Update the patient state to reflect the fact that death has occurred */
		setCauseOfDeath(mortalityRisks[causeOfDeathId].causeOfDeath);
		/** add cost here */
		incrementCostsMisc(mortalityRisks[causeOfDeathId].costDeath, 1.0);
		
		// if this is an HIV negative child, check if they died having never been exposed to HIV
		if(pedsAgeCat < SimContext::PEDS_AGE_LATE && patient->getDiseaseState()->infectedPediatricsHIVState == SimContext::PEDS_HIV_NEG){
			updatePedsNeverExposed(true);
		}		
	}
} /* end performMonthlyUpdates */
