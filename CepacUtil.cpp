#include "include.h"

/** \brief Empty constructor and destructor, should never create an instance of this class */
CepacUtil::CepacUtil(void)
{
}
/** \brief Empty constructor and destructor, should never create an instance of this class */
CepacUtil::~CepacUtil(void)
{
}

/* Constant string values for CEPAC version and file/directory information */
/** The CEPAC Input Version to match that from the .in file */
const char *CepacUtil::CEPAC_INPUT_VERSION = "20210615";
/** CEPAC version string: the version label commonly used in CEPAC vernacular */
const char *CepacUtil::CEPAC_VERSION_STRING = "50d";
/** The compile date of the most recent release */
const char *CepacUtil::CEPAC_EXECUTABLE_COMPILED_DATE = "2023-10-09";
/** .tmp */
const char *CepacUtil::FILE_EXTENSION_FOR_TEMP = ".tmp";
/** .txt */
const char *CepacUtil::FILE_EXTENSION_FOR_TRACE = ".txt";
/** .out */
const char *CepacUtil::FILE_EXTENSION_FOR_OUTPUT = ".out";
/** .cout */
const char *CepacUtil::FILE_EXTENSION_FOR_COSTS_OUTPUT = ".cout";
/** .orph */
const char *CepacUtil::FILE_EXTENSION_FOR_ORPHAN_OUTPUT = ".orph";
/** .in */
const char *CepacUtil::FILE_EXTENSION_FOR_INPUT = ".in";
/** *.in */
const char *CepacUtil::FILE_EXTENSION_INPUT_SEARCH_STR = "*.in";
/** popstats.out */
const char *CepacUtil::FILE_NAME_SUMMARIES = "popstats.out";

/** Vector of the file names to be run*/
std::vector<std::string> CepacUtil::filesToRun;
/** The inputs directory path */
std::string CepacUtil::inputsDirectory;
/** The output directory path */
std::string CepacUtil::resultsDirectory;
/** True if we're using random seed, false for fixed seed */
bool CepacUtil::useRandomSeedByTime;

/** \brief Random number generator class */
MTRand CepacUtil::mtRand;

/** \brief useCurrentDirectoryForInputs determines the current directory and sets as inputs directory */
void CepacUtil::useCurrentDirectoryForInputs() {
#if defined(_WIN32)
	char buffer[512];
	_getcwd(buffer, 512);
	inputsDirectory = buffer;
#else
	char buffer[512];
	getcwd(buffer, 512);
	inputsDirectory = buffer;
#endif
} /* end useCurrentDirectoryForInputs */

/** \brief findInputFiles locates all the .in files in the current directory and adds them
	to the filesToRun vector */
void CepacUtil::findInputFiles() {
#if defined(_WIN32)
	intptr_t hFile;
	struct _finddata_t tFileInfo;
	hFile = _findfirst( FILE_EXTENSION_INPUT_SEARCH_STR, &tFileInfo );
	int nInputFiles = 0;
	string fileName;

	//get the list of files that we have to process
	filesToRun.clear();
	do {
		fileName = (char *) tFileInfo.name;
		filesToRun.push_back(fileName);
		nInputFiles++;
	} while ( _findnext ( hFile, &tFileInfo ) == 0 );
	_findclose( hFile );
#else
	glob_t files;
	glob(FILE_EXTENSION_INPUT_SEARCH_STR, GLOB_ERR, NULL, &files);
	int nInputFiles = 0;
	string fileName;

	filesToRun.clear();
	//get the list of files that we have to process
	int i;
	for( i = 0; i < files.gl_pathc; i++) {
		fileName = (char *) files.gl_pathv[i];
		filesToRun.push_back(fileName);
		++nInputFiles;
	}
	globfree( &files);
#endif
} /* end findInputFiles */

/** \brief createResultsDirectory creates the directory "results" as a subdirectory of the inputs one */
void CepacUtil::createResultsDirectory() {
#if defined(_WIN32)
	resultsDirectory = inputsDirectory;
	resultsDirectory.append("\\");
	resultsDirectory.append("results");
	_mkdir(resultsDirectory.c_str());
#else
	resultsDirectory = inputsDirectory;
	resultsDirectory.append("/");
	resultsDirectory.append("results");
	mkdir(resultsDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
} /* end createResultsDirectory */

/** \brief changeDirectoryToResults changes the working directory to the results one */
void CepacUtil::changeDirectoryToResults() {
#if defined(_WIN32)
	_chdir(resultsDirectory.c_str());
#else
	chdir(resultsDirectory.c_str());
#endif
} /* end changeDirectoryToResults */

/** \brief changeDirectoryToInputs changes the working directory to the inputs one */
void CepacUtil::changeDirectoryToInputs() {
#if defined(_WIN32)
	_chdir(inputsDirectory.c_str());
#else
	chdir(inputsDirectory.c_str());
#endif
} /* end changeDirectoryToInputs */

/** \brief getDateString places the current date string in the specified buffer
 *  \param buffer a pointer to a char array representing the buffer to add the resulting date string to
 *  \param bufsize an integer representing the size of buffer
 **/
void CepacUtil::getDateString(char *buffer, int bufsize) {
#if defined(_WIN32)
	_strdate(buffer);
#else
	time_t currTime;
	time(&currTime);
	strftime(buffer, bufsize, "%m/%d/%y",localtime(&currTime));
#endif
} /* end getDateString */

/** \brief getTimeString places the current system time string in the specified buffer
 *  \param buffer a pointer to a char array representing the buffer to add the resulting time string to
 *  \param bufsize an integer representing the size of buffer */
void CepacUtil::getTimeString(char *buffer, int bufsize) {
#if defined(_WIN32)
	_strtime(buffer);
#else
	time_t currTime;
	time(&currTime);
	strftime(buffer, bufsize, "%H:%M:%S",localtime(&currTime));
#endif
} /* end getTimeString */

/** \brief fileExists returns true if the specified file exists, false otherwise
 * \param filename a pointer to a character array representing the name of the file
 **/
bool CepacUtil::fileExists(const char *filename) {
	FILE *file;
	//fopen_s(&file, filename, "r");
	file = fopen(filename, "r");
	if (!file)
		return false;
	fclose(file);
	return true;
} /* end fileExists */

/** \brief openFile opens the specified file in the given mode
 * \param filename a pointer to a character array representing the name of the file
 * \param mode a pointer to a character array representing the mode to open the file in: "r" for read, "w" for write, "a" for append, "r+" for reading and writing an existing file, "w+" for reading and writing an empty file, "a+" for reading and appending to a file
 **/
FILE *CepacUtil::openFile(const char *filename, const char *mode) {
	FILE *file = fopen(filename, mode);
	return file;
} /* end openFile */

/** \brief closeFile closes the specified file
 * \param filename a pointer to a character array representing the name of the file
 **/
void CepacUtil::closeFile(FILE *file) {
	fclose(file);
} /* end closeFile */
