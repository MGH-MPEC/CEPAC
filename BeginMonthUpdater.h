#pragma once

#include "include.h"

/**
	BeginMonthUpdater does the initial resetting of state and statistics values at the beginning
	of each simulated month.  It is also responsible for initializing all the patient state
	and updating initial distribution stats for the first simulated month.
*/
class BeginMonthUpdater : public StateUpdater {
public:
	/* Constructor and Destrcutor */
	BeginMonthUpdater(Patient *patient);
	~BeginMonthUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
