#pragma once

#include "include.h"

/*
	CepacUtil contains the platform specific utility functions for handling files/directories,
	generating random numbers, and determining the current date/time.
*/
class CepacUtil
{
public:
	/* Empty constructor and destructor, should never create an instance of this class */
	CepacUtil(void);
	~CepacUtil(void);

	/* Constant values for CEPAC version and file/directory information */
	static const char *CEPAC_INPUT_VERSION;
	static const char *CEPAC_VERSION_STRING;
	static const char *CEPAC_EXECUTABLE_COMPILED_DATE;
	static const char *FILE_EXTENSION_FOR_TEMP;
	static const char *FILE_EXTENSION_FOR_TRACE;
	static const char *FILE_EXTENSION_FOR_OUTPUT;
	static const char *FILE_EXTENSION_FOR_COSTS_OUTPUT;
	static const char *FILE_EXTENSION_FOR_ORPHAN_OUTPUT;
	static const char *FILE_EXTENSION_FOR_INPUT;
	static const char *FILE_EXTENSION_INPUT_SEARCH_STR;
	static const char *FILE_NAME_SUMMARIES;

	/* Vector of the file names to be run, and the inputs and results directories paths */
	static std::vector<std::string> filesToRun;
	static std::string inputsDirectory;
	static std::string resultsDirectory;

	/* Functions for handling directories and locating the input files */
	static void useCurrentDirectoryForInputs();
	static void findInputFiles();
	static void createResultsDirectory();
	static void changeDirectoryToResults();
	static void changeDirectoryToInputs();

	/* Functions for returning the current system date and time */
	static void getDateString(char *buffer, int bufsize);
	static void getTimeString(char *buffer, int bufsize);

	/* Functions and state variables for generating uniform and gaussian random numbers */
	static void setRandomSeedType(bool useTimeSeed);
	static void setFixedSeed(Patient *patient);
	static double getRandomDouble(int callSiteId, Patient *patient);
	static double getRandomGaussian(double mean, double stdDev, int callSiteId, Patient *patient);
	static int getRandomDiscrete(int min, int numOutcomes);
	static bool useRandomSeedByTime;
	static MTRand mtRand;

	/* Probability modification functions */
	static double probToRate(double prob);
	static double rateToProb(double rate);
	static double probRateMultiply(double prob, double rateMult);
	static double probToLogit(double prob);
	static double logitToProb(double logit);
	static double probLogitAdjustment(double prob, double logitAdjust);

	/* Functions for opening and closing files */
	static bool fileExists(const char *filename);
	static FILE *openFile(const char *filename, const char *mode);
	static void closeFile(FILE *file);
};

/** \brief setRandomSeedType sets up the random number generator to use seed by time (i.e. random seed) or fixed seed
 *
 * \param useTimeSeed a boolean that determines whether to use fixed or random seed: if true, use random, else use fixed
 **/
inline void CepacUtil::setRandomSeedType(bool useTimeSeed) {
	useRandomSeedByTime = useTimeSeed;
	if (useRandomSeedByTime)
		mtRand.seed((unsigned int) time(0));
	else
		mtRand.seed(8675309);
} /* end setRandomSeedType */

/** \brief setfixed seed type sets the seed using the patient number
 *
 */
inline void CepacUtil::setFixedSeed(Patient *patient){
	mtRand.seed(patient->getGeneralState()->patientNum);
}/* end setFixedSeed */

/** \brief getRandomDouble returns a random number within the range [0,1)
 *
 * \param callSiteId an integer specifying what function called the random number generator: was used for synchronized fixed seed and now no longer has a function
 * \param patient a pointer to the Patient; was used for synchronized fixed seed and now no longer has a function
 * \return a double randomly selected in the range [0,1)
 **/
inline double CepacUtil::getRandomDouble(int callSiteId, Patient *patient) {
	return mtRand();
} /* end getRandomDouble */

