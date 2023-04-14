#pragma once

#include "include.h"

/**
	DrugEfficacyUpdater is a state updater that handles the efficacy changes of drug treatments
	that the patient is on.  This includes ART efficacy state change such as late failure and
	prophylaxis resistance.
*/
class DrugEfficacyUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	DrugEfficacyUpdater(Patient *patient);
	~DrugEfficacyUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:
	/* performARTEfficacyUpdates handles all efficacy updates for ART regimens */
	void performARTEfficacyUpdates();
	/* performARTEnvelopeEfficacyUpdates handles the efficacy updates of the ART CD4 envelope */
	void performARTEnvelopeEfficacyUpdates();
	/* performProphEfficacyUpdates handles all efficacy updates for acute OI prophylaxis */
	void performProphEfficacyUpdates();
};
