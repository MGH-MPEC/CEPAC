#pragma once

#include "include.h"

/**
	The CHRMsUpdater class tests for the initiation of chronic events (CHRMs) each month and
	the continuing effects of existing ones.  Performs all updates of the patient state and
	statistics for the chronic events.
*/
class CHRMsUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	CHRMsUpdater(Patient *patient);
	~CHRMsUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:

};