/** \brief getRandomGaussian returns a random normally distributed value with the specified mean and standard deviation
 *
 * \param mean a double representing the mean of the normal distribution
 * \param stdDev a double representing the standard deviation of the normal distribution
 * \param callSiteId an integer specifying what function called the random number generator: was used for synchronized fixed seed and now no longer has a function
 * \param patient a pointer to the Patient; was used for synchronized fixed seed and now no longer has a function
 * \return a double randomly selected from the defined distribution
 **/
inline double CepacUtil::getRandomGaussian(double mean, double stdDev, int callSiteId, Patient *patient) {
	// Polar form of Box-Muller transformation
    double x1, x2, w, y1, y2;
	do {
		x1 = 2.0 * mtRand() - 1.0;
		x2 = 2.0 * mtRand() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );
	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;

	return (mean + (y2 * stdDev));
} /* end getRandomGaussian */

/** \brief getRandomDiscrete returns a random integer from a discrete uniform integer distribution containing the number of possible values specified
 * \param min an int for the minimum allowed return value
 * \param numOutcomes an int for the number of possible outcomes
 * \return an integer from the specified set of possible outcomes
 * */
inline int CepacUtil::getRandomDiscrete(int min, int numOutcomes){
	double randNum = mtRand();
	int outcome = (int) (randNum * numOutcomes) + min;
	return outcome;
} /* end getRandomDiscrete */

/** \brief probToRate converts a probability to a rate
 *
 * \f$ rate = -\log (1 - prob) \f$
 *
 * \param prob a double representing the probability to be converted to a rate
 * \return a double representing the equivalent rate
 **/
inline double CepacUtil::probToRate(double prob) {
	return (-1 * log(1 - prob));
} /* end probToRate */

/** \brief rateToProb converts a rate to a probability
 *
 *  \f$ prob = 1 - e^{-rate} \f$
 *
 * \param rate a double representing the rate to be converted to a probability
 * \return a double representing the equivalent probability
 **/
inline double CepacUtil::rateToProb(double rate) {
	return (1 - exp(-1 * rate));
} /* end rateToProb */

/** \brief probRateMultiply modifies a probability by a rate multiplier
 *
 * \f$ return = 1 - (1 - prob)^{rateMult} \f$
 *
 * \param prob a double representing the probability to be multiplied
 * \param rateMult a double representing the rate multiplier to apply to the probability
 * \return a double representing the adjusted probability
 **/
inline double CepacUtil::probRateMultiply(double prob, double rateMult) {
	// Formula is derived from conversion to rate, perform multiply, and convert back to prob
	if (rateMult == 0)
		return 0;
	if (rateMult == 1)
		return prob;
	return (1 - pow(1 - prob, rateMult));
} /* end probRateMultiply */

/** \brief probToLogit converts a probability to a logit
 *
 * \f$ logit = \log(\frac{prob}{1 - prob}) \f$
 *
 * \param prob a double representing the probability to be converted to a logit
 * \return a double representing the equivalent logit
 **/
inline double CepacUtil::probToLogit(double prob) {
	//Watch out for invalid values!
	return log(prob / (1 - prob));
}

/** \brief logitToProb converts a logit to a probability
 *
 * \f$ prob = \frac{1}{1 + e^{-logit}} \f$
 *
 * \param logit a double representing the logit to be converted to a probability
 * \return a double representing the equivalent probability
 **/
inline double CepacUtil::logitToProb(double logit) {
	return 1 / (1 + exp(-1 * logit));
}

/** \brief probLogitAdjustment takes a probability and adjusts it by a given logit by converting the probability to a logit, summing the logits together, and converting back to a probability
 *
 * \param prob a double representing the initial probability
 * \param logitAdjust a double representing the logit factor to add to the probability
 * \return a double representing the new adjusted probability
 **/
inline double CepacUtil::probLogitAdjustment(double prob, double logitAdjust) {
	return logitToProb(probToLogit(prob) + logitAdjust);
}

