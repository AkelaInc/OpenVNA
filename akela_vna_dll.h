#ifndef __AKELA_VNA_DLL_H
#define __AKELA_VNA_DLL_H


/** \addtogroup C-API
 * \authors Stephen Hunt (shunt@akelainc.com), Connor Wolf (cwolf@akelainc.com)
 *
 *  \section state-overview Run-state overview
 *
 *  A Task exists in one of three states:
 *  1. Uninitialized (`TASK_UNINITIALIZED`)
 *  2. Stopped (`TASK_STOPPED`)
 *  3. Started (`TASK_STARTED`)
 *
 *  When the object is first created, it is in the uninitialized state.
 *  Here is the state table. The cell content is the new state. Blank cells
 *  mean the action is ignored.
 *
 *  |      State        |              |  Action |         |                |
 *  |-------------------|--------------|---------|---------|----------------|
 *  |   Current state   | initialize() | start() | stop()  | setIPAddress() |
 *  | uninitialized     | stopped      |         |         | uninitialized  |
 *  | stopped           |              | started |         | uninitialized  |
 *  | started           |              |         | stopped |                |
 *
 *  The initialize() action ensures that the AVMU unit is online and
 *  is responding to commands.  It also downloads the hardware details from
 *  the unit needed to properly program the unit.

 *  The start() action programs the AVMU unit. At this stage the unit is able
 *  to respond to measurement commands.

 *  The stop() action idles the unit.

 *  The setIPAddress() action puts the state back to uninitialized, because
 *  the assumption is that a different VNA unit is going to be targeted.
 *  Hardware details can vary from unit to unit and so those details must be
 *  re-downloaded prior to programming the new unit.
 *  NOTE: any calibration data is marked invalid when the state transitions
 *  to uninitialized. If you would like to use the calibration on a different
 *  unit (or save it for later), see the exportCalibration() function.
 *
 */

//Do not define the VNADLL_EXPORTS symbol when linking to this DLL
#ifdef WIN32
	#ifdef VNADLL_EXPORTS
		#define VNADLL_API __declspec(dllexport)
	#else
		#define VNADLL_API __declspec(dllimport)
	#endif
#endif
#ifdef LINUX
	#ifdef VNADLL_EXPORTS
		#define __attribute__((visibility("default")))
	#else
		#define VNADLL_API
	#endif
#endif


/** \addtogroup C-API
 *  @{
 */

