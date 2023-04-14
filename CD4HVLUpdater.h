#pragma once

#include "include.h"

/**
	CD4HVLUpdater updates the patient's monthly CD4 and HVL levels according to their natural
	history disease progression and ART treatment effects.
*/
class CD4HVLUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	CD4HVLUpdater(Patient *patient);
	~CD4HVLUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
};
