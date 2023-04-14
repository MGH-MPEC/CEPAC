#pragma once

#include "include.h"

/**
	BehaviorUpdater is a state updater that handles behavioral issues such as compliance
	and adherence.  For now this only contains loss to follow up and return to care, but
	it will eventually include drug use, adherence, pregnancy, etc.
*/
class BehaviorUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	BehaviorUpdater(Patient *patient);
	~BehaviorUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
