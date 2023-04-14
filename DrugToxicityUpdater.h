#pragma once

#include "include.h"

/**
	DrugToxicityUpdater is an updater that handles all the toxicity events and effects
	for ART regimens, prophylaxis, and TB treatments.
*/
class DrugToxicityUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	DrugToxicityUpdater(Patient *patient);
	~DrugToxicityUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:
	/* performARTToxicityUpdates handles all toxicity updates for ART regimens */
	void performARTToxicityUpdates();
	/* performProphToxicityUpdates handles all toxicity updates for prophylaxis */
	void performProphToxicityUpdates();
	/* performTBProphToxicityUpdates handles all toxicity updates for TB prophylaxis */
	void performTBProphToxicityUpdates();
	/* performTBTreatmentToxicityUpdates handles all toxicity updates for TB treatment */
	void performTBTreatmentToxicityUpdates();
	/* evaluateARTToxDuration evaluates if the given ART toxicity duration criteria are still met */
	bool evaluateARTToxDuration(const SimContext::ARTToxicityEffect &toxEffect, SimContext::ART_TOX_DUR duration);
};
