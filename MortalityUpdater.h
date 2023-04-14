#pragma once

#include "include.h"

/**
	The MortalityUpdater class is a state updater that checks for the occurrence of
	death each month, including calculating HIV and background mortality, and updates the patient's state if the patient dies.
*/
class MortalityUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	MortalityUpdater(Patient *patient);
	~MortalityUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
