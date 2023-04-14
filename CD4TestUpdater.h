#pragma once

#include "include.h"

/**
	CD4TestUpdater handles the clinical CD4 testing of patients and the associated state
	updates.  It usually coincides with clinic visits but was split out to handle CD4 tests
	that occur outside of clinic visits due to ART initiation or repeat ART failure testing.
*/
class CD4TestUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	CD4TestUpdater(Patient *patient);
	~CD4TestUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
