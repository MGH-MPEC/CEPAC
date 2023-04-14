#pragma once

#include "include.h"

/**
	HVLTestUpdater handles the clinical HVL testing of patients and the associated state
	updates.  It usually coincides with clinic visits but was split out to handle HVL tests
	that occur outside of clinic visits due to ART initiation or repeat ART failure testing.
*/
class HVLTestUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	HVLTestUpdater(Patient *patient);
	~HVLTestUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
