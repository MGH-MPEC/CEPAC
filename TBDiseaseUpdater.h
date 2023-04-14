#pragma once

#include "include.h"

/**
	TBDiseaseUpdater is a state updater that handles the occurence of acute TB and the
	disease progression of reactivation, reinfection, and relapse.
	It also handles the effects of TB treatments and the switch of
	treatment after failure (though this may be moved into the clinic visit at some point).
*/
class TBDiseaseUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	TBDiseaseUpdater(Patient *patient);
	~TBDiseaseUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:
	/* performUninfectedTBUpdates manages TB updates for uninfected population*/
	void performUninfectedTBUpdates();
	/* performLatentTBUpdates manages TB updates for latent state */
	void performLatentTBUpdates();
	/* performActivePulmTBUpdates manages TB updates for active pulmonary state */
	void performActivePulmTBUpdates();
	/* performActiveExtraPulmTBUpdates manages TB updates for active extra pulmonary state */
	void performActiveExtraPulmTBUpdates();
	/* performPrevTreatTBUpdates manages TB updates for Prev Treated TB state */
	void performPrevTreatTBUpdates();
	/* performTreatDefaultTBUpdates manages TB updates for TB Treatment default state */
	void performTreatDefaultTBUpdates();
	/* rollForTBSymptoms checks to see if patients aquire TB symptoms that month and also clears TB symptoms from the previous month (active TB states do not clear symptoms)*/
	void rollForTBSymptoms();
	/* rollForInfection handles Infection and Reinfection from the uninfected, latent, prev treated or default tb states*/
	void rollForInfection(SimContext::TB_STATE tbState);
	/* rollForRelapse handles Relapse from the treated or default states to the active pulm or active extra pulm states*/
	void rollForRelapse(SimContext::TB_STATE tbState);

};
