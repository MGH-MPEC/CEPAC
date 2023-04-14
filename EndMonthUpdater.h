#pragma once

#include "include.h"

/**
	The EndMonthUpdater class is a state updater that is run at the end of each simulated
	month.  It updates and accumulates all necessary patient state and statistics for that
	month, and generates the monthly tracing.  It is also handles the final processing after
	patient death and uses a half a month for survival and cost accounting.
*/
class EndMonthUpdater : public StateUpdater
{
public:
	/* Constructor and Destructor */
	EndMonthUpdater(Patient *patient);
	~EndMonthUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
