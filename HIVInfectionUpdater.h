#pragma once

#include "include.h"

/**
	HIVInfectionUpdater handles transitions related to HIV Infection and updates statistics. It covers the transition from acute to chronic HIV, incident infection, and age-related transitions. 
*/
class HIVInfectionUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	HIVInfectionUpdater(Patient *patient);
	~HIVInfectionUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

	/* performHIVNewInfectionUpdates handles the new infection of an HIV negative patient*/
	//This function has to be public so that the Transmission model can access it
	void performHIVNewInfectionUpdates();

private:
	/* performHIVNegativeUpdates determines if HIV negative patients become infected */
	void performHIVNegativeUpdates();
	/* performAcuteToChronicHIVUpdates handles the transition from acute to chronic HIV */
	void performAcuteToChronicHIVUpdates();
	/* performPediatricDiseaseUpdates determines maternal updates and early to late childhood transition */
	void performPediatricDiseaseUpdates();
	/* performAdolescentDiseaseUpdates determines adolescent transitions */
	void performAdolescentDiseaseUpdates();
	/* drawCD4Slope finds the mean and SD for the monthly CD4 change on suppressive ART and returns the new slope value */
	double drawCD4Slope(int artLineNum, int monthsSinceStart);
	/* performCD4EnvelopeAgeTransition updates the overall and individual CD4 envelope slopes during age category transitions */
	void performCD4EnvelopeAgeTransition(bool setOverallEnvelopeSlope, bool setIndivEnvelopeSlope );
	/* evaluatePedsHIVProphPolicy determines if the starting criteria for the indexed Peds Proph has been met */
	bool evaluatePedsHIVProphPolicy(const SimContext::EIDInputs::InfantHIVProph &infantHIVProph);
};
