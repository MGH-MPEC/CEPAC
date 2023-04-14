#include "include.h"

// Disable warning for unsafe use of this pointer in initialization,
//	state updater constructor only copies the pointer and does not access any of its fields so it is safe
#pragma warning(disable:4355)

/** Constructor takes in the patient number, simulation context, run stats object, and tracing object
	Initializes all subclass state values

	\param *simContext a pointer to the SimContext the patient should use for inputs
	\param *runStats a pointer to the RunStats the patient should use for outputs
	\param *costStats a pointer to the RunStats the patient should use for outputs
	\param *tracer a pointer to the Tracer used for tracing the patient
	\param _predefinedAgeAndGender a bool that is true if the patient's age and gender are predefined (likely by the transmission model) instead of drawn from the SimContext distribution
	\param _ageMonths an integer specifying the initial age of the patient IF it is not to be drawn from the SimContext distribution
	\param _gender a SimContext::GENDER_TYPE specifying the patient's gender IF it is not to be drawn from the SimContext distribution
	\param _setAsIncidentCase a boolean that is true if this is an incident HIV case (determined likely by the transmission model)
	\param startingMonth an integer specifying the month the patient should start in (for syncing up the patients in the transmission model)
**/
Patient::Patient(SimContext *simContext, RunStats *runStats, CostStats *costStats, Tracer *tracer,  bool _predefinedAgeAndGender, int _ageMonths, SimContext::GENDER_TYPE _gender, bool _setAsIncidentCase, int startingMonth) :
		simContext(simContext),
		runStats(runStats),
		costStats(costStats),
		tracer(tracer),
		beginMonthUpdater(this),
		hivInfectionUpdater(this),
		chrmsUpdater(this),
		drugToxicityUpdater(this),
		tbDiseaseUpdater(this),
		acuteOIUpdater(this),
		mortalityUpdater(this),
		cd4HVLUpdater(this),
		hivTestingUpdater(this),
		behaviorUpdater(this),
		drugEfficacyUpdater(this),
		cd4TestUpdater(this),
		hvlTestUpdater(this),
		clinicVisitUpdater(this),
		tbClinicalUpdater(this),
		endMonthUpdater(this)
{
	//Determine if Age and Gender and Incident Case status are input defined rather than drawn from a distribution
	this->generalState.predefinedAgeAndGender = _predefinedAgeAndGender;
	if (this->generalState.predefinedAgeAndGender){
		this->generalState.ageMonths = _ageMonths;
		this->generalState.gender = _gender;
		this->diseaseState.isPrevalentHIVCase = !(_setAsIncidentCase);
	}

	//Set the initial time
	this->generalState.initialMonthNum = startingMonth;

	// initialize uninitialized variables
	this->tbState.hasTrueHistoryTB = false;
	this->tbState.observedHistActiveTBAtEntry = false;
	this->tbState.hadProph = false;
	this->diseaseState.hasDrawnPatientSpecificCD4Decline = false;
	this->monitoringState.hasScheduledCD4Test = false;
	this->monitoringState.hasScheduledHVLTest = false;
	this->monitoringState.hasObservedCD4 = false;
	this->monitoringState.hasObservedCD4Percentage = false;
	this->monitoringState.hasObservedCD4NonLabStaging = false;

	//Clear vectors
	this->pedsState.eidPendingTestResults.clear();
	this->pedsState.eidScheduledConfirmatoryTests.clear();

	beginMonthUpdater.performInitialUpdates();
	hivInfectionUpdater.performInitialUpdates();
	chrmsUpdater.performInitialUpdates();
	drugToxicityUpdater.performInitialUpdates();
	tbDiseaseUpdater.performInitialUpdates();
	acuteOIUpdater.performInitialUpdates();
	mortalityUpdater.performInitialUpdates();
	cd4HVLUpdater.performInitialUpdates();
	hivTestingUpdater.performInitialUpdates();
	behaviorUpdater.performInitialUpdates();
	drugEfficacyUpdater.performInitialUpdates();
	cd4TestUpdater.performInitialUpdates();
	hvlTestUpdater.performInitialUpdates();
	clinicVisitUpdater.performInitialUpdates();
	tbClinicalUpdater.performInitialUpdates();
	endMonthUpdater.performInitialUpdates();
}

/** Cleanup the state updater classes */
Patient::~Patient(void) {
	this->pedsState.eidPendingTestResults.clear();
	this->pedsState.eidScheduledConfirmatoryTests.clear();
}

/** simulateMonth runs a single month of simulation for this patient, and updates
	its state and runStats statistics */
void Patient::simulateMonth() {

	beginMonthUpdater.performMonthlyUpdates();

	/** Disease and General Health updaters are called */
	/** If this is being run in the transmission model (i.e. age and gender were predefined) ignore all incidence (i.e. do not perform hivInfection updates until after infection)*/
	if (!(generalState.predefinedAgeAndGender && diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)){
		hivInfectionUpdater.performMonthlyUpdates();
	}
	chrmsUpdater.performMonthlyUpdates();
	drugToxicityUpdater.performMonthlyUpdates();

	if (diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		acuteOIUpdater.performMonthlyUpdates();
	}

	tbDiseaseUpdater.performMonthlyUpdates();
	mortalityUpdater.performMonthlyUpdates();

	if (!diseaseState.isAlive) {
		endMonthUpdater.performMonthlyUpdates();
		return;
	}

	if (diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		cd4HVLUpdater.performMonthlyUpdates();
	}

	/** Treatment, Monitoring, and Behavior updaters are called */
	hivTestingUpdater.performMonthlyUpdates();

	if (diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {

		behaviorUpdater.performMonthlyUpdates();
		drugEfficacyUpdater.performMonthlyUpdates();
		cd4TestUpdater.performMonthlyUpdates();
		hvlTestUpdater.performMonthlyUpdates();
		clinicVisitUpdater.performMonthlyUpdates();
	}
	tbClinicalUpdater.performMonthlyUpdates();
	endMonthUpdater.performMonthlyUpdates();

} /* endSimulateMonth */