extern "C" {

	/**
	 * @brief Opaque type for containing the parameters and associated
	 *        resources for interfacing with a single VNA.
	 *
	 *        One piece of software can have multiple tasks open at
	 *        any one time, though having multiple tasks for the same
	 *        VNA simultaneouslt can cause undetermined behaviour.
	 *        At any time, there should only ever be one non-`TASK_UNINITIALIZED`
	 *        task for a single VNA.
	 *
	 */
	typedef struct task_t* TaskHandle;

	/** \addtogroup ErrorCodes
	 *  @brief Potential error-codes API calls can return.
	 *
	 *  @{
	 */
	/**
	 * Error-Code value type. Treat this as an opaque type.
	 */
	typedef int ErrCode;
	extern VNADLL_API ErrCode ERR_OK;                 //!< Return code for when there was no error.
	extern VNADLL_API ErrCode ERR_BAD_ATTEN;          //!< Error code for an invalid attenuation setting
	extern VNADLL_API ErrCode ERR_BAD_CAL;            //!< Error code for a missing or invalid calibration
	extern VNADLL_API ErrCode ERR_BAD_HANDLE;         //!< Error code for when the passed Task is not valid.
	extern VNADLL_API ErrCode ERR_BAD_HOP;            //!< Error code for an invalid hop-rate setting
	extern VNADLL_API ErrCode ERR_BAD_PATH;           //!< Error code for an invalid path setting
	extern VNADLL_API ErrCode ERR_FREQ_OUT_OF_BOUNDS; //!< Error code indicating the start or end frequency was beyond the hardware's capabilities.
	extern VNADLL_API ErrCode ERR_INTERRUPTED;        //!< Error code for when the `initialize()` or `measureNNNN()` call was interrupted by the application code.
	extern VNADLL_API ErrCode ERR_NO_RESPONSE;        //!< Error code for when the VNA failed to respond to communications.
	extern VNADLL_API ErrCode ERR_MISSING_IP;         //!< Error code for when the task does not have a VNA IP configured.
	extern VNADLL_API ErrCode ERR_MISSING_PORT;       //!< Error code for when the task does not have a VNA port configured.
	extern VNADLL_API ErrCode ERR_MISSING_HOP;        //!< Error code for when the task does not have a hop-rate configured.
	extern VNADLL_API ErrCode ERR_MISSING_ATTEN;      //!< Error code for when the task does not have a attenuation value configured.
	extern VNADLL_API ErrCode ERR_MISSING_FREQS;      //!< Error code for when the task does not have the measurement frequencies configured.
	extern VNADLL_API ErrCode ERR_PROG_OVERFLOW;      //!< Error code for when the number of points requested would be too large for the VNA memory.
	extern VNADLL_API ErrCode ERR_SOCKET;             //!< Error code for when there was a local socket error when communicating with the VNA.
	extern VNADLL_API ErrCode ERR_TOO_MANY_POINTS;    //!< Error code for when the number of points requested is greater then the maximum in \ref HardwareDetails.
	extern VNADLL_API ErrCode ERR_WRONG_STATE;        //!< Error code for when a call was made while the task was in the incorrect state for that call.

	//! Error code indicating that the VNA PROM was not in a known format.  Please contact support if this is a reoccuring issue
	extern VNADLL_API ErrCode ERR_BAD_PROM;
	//! Error code indicating the DLL has received an incorrect number of bytes from the VNA. Possibly indicative of a network issue.
	extern VNADLL_API ErrCode ERR_BYTES;
	/** @}*/


	/** \addtogroup HopRateSettings
	 *  @brief Settings for the hop-rate (e.g. time spent sampling each
	 *         frequency point) in a sweep.
	 *
	 *         Faster hop rates result in lower dynamic range (but faster sweeps).
	 *
	 *  @{
	 */

	/**
	 * Hop-Rate value type. Treat this as an opaque type.
	 */
	typedef int HopRate;

	extern VNADLL_API HopRate HOP_UNDEFINED;  //!< Hop rate not set yet
	/* extern VNADLL_API HopRate HOP_90K; */  // This rate is currently unsupported but may be enabled in the future
	extern VNADLL_API HopRate HOP_45K;  //!< Hop rate of 45K points/second
	extern VNADLL_API HopRate HOP_30K;  //!< Hop rate of 30K points/second
	extern VNADLL_API HopRate HOP_15K;  //!< Hop rate of 15K points/second
	extern VNADLL_API HopRate HOP_7K;   //!< Hop rate of 7K points/second
	extern VNADLL_API HopRate HOP_3K;   //!< Hop rate of 3K points/second
	extern VNADLL_API HopRate HOP_2K;   //!< Hop rate of 2K points/second
	extern VNADLL_API HopRate HOP_1K;   //!< Hop rate of 1K points/second
	extern VNADLL_API HopRate HOP_550;  //!< Hop rate of 550 points/second
	extern VNADLL_API HopRate HOP_312;  //!< Hop rate of 312 points/second
	extern VNADLL_API HopRate HOP_156;  //!< Hop rate of 156 points/second
	extern VNADLL_API HopRate HOP_78;   //!< Hop rate of 78 points/second
	extern VNADLL_API HopRate HOP_39;   //!< Hop rate of 39 points/second
	extern VNADLL_API HopRate HOP_20;   //!< Hop rate of 20 points/second
	/** @}*/



	/** \addtogroup AttenuationSettings
	 *  @brief Attenuation options for the attenuation parameter.
	 *
	 *  @{
	 */

	/**
	 * Attenuation value type. Treat this as an opaque type.
	 */
	typedef int Attenuation;
	extern VNADLL_API Attenuation ATTEN_UNDEFINED;  //!< Attenuation value not set
	extern VNADLL_API Attenuation ATTEN_0;  //!< Attenuation value of 0 dB
	extern VNADLL_API Attenuation ATTEN_1;  //!< Attenuation value of 1 dB
	extern VNADLL_API Attenuation ATTEN_2;  //!< Attenuation value of 2 dB
	extern VNADLL_API Attenuation ATTEN_3;  //!< Attenuation value of 3 dB
	extern VNADLL_API Attenuation ATTEN_4;  //!< Attenuation value of 4 dB
	extern VNADLL_API Attenuation ATTEN_5;  //!< Attenuation value of 5 dB
	extern VNADLL_API Attenuation ATTEN_6;  //!< Attenuation value of 6 dB
	extern VNADLL_API Attenuation ATTEN_7;  //!< Attenuation value of 7 dB
	extern VNADLL_API Attenuation ATTEN_8;  //!< Attenuation value of 8 dB
	extern VNADLL_API Attenuation ATTEN_9;  //!< Attenuation value of 9 dB
	extern VNADLL_API Attenuation ATTEN_10; //!< Attenuation value of 10 dB
	extern VNADLL_API Attenuation ATTEN_11; //!< Attenuation value of 11 dB
	extern VNADLL_API Attenuation ATTEN_12; //!< Attenuation value of 12 dB
	extern VNADLL_API Attenuation ATTEN_13; //!< Attenuation value of 13 dB
	extern VNADLL_API Attenuation ATTEN_14; //!< Attenuation value of 14 dB
	extern VNADLL_API Attenuation ATTEN_15; //!< Attenuation value of 15 dB
	extern VNADLL_API Attenuation ATTEN_16; //!< Attenuation value of 16 dB
	extern VNADLL_API Attenuation ATTEN_17; //!< Attenuation value of 17 dB
	extern VNADLL_API Attenuation ATTEN_18; //!< Attenuation value of 18 dB
	extern VNADLL_API Attenuation ATTEN_19; //!< Attenuation value of 19 dB
	extern VNADLL_API Attenuation ATTEN_20; //!< Attenuation value of 20 dB
	extern VNADLL_API Attenuation ATTEN_21; //!< Attenuation value of 21 dB
	extern VNADLL_API Attenuation ATTEN_22; //!< Attenuation value of 22 dB
	extern VNADLL_API Attenuation ATTEN_23; //!< Attenuation value of 23 dB
	extern VNADLL_API Attenuation ATTEN_24; //!< Attenuation value of 24 dB
	extern VNADLL_API Attenuation ATTEN_25; //!< Attenuation value of 25 dB
	extern VNADLL_API Attenuation ATTEN_26; //!< Attenuation value of 26 dB
	extern VNADLL_API Attenuation ATTEN_27; //!< Attenuation value of 27 dB
	extern VNADLL_API Attenuation ATTEN_28; //!< Attenuation value of 28 dB
	extern VNADLL_API Attenuation ATTEN_29; //!< Attenuation value of 29 dB
	extern VNADLL_API Attenuation ATTEN_30; //!< Attenuation value of 30 dB
	extern VNADLL_API Attenuation ATTEN_31; //!< Attenuation value of 31 dB
	/** @}*/



	/** \addtogroup TaskState
	 *  @brief Available task states.
	 *
	 *  @{
	 */
	/**
	 * Attenuation value type. Treat this as an opaque type.
	 * see @ref state-overview for more details of what the different
	 * states mean.
	 */
	typedef int TaskState;
	extern VNADLL_API TaskState TASK_UNINITIALIZED; //!< Task state of uninitialized
	extern VNADLL_API TaskState TASK_STOPPED;       //!< Task state of stopped
	extern VNADLL_API TaskState TASK_STARTED;       //!< Task state of started
	/** @}*/


	/** \addtogroup RFPathSelector
	 *  @brief Available RF paths the hardware can support.
	 *
	 *  @{
	 */
	/**
	 * RF path selector type. Treat this as an opaque type.
	 */
	typedef int RFPath;

	extern VNADLL_API RFPath PATH_T1R1; //!< Out port T1, in port T1
	extern VNADLL_API RFPath PATH_T1R2; //!< Out port T1, in port T2
	extern VNADLL_API RFPath PATH_T2R1; //!< Out port T2, in port T1
	extern VNADLL_API RFPath PATH_T2R2; //!< Out port T2, in port T2
	extern VNADLL_API RFPath PATH_REF;  //!< Reference path

	/** @}*/

	/** \addtogroup SParameterSelector
	 *  @brief Available S-parameter measurements.
	 *
	 *
	 *  @{
	 */
	/**
	 * S-parameter path selector type. Treat this as an opaque type.
	 */
	typedef int SParameter;
	extern VNADLL_API SParameter PARAM_S11; //!< S11 Parameter
	extern VNADLL_API SParameter PARAM_S21; //!< S21 Parameter
	extern VNADLL_API SParameter PARAM_S12; //!< S12 Parameter
	extern VNADLL_API SParameter PARAM_S22; //!< S22 Parameter
	/** @}*/

	/** \addtogroup CalibrationStep
	 *  @brief Available calibration steps.
	 *
	 *  @{
	 */
	/**
	 * Calibration mode type. Treat this as an opaque type.
	 */
	typedef int CalibrationStep;
	extern VNADLL_API CalibrationStep STEP_P1_OPEN;  //!< Calibration step for measuring a open on port 1
	extern VNADLL_API CalibrationStep STEP_P1_SHORT; //!< Calibration step for measuring a short on port 1
	extern VNADLL_API CalibrationStep STEP_P1_LOAD;  //!< Calibration step for measuring a load on port 1
	extern VNADLL_API CalibrationStep STEP_P2_OPEN;  //!< Calibration step for measuring a open on port 2
	extern VNADLL_API CalibrationStep STEP_P2_SHORT; //!< Calibration step for measuring a short on port 2
	extern VNADLL_API CalibrationStep STEP_P2_LOAD;  //!< Calibration step for measuring a load on port 2
	extern VNADLL_API CalibrationStep STEP_THRU;     //!< Calibration step for measuring a through from port 1 - port 2
	/** @}*/

	/**
	 * HardwareDetails struct.
	 * This struct is used to encapsulate the capabilities of a specific VNA.
	 *
	 * Values for minimum and maximum frequencies, as well as the band boundaries
	 * are all in megahertz.
	 */
	typedef struct
	{
		/** Minimum frequency, in Mhz, that the VNA can measure. */
		int minimum_frequency;
		/** Maximum frequency, in Mhz, that the VNA can measure. */
		int maximum_frequency;
		/** Maximum number of points the VNA can sample in a single acquisition. */
		int maximum_points;
		/** Serial number of connected VNA */
		int serial_number;
		/**
		 * Band boundaries in the connected VNA. Highest frequency first, values in Mhz.
		 * Array size is specified in the `number_of_band_boundaries` struct member.
		 */
		int band_boundaries[8];
		/** Number of band boundaries in `band_boundaries` struct member. */
		int number_of_band_boundaries;
	} HardwareDetails;

	/**
	 * Container struct for passing IQ data sets around.
	 * The `I` and `Q` members are pointers to caller-allocated
	 * arrays of the requisite type (double, in this case).
	 */
	typedef struct
	{
		/** In-phase component value array */
		double* I;
		/** Quadrature component value array */
		double* Q;
	} ComplexData;

	/**
	 * @brief This is the method signature for the callback function
	 *        passed to the `initialize()` function.
	 *
	 *        The `initialize()` function takes a callback because it can
	 *        take >30 seconds to execute while it downloads the embedded
	 *        calibration from the VNA.
	 *
	 *        As such, the callback is called periodically while retreiving the
	 *        cal data so the process can be presented to the user.
	 *
	 * @param progressPercent Percentage of the download, from 0 - 100 %.
	 *                        Value is an integer.
	 *                        The callback may be called multiple times with the same
	 *                        progress percentage.
	 * @param user "user data". This is a void pointer that is passed in to the
	 *             `initialize()` function, and then simply passed through to
	 *             the callback every time it's called. It's intention is to
	 *             allow a reference to any relevant user-code to be made available
	 *             to the callback, so it could potentially update some external
	 *             state.
	 *             Set as NULL if unused.
	 *
	 * @return Continue value. If the callback returns false, the cal download will halt
	 *                  immediately and `initialize()` will return ERR_INTERRUPTED.
	 *                  If the callback returns true, the download will continue.
	 *
	 */
	typedef bool (*progress_callback)(int progressPercent, void* user); // return false to cancel


	/**
	 * @brief Returns a string describing the version of the DLL and its components
	 * @return Human-readable ASCII string pointer.
	 */
	VNADLL_API const char* versionString();

	/**
	 * @brief Creates a new Task object and returns a handle to it.
	 *        This handle is required by all of the other API
	 *        functions. The returned object is in the
	 *        TASK_UNINITIALIZED state.
	 * @return new task handle
	 */
	VNADLL_API TaskHandle createTask();


	//
	/**
	 * @brief Deletes the Task object. If the caller does not do this, the handle memory will leak.
	 *
	 * @param t Task to delete
	 */
	VNADLL_API void deleteTask(TaskHandle t);



	/**
	 * @brief Attempts to talk to the unit specified by the Task's IP address, and download
	 *        its details. If it succeeds the Task enters the TASK_STOPPED state.
	 *
	 *        This call can take a fair bit of time, up to 30 seconds <-> 1 minte.
	 *        For that purpose, a callback interface is provided
	 *
	 * @param t Handle for the current task
	 * @param callback User-provided callback function. Method signature must match
	 *                 \ref progress_callback. See \ref progress_callback for further
	 *                 details of the callback system.
	 *                 Set to NULL if no callback is required.
	 * @param user user-data provided to the callback function. See \ref progress_callback
	 *             for further description.
	 *             Not used if the \ref progress_callback param is NULL;
	 * @return Call status - Possible return values:
	 *          - ERR_OK if all went according to plan
	 *          - ERR_MISSING_IP if an IP has not been specified
	 *          - ERR_MISSING_PORT if a port has not been specified
	 *          - ERR_SOCKET if there was a problem setting up the UDP socket or sending a message
	 *          - ERR_NO_RESPONSE if the unit did not respond to commands
	 *          - ERR_BAD_PROM if the unit returned hardware details that this DLL doesn't understand
	 *          - ERR_WRONG_STATE if the Task is not in the TASK_UNINITIALIZED state
	 *          - ERR_INTERRUPTED if the initialization was cancelled from the callback.
	 */
	VNADLL_API ErrCode initialize(TaskHandle t, progress_callback callback = 0, void* user=0);


	/**
	 * @brief Attempts to program the VNA using the settings stored in the Task object. If it
	 *        succeeds the Task enters the TASK_STARTED state.
	 *
	 * @param t Handle for the current task
	 * @return Call status - Possible return values:
	 *
	 * @return Call status - Possible return values:
	 *          - ERR_OK if all went according to plan
	 *          - ERR_SOCKET if there was a problem sending a message
	 *          - ERR_NO_RESPONSE if the unit did not respond to commands
	 *          - ERR_WRONG_STATE if the Task is not in the TASK_STOPPED state
	 *          - ERR_MISSING_HOP if the hop rate has not yet been specified
	 *          - ERR_MISSING_ATTEN if the attenuation has not yet been specified
	 *          - ERR_MISSING_FREQS if the frequencies have not yet been specified
	 *          - ERR_PROG_OVERFLOW if the size of the program is too large for the hardware's memory
	 *                    (this can happen if the VNA is asked to sample too many frequency points)
	 */
	VNADLL_API ErrCode start(TaskHandle t);

	/**
	 * @brief Puts the Task object into the TASK_STOPPED state.
	 *
	 * @param t Handle for the current task
	 * @return Call status - Possible return values:
	 *           - ERR_OK if all went according to plan
	 *           - ERR_WRONG_STATE if the Task is not in the TASK_STARTED state
	 */
	VNADLL_API ErrCode stop(TaskHandle t);

	/**
	 * @brief Sets the IPv4 address on which to communicate with the unit.
	 *        The ipv4 parameter is copied into the Task's memory.
	 *        On success the Task's state will be TASK_UNINITIALIZED.
	 *        Example: setIPAddress(t, "192.168.1.197");
	 *
	 * @param t Handle for the current task
	 * @param ipv4 ASCII String IP representation, e.g. "192.168.1.207", etc...
	 *
	 * @return Call status - Possible return values:
	 *       - ERR_OK if all went according to plan
	 *       - ERR_MISSING_IP if the pointer is null
	 *       - ERR_WRONG_STATE if the Task is not in the TASK_UNINITIALIZED or TASK_STOPPED state
	 */
	VNADLL_API ErrCode setIPAddress(TaskHandle t, const char* ipv4);

	/**
	 * @brief Sets the port on which to communicate with the unit.
	 *        In general, ports >= 1024 should be used. On success
	 *        the Task's state will be TASK_UNINITIALIZED.
	 *
	 * @param t Handle for the current task
	 * @param port integer port number
	 *
	 * @return Call status - Possible return values:
	 *       - ERR_OK if all went according to plan
	 *       - ERR_WRONG_STATE if the Task is not in the TASK_UNINITIALIZED or TASK_STOPPED state
	 */
	VNADLL_API ErrCode setIPPort(TaskHandle t, const int port);

	//
	//
	/**
	 * @brief Sets the default time to wait, in milliseconds, for a unit to
	 *        reply to a command before giving up and returning an ERR_NO_RESPONSE
	 *        condition. For the measurement functions, this is the amount
	 *        of time to wait beyond the expected sweep time. When a Task is
	 *        created, the timeout value defaults to 150.
	 *
	 *   TODO: VALIDATE THIS - What happens if passed 0?
	 *
	 * @param t Handle for the current task
	 * @param timeout timeout in milliseconds
	 *
	 * @return Always returns ERR_OK.
	 */
	VNADLL_API ErrCode setTimeout(TaskHandle t, const unsigned int timeout);

	/**
	 * @brief Set the frequency hopping rate. See the values defined in the "HopRate" type above.
	 *
	 * @param t Handle for the current task
	 * @param rate HopRate as member of HopRate type.
	 *
	 * @return Call status - Possible return values:
	 *      - ERR_OK if all went according to plan
	 *      - ERR_BAD_HOP if there was something wrong with the hop rate parameter
	 *      - ERR_WRONG_STATE if the Task is not in the TASK_UNINITIALIZED or TASK_STOPPED state
	 */
	VNADLL_API ErrCode setHopRate(TaskHandle t, const HopRate rate);

	/**
	 * @brief Set the attenuation amount. See the values defined above.
	 *
	 * @param t Handle for the current task
	 * @param atten One of the `Attenuation` types defined in the DLL
	 *
	 * @return Call status - Possible return values:
	 *        - ERR_OK if all went according to plan
	 *        - ERR_BAD_ATTEN if there was something wrong with the attenuation parameter
	 *        - ERR_WRONG_STATE if the Task is not in the TASK_UNINITIALIZED or TASK_STOPPED state
	 */
	VNADLL_API ErrCode setAttenuation(TaskHandle t, const Attenuation atten);


	/**
	 * @brief Set the frequencies to measure during each sweep. Units are MHz. The freqs
	 *        parameter is an array of length N. Note that the VNA frequency generation
	 *        hardware has fixed precision and so the generated frequency may not be exactly
	 *        equal to the requested frequency. This function silently converts all requested
	 *        frequencies to frequencies that can be exactly generated by the hardware.
	 *        This has important implications for doppler noise when doing a linear sweep.
	 *        AKELA recommends using the function utilFixLinearSweepLimits() to ensure
	 *        every frequency is exactly generateable and that the frequencies are equally
	 *        spaced. Use the getFrequencies() function to get the actual frequencies being
	 *        generated.
	 *
	 * @param t Handle for the current task
	 * @param freqs array of frequencies to sample (each frequency is a single double)
	 * @param N Length of `freqs` array.
	 * @return Call status - Possible return values:
	 *       - ERR_OK if all went according to plan
	 *       - ERR_WRONG_STATE if the Task is not in the TASK_STOPPED state
	 *       - ERR_FREQ_OUT_OF_BOUNDS if a frequency is beyond the allowed min/max. (You can get
	 *         the min and max from the HardwareDetails struct returned by getHardwareDetails())
	 *       - ERR_TOO_MANY_POINTS if N is larger than the maximum allowed (see HardwareDetails)
	 */
	VNADLL_API ErrCode setFrequencies(TaskHandle t, const double* freqs, const unsigned int N);

	//
	/**
	 * @brief Get the current state of the Task object. Returns one of the values defined above.
	 *
	 * @param t Handle for the current task
	 * @return A value from the TaskState state list
	 */
	VNADLL_API TaskState getState(TaskHandle t);

	//
	/**
	 * @brief Get the current time to wait for the unit to reply to commands. When a Task is first created, this will default to 150.
	 *
	 * @param t Handle for the current task
	 * @return current timeout in milliseconds
	 */
	VNADLL_API unsigned int getTimeout(TaskHandle t);

	/**
	 * @brief Get the current AVMU IP address for the Task object.
	 *        When no IP has yet been set, this will return a NULL
	 *        char*.
	 *
	 * @param t Handle for the current task
	 * @return char* to string containing the AVMU IP address. NULL if not set.
	 */
	VNADLL_API const char* getIPAddress(TaskHandle t);

	// Get the port associated with this Task object. If no port has yet been set, this
	// function returns 0.
	/**
	 * @brief Get the current port for IP communications.
	 *        When uninitialized, this will default to 0
	 *
	 * @param t Handle for the current task
	 * @return current port number
	 */
	VNADLL_API int getIPPort(TaskHandle t);

	//
	/**
	 * @brief Get the frequency hopping rate associated with this Task object.
	 *        If no rate has yet been set, this function returns HOP_UNDEFINED.
	 *
	 * @param t Handle for the current task
	 * @return HopRate for the Task `t` in question.
	 */
	VNADLL_API HopRate getHopRate(TaskHandle t);

	//
	/**
	 * @brief Get the attenuation associated with this Task object. If no rate has yet been set,  this function returns ATTEN_UNDEFINED.
	 * @details [long description]
	 *
	 * @param t Handle for the current task
	 * @return Attenuation for Task `t`.
	 */
	VNADLL_API Attenuation getAttenuation(TaskHandle t);

	/**
	 * @brief Get the number of frequency points for the sweep configured for
	 *        Task `t`. If no frequencies have been set, this function returns 0.
	 *
	 * @param t Handle for the current task
	 * @return Number of frequency points in Task `t`.
	 */
	VNADLL_API unsigned int getNumberOfFrequencies(TaskHandle t);

	// Get the list of actual frequencies the hardware generates during the sweep. If no
	// frequencies have been set, this function returns 0.
	/**
	 * @brief Get a list containing the actual frequencies the hardware will sample for
	 *        the configured sweep in task `t`.
	 *
	 *        The actual frequency points can differ from the requested frequency points
	 *        because the hardware has fixed precision, and cannot achieve every arbitrary
	 *        frequency value within it's tunable bands. The values in this list are the
	 *        requested frequency points after snapping them to the closest achievable
	 *        frequency.
	 *
	 * TODO: What units are these in?
	 *
	 * @param t Handle for the current task
	 * @return array of double frequency values. The array length
	 *               can be retreived by calling getNumberOfFrequencies()
	 */
	VNADLL_API const double* getFrequencies(TaskHandle t);

	/**
	 * @brief Get the hardware details for the unit associated with
	 *        Task `t`. If the Task has not yet been initialized, the
	 *        returned struct has all values set to 0.
	 *
	 * @param t Handle for the current task
	 * @return `HardwareDetails` containing the details of the connected
	 *                           hardware (if initialized), else zeroes.
	 */
	VNADLL_API HardwareDetails getHardwareDetails(TaskHandle t);

	//
	// Possible return values:
	// the min and max from the HardwareDetails struct returned by getHardwareDetails())
	/**
	 * @brief Adjusts a requested frequency, in MHz, to the nearest able to be generated by the VNA
	 *        hardware. This is not available in the TASK_UNINITIALIZED state.
	 *
	 * @param t Handle for the current task
	 * @param freq Target frequency value pointer, which will be modified to the nearest
	 *             frequency the hardware can achieve.
	 *
	 * @return Call status - Possible return values:
	 *            - ERR_OK if all went according to plan
	 *            - ERR_WRONG_STATE if the Task is in the TASK_UNINITIALIZED state
	 *            - ERR_FREQ_OUT_OF_BOUNDS if the frequency is beyond the allowed min/max. (You can get
	 */
	VNADLL_API ErrCode utilNearestLegalFreq(TaskHandle t, double& freq);

	//
	//
	// Possible return values:
	/**
	 * @brief Adjusts the start and end of a requested linear sweep with N points such
	 *        that all frequencies in the sweep will land on exactly generateable values.
	 *        This is important so that the frequency spacing between all points is
	 *        identical. Unequal spacing can cause doppler noise in your data.
	 *
	 *        If the input frequencies are equal, or N is 0 or 1, the frequencies are each adjusted to exactly generateable values.
	 *
	 * TODO: More units to validate!
	 *
	 * @param t Handle for the current task
	 * @param startFreq Start frequency (in Mhz)
	 * @param endFreq Stop freqency (in Mhz)
	 * @param N Number of frequencies
	 * @return Call status - Possible return values:
	 *             - ERR_OK if all went according to plan
	 *             - ERR_WRONG_STATE if the Task is in the TASK_UNINITIALIZED state
	 *             - ERR_FREQ_OUT_OF_BOUNDS if one of the bounds is beyond the allowed min/max. (You can get
	 *             - the min and max from the HardwareDetails struct returned by getHardwareDetails())
	 *             - ERR_TOO_MANY_POINTS if N is larger than the maximum allowed (see HardwareDetails)
	 */
	VNADLL_API ErrCode utilFixLinearSweepLimits(TaskHandle t, double& startFreq, double& endFreq, const unsigned int N);



	// Possible return values:
	/**
	 * @brief Sends an "are you there" message to the unit. Note that this function should not be
	 *        called while a frequency sweep is ongoing, because it causes that sweep to prematurely
	 *        halt and respond to this message instead. This is only an issue in multithreaded code,
	 *        since the data acquisition functions are blocking. This function waits for a reply for
	 *        the length of time specified by getTimeout() before giving up.
	 *
	 *        Note that this can be called from any state, provided an IP and port are present.
	 *
	 * @param t Handle for the current task
	 * @return Call status - Possible return values:
	 -        - ERR_OK if all went according to plan
	 -        - ERR_SOCKET if there was a problem sending a message
	 -        - ERR_NO_RESPONSE if the unit did not respond to commands
	 -        - ERR_MISSING_IP if no IP address has been set
	 -        - ERR_MISSING_PORT if no port has been set
	 */
	VNADLL_API ErrCode utilPingUnit(TaskHandle t);

	/**
	 * @brief Generates a linear sweep with the requested parameters. Note that the start and end
	 *        frequency will be adjusted as documented in utilFixLinearSweepLimits() so that all
	 *        frequency points fall on exactly generateable values. This function internally calls
	 *        setFrequencies() with the resulting array. The caller can retrieve the frequency list
	 *        with the getFrequencies() function. Since it changes the frequencies this function
	 *        is only available in the TASK_STOPPED state.
	 *
	 *        If `startFreq` == `endFreq`, the hardware will effectively be placed in zero-span
	 *        mode, as it will repeatedly sample the same frequency for the duration of the
	 *        sweep. This is a valid operating mode.
	 *
	 * @param t Handle for the current task
	 * @param startFreq Start frequency of sweep in Mhz
	 * @param endFreq End frequency of sweep in Mhz
	 * @param N Number of points to sample.
	 * @return Call status - Possible return values:
	 *        - ERR_OK if all went according to plan
	 *        - ERR_WRONG_STATE if the Task is not in the TASK_STOPPED state
	 *        - ERR_FREQ_OUT_OF_BOUNDS if one of the bounds is beyond the allowed min/max. (You can get
	 *          the min and max from the HardwareDetails struct returned by getHardwareDetails())
	 *        - ERR_TOO_MANY_POINTS if N is larger than the maximum allowed (see HardwareDetails)
	 */
	VNADLL_API ErrCode utilGenerateLinearSweep(TaskHandle t, const double startFreq, const double endFreq, const unsigned int N);

	/**
	 * @brief Measures the path(s) of your choice through the VNA, without applying calibration.
	 *       The reference path is always measured.
	 *       The ComplexData parameters are objects that contain pointers to data arrays that
	 *       the caller has allocated. This function copies the returned data into those arrays,
	 *       so they must have a minimum length of getNumberOfFrequencies().
	 *       Note that this function blocks while the measurement is being performed. Use the
	 *       interruptMeasurement() function to prematurely halt a slow measurement. The automatic
	 *       timeout value is the length of the measurement plus getTimeout().
	 *       Note that it is safe to supply null pointers.
	 *
	 * @param t Handle for the current task
	 * @param paths Which paths to sample. Logical OR of the \ref RFPathSelector values desired.
	 * @param T1R1 Transmit port 1, receive port 1 reading.
	 * @param T1R2 Transmit port 1, receive port 2 reading.
	 * @param T2R1 Transmit port 2, receive port 1 reading.
	 * @param T2R2 Transmit port 2, receive port 2 reading.
	 * @param Ref Reference reading
	 * @return Call status - Possible return values:
	 *        - ERR_OK if all went according to plan
	 *        - ERR_SOCKET if there was a problem sending a message
	 *        - ERR_NO_RESPONSE if the unit did not respond to commands
	 *        - ERR_WRONG_STATE if the Task is not in the TASK_STARTED state
	 *        - ERR_BAD_PATH if there is something wrong with the paths parameter
	 *        - ERR_BYTES if the wrong number of bytes were received
	 *        - ERR_INTERRUPTED if the measurement was interrupted
	 */
	VNADLL_API ErrCode measureUncalibrated(TaskHandle t, RFPath paths, ComplexData T1R1, ComplexData T1R2,
	                                       ComplexData T2R1, ComplexData T2R2,
	                                       ComplexData Ref);

	/**
	 * @brief Measures the S-parameter(s) of your choice, applying the current calibration.
	 *         Only paths specified by the 'paths' parameter get data copied into the output
	 *         ComplexData structures. To select more than one path, bitwise-OR the constants
	 *         together, e.g. `PARAM_S11 | PARAM_S22`
	 *         Like the other measurement functions, the ComplexData structures contain pointers
	 *         to memory that you the caller have allocated.
	 *         Note that this function blocks while the measurement is being performed. Use the
	 *         interruptMeasurement() function to prematurely halt a slow measurement.
	 *         You must pass buffers for all parameters, even if you don't want to measure them.
	 *         Possible return values:
	 *
	 * @param t Handle for the current task
	 * @param paths Logical OR of the S-parameter paths to measure.
	 * @param S11 User-Allocated \ref ComplexData() instance.
	 * @param S21 User-Allocated \ref ComplexData() instance.
	 * @param S12 User-Allocated \ref ComplexData() instance.
	 * @param S22 User-Allocated \ref ComplexData() instance.
	 * @return Call status - Possible return values:
	 *       - ERR_OK if all went according to plan
	 *       - ERR_SOCKET if there was a problem sending a message
	 *       - ERR_NO_RESPONSE if the unit did not respond to commands
	 *       - ERR_WRONG_STATE if the Task is not in the TASK_STARTED state
	 *       - ERR_BAD_PATH if there is something wrong with the paths parameter
	 *       - ERR_BAD_CAL if the calibration is missing (i.e. `isCalibrationComplete() == false`)
	 *       - ERR_BYTES if the wrong number of bytes were received
	 *       - ERR_INTERRUPTED if the measurement was interrupted
	 */
	VNADLL_API ErrCode measure2PortCalibrated(TaskHandle t, SParameter paths,
	        ComplexData S11, ComplexData S21,
	        ComplexData S12, ComplexData S22);

	/**
	 * @brief Measures the paths necessary to get data for the requested calibration step.
	 *         Note that this function blocks while the measurement is being performed. Use the
	 *         interruptMeasurement() function to prematurely halt a slow measurement.
	 *         Possible return values:
	 *
	 * @param t Handle for the current task
	 * @param step Which step of the calibration process you want to run. One of
	 *             \ref CalibrationStep options.
	 *
	 * @return Call status - Possible return values:
	 *       - ERR_OK if all went according to plan
	 *       - ERR_SOCKET if there was a problem sending a message
	 *       - ERR_NO_RESPONSE if the unit did not respond to commands
	 *       - ERR_WRONG_STATE if the Task is not in the TASK_STARTED state
	 *       - ERR_BAD_CAL if the current calibration settings do not match the current
	 *         sweep settings (clear the calibration first before recalibrating)
	 *       - ERR_BYTES if the wrong number of bytes were received
	 *       - ERR_INTERRUPTED if the measurement was interrupted
	 */
	VNADLL_API ErrCode measureCalibrationStep(TaskHandle t, CalibrationStep step);

	// These functions let you know the status of the internal raw data arrays
	// for each calibration step

	/** Return whether internal cal array for the Port-1 open measurement is present  */
	VNADLL_API bool haveCalP1Open(TaskHandle t);
	/** Return whether internal cal array for the Port-1 shorted measurement is present  */
	VNADLL_API bool haveCalP1Short(TaskHandle t);
	/** Return whether internal cal array for the Port-1 load measurement is present  */
	VNADLL_API bool haveCalP1Load(TaskHandle t);
	/** Return whether internal cal array for the Port-2 open measurement is present  */
	VNADLL_API bool haveCalP2Open(TaskHandle t);
	/** Return whether internal cal array for the Port-2 shorted measurement is present  */
	VNADLL_API bool haveCalP2Short(TaskHandle t);
	/** Return whether internal cal array for the Port-2 load measurement is present  */
	VNADLL_API bool haveCalP2Load(TaskHandle t);
		/** Return whether internal cal array for the through-cal measurement is present  */
	VNADLL_API bool haveCalThru(TaskHandle t);

	/**
	 * @brief Interrupts one of the measurement functions while it is waiting for
	 *        data. Since the measurement functions are blocking, this function
	 *        must be called from a different thread. This function returns
	 *        immediately, however the measurement function may continue to block
	 *        for a short additional amount of time.
	 *
	 * @param t Handle for the current task
	 * @return Call status - Possible return values:
	 *      - ERR_OK if all went according to plan
	 *      - ERR_WRONG_STATE if the Task is not in the TASK_STARTED state
	 */
	VNADLL_API ErrCode interruptMeasurement(TaskHandle t);

	/**
	 * @brief Delete any calibration data stored in the Task `t`.
	 *
	 * @param t Handle for the current task
	 * @return Always returns `ERR_OK`
	 */
	VNADLL_API ErrCode clearCalibration(TaskHandle t);

	/**
	 * @brief Query if calibration parameters are present in the task.
	 *
	 * @param t Handle for the current task
	 * @return true if the task has cal parameters, false otherwise.
	 */
	VNADLL_API bool isCalibrationComplete(TaskHandle t);

	// Get the number of frequencies in the calibration data
	/**
	 * @brief Get the number of frequencies in the calibration data
	 *
	 * @param t Handle for the current task
	 * @return number of frequencies in calibration data.
	 */
	VNADLL_API unsigned int getCalibrationNumberOfFrequencies(TaskHandle t);

	/**
	 * @brief Get a pointer to the list of frequencies used in the calibration data.
	 *        This pointer becomes invalid after clearCalibration() is called.
	 *        When no calibration data is loaded, this returns 0.
	 *        Calibration data must be loaded via a measureCalibrationStep() call.
	 *
	 * @param t Handle for the current task
	 * @return Pointer to array of double containing calibration frequencies.
	 *         array size can be determined by calling `getCalibrationNumberOfFrequencies()`
	 */
	VNADLL_API const double* getCalibrationFrequencies(TaskHandle t);

	//
	//

	/**
	 * @brief Copies out the calibration coefficients to caller-allocated arrays. Note that
	 *          to fully contain a calibration the caller must also get the frequency list
	 *          and number of frequencies. It is the caller's responsibility to ensure the
	 *          arrays have enough memory allocated to hold all of the data. It is
	 *          recommended to use the getCalibrationNumberOfFrequencies() function to get the
	 *          length because the number of calibration frequencies does not have to match
	 *          the number of sweep frequencies.
	 *
	 *       The calibration terms are for a full, 12-term model.
	 *
	 * @param t Handle for the current task
	 * @param e00 EDF Calibration term.
	 * @param e11 ESF Calibration term.
	 * @param e10e01 ERF Calibration term.
	 * @param e30 EXF Calibration term.
	 * @param e22 ELF Calibration term.
	 * @param e10e32 ETF Calibration term.
	 * @param ep33 EDR Calibration term.
	 * @param ep22 ESR Calibration term.
	 * @param ep23ep32 ERR Calibration term.
	 * @param ep03 EXR Calibration term.
	 * @param ep11 ELR Calibration term.
	 * @param ep23ep01 ETR Calibration term.
	 *
	 * @return Call status - Possible return values:
	 *         - ERR_OK if all went according to plan
	 *         - ERR_BAD_CAL if isCalibrationComplete() returns false
	 */
	VNADLL_API ErrCode exportCalibration(TaskHandle t,
	                                     ComplexData e00,         /* EDF */
	                                     ComplexData e11,         /* ESF */
	                                     ComplexData e10e01,      /* ERF */
	                                     ComplexData e30,         /* EXF */
	                                     ComplexData e22,         /* ELF */
	                                     ComplexData e10e32,      /* ETF */
	                                     ComplexData ep33,        /* EDR */
	                                     ComplexData ep22,        /* ESR */
	                                     ComplexData ep23ep32,    /* ERR */
	                                     ComplexData ep03,        /* EXR */
	                                     ComplexData ep11,        /* ELR */
	                                     ComplexData ep23ep01     /* ETR */);


	/**
	 * @brief Imports calibration coefficients from caller-allocated arrays. Note that these
	 *        frequencies do not have to be exactly generateable by the hardware. At import
	 *        an interpolated calibration is generated that matches the current sweep settings.
	 *        Altering sweep settings automatically generates a new interpolated calibration
	 *        from the original imported data provided here. Data is copied out of these
	 *        arrays into the Tasks's memory.
	 *
	 *       The calibration terms are for a full, 12-term model.
	 *
	 * @param t Handle for the current task
	 * @param freqs Double[] of frequencies for the calibration data.
	 * @param N Size of `freqs` value
	 * @param e00 EDF calibration term.
	 * @param e11 ESF calibration term.
	 * @param e10e01 ERF calibration term.
	 * @param e30 EXF calibration term.
	 * @param e22 ELF calibration term.
	 * @param e10e32 ETF calibration term.
	 * @param ep33 EDR calibration term.
	 * @param ep22 ESR calibration term.
	 * @param ep23ep32 ERR calibration term.
	 * @param ep03 EXR calibration term.
	 * @param ep11 ELR calibration term.
	 * @param ep23ep01 ETR calibration term.
	 *
	 * @return Call status - Possible return values:
	 *         - ERR_OK if all went according to plan
	 *         - ERR_BAD_CAL if any of the array pointers are null
	 *         - ERR_WRONG_STATE if the Task is not in the TASK_STOPPED or TASK_STARTED state
	 */
	VNADLL_API ErrCode importCalibration(TaskHandle t,
	                                     const double* freqs,
	                                     const unsigned int N,
	                                     const ComplexData e00,		/* EDF */
	                                     const ComplexData e11,		/* ESF */
	                                     const ComplexData e10e01,	/* ERF */
	                                     const ComplexData e30,		/* EXF */
	                                     const ComplexData e22,		/* ELF */
	                                     const ComplexData e10e32,	/* ETF */
	                                     const ComplexData ep33,		/* EDR */
	                                     const ComplexData ep22,		/* ESR */
	                                     const ComplexData ep23ep32,	/* ERR */
	                                     const ComplexData ep03,		/* EXR */
	                                     const ComplexData ep11,		/* ELR */
	                                     const ComplexData ep23ep01	/* ETR */);

	/**
	 * @brief Determine of the VNA has a factory calibration stored in it's
	 *        onboard EEPROM.
	 *
	 * @param t Handle for the current task
	 * @return True if the EEPROM has a factory calibration, False if not.
	 */
	VNADLL_API bool hasFactoryCalibration(TaskHandle t);

	/**
	 * @brief Load the factory calibration from the VNA into the current task.
	 *
	 * @param t Handle for the current task
	 * @return Call status - Possible return values:
	 *          - ERR_BAD_CAL if the unit doesn't have a factory calibration
	 *          - ERR_WRONG_STATE if the task is not in the TASK_STOPPED or TASK_STARTED state
	 */
	VNADLL_API ErrCode importFactoryCalibration(TaskHandle t);

	/**
	 * @brief  Set phase correction factors for the open used during calibration.
	 *         Each time calibration parameters are computed from raw data, these phases
	 *         are added to the phase of the 'open' data. The operation is not cumulative,
	 *         meaning if you first set a value of 6, and then 7, the final corrected phase
	 *         is +7, not +13.
	 *         Because there is one phase for each frequency point, this call only succeeds
	 *         when the task is in the TASK_STOPPED or TASK_STARTED state.
	 *         The data you provide here is internally copied, so the passed pointer need
	 *         not remain valid after the call. Null may be passed as a shortcut to setting
	 *         the offset to zero for all frequencies.
	 *
	 * @param t Handle for the current task
	 * @param phaseCorrections Phase correction value in radians.
	 *
	 * @return Call status - Possible return values:
	 *          - ERR_OK
	 *          - ERR_BAD_CAL if `!(haveCalP1Open() || haveCalP2Open())`
	 *          - ERR_WRONG_STATE if the Task is not in the TASK_STOPPED or TASK_STARTED state
	 */
	VNADLL_API ErrCode setOpenPhaseCorrection(TaskHandle t, const double* const phaseCorrections);

}

/** @}*/

#endif