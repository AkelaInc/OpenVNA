// vnadll_test.cpp : Demonstration of the AKELA VNA API
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vna_header_agg_c.h"

#define TEST_TARGET_IP_VNA "192.168.1.193"

void logCodeAndQuitIfError(ErrCode code)
{

	if (code == ERR_OK)                    std::cout << "ERR_OK"                     << std::endl;
	if (code == ERR_BAD_ATTEN)             std::cout << "ERR_BAD_ATTEN"              << std::endl;
	if (code == ERR_BAD_CAL)               std::cout << "ERR_BAD_CAL"                << std::endl;
	if (code == ERR_BAD_HANDLE)            std::cout << "ERR_BAD_HANDLE"             << std::endl;
	if (code == ERR_BAD_HOP)               std::cout << "ERR_BAD_HOP"                << std::endl;
	if (code == ERR_BAD_PATH)              std::cout << "ERR_BAD_PATH"               << std::endl;
	if (code == ERR_BAD_PROM)              std::cout << "ERR_BAD_PROM"               << std::endl;
	if (code == ERR_BYTES)                 std::cout << "ERR_BYTES"                  << std::endl;
	if (code == ERR_FREQ_OUT_OF_BOUNDS)    std::cout << "ERR_FREQ_OUT_OF_BOUNDS"     << std::endl;
	if (code == ERR_INTERRUPTED)           std::cout << "ERR_INTERRUPTED"            << std::endl;
	if (code == ERR_NO_RESPONSE)           std::cout << "ERR_NO_RESPONSE"            << std::endl;
	if (code == ERR_MISSING_IP)            std::cout << "ERR_MISSING_IP"             << std::endl;
	if (code == ERR_MISSING_PORT)          std::cout << "ERR_MISSING_PORT"           << std::endl;
	if (code == ERR_MISSING_HOP)           std::cout << "ERR_MISSING_HOP"            << std::endl;
	if (code == ERR_MISSING_ATTEN)         std::cout << "ERR_MISSING_ATTEN"          << std::endl;
	if (code == ERR_MISSING_FREQS)         std::cout << "ERR_MISSING_FREQS"          << std::endl;
	if (code == ERR_PROG_OVERFLOW)         std::cout << "ERR_PROG_OVERFLOW"          << std::endl;
	if (code == ERR_SOCKET)                std::cout << "ERR_SOCKET"                 << std::endl;
	if (code == ERR_TOO_MANY_POINTS)       std::cout << "ERR_TOO_MANY_POINTS"        << std::endl;
	if (code == ERR_WRONG_STATE)           std::cout << "ERR_WRONG_STATE"            << std::endl;
	if (code == ERR_EMPTY_PROM)            std::cout << "ERR_EMPTY_PROM"             << std::endl;
	if (code == ERR_PATH_ALREADY_MEASURED) std::cout << "ERR_PATH_ALREADY_MEASURED"  << std::endl;
	if (code == ERR_NO_PATHS_MEASURED)     std::cout << "ERR_NO_PATHS_MEASURED"      << std::endl;
	if (code == ERR_NO_ATTEN_PRESENT)      std::cout << "ERR_NO_ATTEN_PRESENT"       << std::endl;
	if (code == ERR_WRONG_PROGRAM_TYPE)    std::cout << "ERR_WRONG_PROGRAM_TYPE"     << std::endl;
	if (code == ERR_FEATURE_NOT_PRESENT)   std::cout << "ERR_FEATURE_NOT_PRESENT"    << std::endl;
	if (code == ERR_UNKNOWN_FEATURE)       std::cout << "ERR_UNKNOWN_FEATURE"        << std::endl;
	if (code == ERR_BAD_PORT)              std::cout << "ERR_BAD_PORT"               << std::endl;


	if(code != ERR_OK)
	{
		std::cout << "Error!" << std::endl;

		if (system("pause"))
			;

		exit(-1);
	}
}


bool download_callback(int progressPercent, void* user)
{
	printf("%d%% complete (task %p)\n", progressPercent, user);
	return true;
}

int main(int argc, char* argv[])
{
	ErrCode code;
	TaskHandle task;

	printf("%s\n", versionString());

	printf("\nCreating task\n");
	task = createTask();

	printf("\nSetting ip to " TEST_TARGET_IP_VNA ", port to 1025\n");
	code = setIPAddress(task, TEST_TARGET_IP_VNA);
	logCodeAndQuitIfError(code);
	code = setIPPort(task, 1025);
	logCodeAndQuitIfError(code);

	printf("\nSetting default timeout to 150 ms\n");
	code = setTimeout(task, 150);
	logCodeAndQuitIfError(code);

	const int tries = 10;

	for (int x = 0; x < tries; x += 1)
	{
		printf("\nChecking if unit is responding\n");
		code = utilPingUnit(task);
		if (x >(tries - 1))
			logCodeAndQuitIfError(code);

		printf("\nInitializing task (could take several seconds)\n");
		code = initialize(task, download_callback, (void*)task);
		if (code == ERR_OK)
			break;
		if (x > (tries - 1))
			logCodeAndQuitIfError(code);
	}

	printf("\nUnit properties:\n");
	HardwareDetails details = getHardwareDetails(task);
	printf("Serial number = %d\n", details.serial_number);
	printf("Min freq (MHz) = %d\n", details.minimum_frequency);
	printf("Max freq (MHz) = %d\n", details.maximum_frequency);
	printf("Max number of points = %d\n", details.maximum_points);
	printf("Has embedded cal data = %s\n", hasFactoryCalibration(task) ? "yes" : "no");

	printf("\nSetting stimulus parameters\n");
	code = setHopRate(task, HOP_45K);
	logCodeAndQuitIfError(code);
	code = setAttenuation(task, ATTEN_0);
	logCodeAndQuitIfError(code);


	//code = utilGenerateLinearSweep(task, details.minimum_frequency, details.maximum_frequency, 517);
	code = utilGenerateLinearSweep(task, 1500, 1500, 512);
	logCodeAndQuitIfError(code);

	printf("\nStarting task\n");
	code = start(task);
	logCodeAndQuitIfError(code);
	{
		ComplexData t1r1;
		t1r1.I = new double[getNumberOfFrequencies(task)];
		t1r1.Q = new double[getNumberOfFrequencies(task)];
		ComplexData dontcare = { NULL, NULL };

		printf("\nMeasuring 512 points\n");
		code = measureUncalibrated(task, t1r1, dontcare, dontcare, dontcare, dontcare);
		logCodeAndQuitIfError(code);

		printf("\nStopping the task\n");
		code = stop(task);
		logCodeAndQuitIfError(code);

		code = utilGenerateLinearSweep(task, 1500, 1500, 513);
		logCodeAndQuitIfError(code);

		printf("\nStarting task\n");
		code = start(task);
		logCodeAndQuitIfError(code);


		delete[] t1r1.I;
		delete[] t1r1.Q;

		t1r1.I = new double[getNumberOfFrequencies(task)];
		t1r1.Q = new double[getNumberOfFrequencies(task)];

		printf("\nMeasuring 513 points\n");
		code = measureUncalibrated(task, t1r1, dontcare, dontcare, dontcare, dontcare);
		logCodeAndQuitIfError(code);


		delete[] t1r1.I;
		delete[] t1r1.Q;
	}
	printf("\nStopping the task\n");
	code = stop(task);
	logCodeAndQuitIfError(code);

	code = utilGenerateLinearSweep(task, 1500, 1500, 512);
	logCodeAndQuitIfError(code);

	code = setHopRate(task, HOP_45K);
	logCodeAndQuitIfError(code);

	printf("\nStarting task\n");
	code = start(task);
	logCodeAndQuitIfError(code);


	printf("\nMeasuring 512 points\n");

	{

		ComplexData t1r1, t1r2, t2r1, t2r2, ref;
		t1r1.I = new double[getNumberOfFrequencies(task)];
		t1r1.Q = new double[getNumberOfFrequencies(task)];

		t1r2.I = new double[getNumberOfFrequencies(task)];
		t1r2.Q = new double[getNumberOfFrequencies(task)];

		t2r1.I = new double[getNumberOfFrequencies(task)];
		t2r1.Q = new double[getNumberOfFrequencies(task)];

		t2r2.I = new double[getNumberOfFrequencies(task)];
		t2r2.Q = new double[getNumberOfFrequencies(task)];

		ref.I = new double[getNumberOfFrequencies(task)];
		ref.Q = new double[getNumberOfFrequencies(task)];
		for (int x = 0; x < 2; x += 1)
		{

			printf("\nStopping the task\n");
			code = stop(task);
			logCodeAndQuitIfError(code);


			printf("\nStarting task\n");
			code = start(task);
			logCodeAndQuitIfError(code);
			code = measureUncalibrated(task, t1r1, t1r2, t2r1, t2r2, ref);
		}
		delete[] t1r1.I;
		delete[] t1r1.Q;
		delete[] t1r2.I;
		delete[] t1r2.Q;
		delete[] t2r1.I;
		delete[] t2r1.Q;
		delete[] t2r2.I;
		delete[] t2r2.Q;
		delete[] ref.I ;
		delete[] ref.Q ;

	}

	printf("\nStopping the task\n");
	code = stop(task);
	logCodeAndQuitIfError(code);

	printf("\nActual frequencies to be measured: (first 5 for brevity)\n");

	{
		int npts = getNumberOfFrequencies(task);
		double* f = new double[npts];
		code = getFrequencies(task, f, npts);
		logCodeAndQuitIfError(code);
		for (unsigned int i = 0; i < 5; ++i)
			printf("%f\n", f[i]);

		code = setFrequencies(task, f, npts);
		logCodeAndQuitIfError(code);

		delete[] f;

	}

	printf("\nStarting task\n");
	code = start(task);
	logCodeAndQuitIfError(code);



	if(isCalibrationComplete(task))
	{
		printf("\nUnit has embedded calibration data, let's try using it\n");
		ComplexData S11, S21;
		ComplexData dontcare;
		dontcare.I = 0;
		dontcare.Q = 0;
		S11.I = new double[getNumberOfFrequencies(task)];
		S11.Q = new double[getNumberOfFrequencies(task)];
		S21.I = new double[getNumberOfFrequencies(task)];
		S21.Q = new double[getNumberOfFrequencies(task)];
		code = measure2PortCalibrated(task, S11, S21, dontcare, dontcare);
		logCodeAndQuitIfError(code);
		printf("S11.I\tS11.Q\t\tS21.I\tS21.Q\n");
		for(unsigned int i=0; i<5; ++i)
			printf("%.2f\t%.2f\t\t%.2f\t%.2f\n", S11.I[i], S11.Q[i], S21.I[i], S21.Q[i]);

		delete [] S11.I;
		delete [] S11.Q;
		delete [] S21.I;
		delete [] S21.Q;
	}

	{
		printf("\nMeasuring some uncalibrated data\n");
		ComplexData t1r1;
		t1r1.I = new double[getNumberOfFrequencies(task)];
		t1r1.Q = new double[getNumberOfFrequencies(task)];
		ComplexData dontcare = { NULL, NULL };
		code = measureUncalibrated(task, t1r1, dontcare, dontcare, dontcare, dontcare);
		logCodeAndQuitIfError(code);



		printf("Data: (first 5 points for brevity)\n");
		for (unsigned int i = 0; i < 5; ++i)
			printf("%f\t%f\n", t1r1.I[i], t1r1.Q[i]);

		//FILE* datafile = fopen("data.csv","w");
		//if(datafile)
		//{
		//	fprintf(datafile, "Frequency,I,Q,LogMag\n");
		//	for(unsigned int i=0; i<getNumberOfFrequencies(task); ++i)
		//		fprintf(datafile, "%f,%f,%f,%f\n", f[i], t1r1.I[i], t1r1.Q[i],   20*log10(sqrt(t1r1.I[i]*t1r1.I[i] + t1r1.Q[i]*t1r1.Q[i])));
		//	fclose(datafile);
		//}

		delete[] t1r1.I;
		delete[] t1r1.Q;
	}

	printf("\nExercising calibration functions\n");
	code = clearCalibration(task);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P1_OPEN);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P1_SHORT);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P1_LOAD);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P2_OPEN);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P2_SHORT);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_P2_LOAD);
	logCodeAndQuitIfError(code);
	code = measureCalibrationStep(task, STEP_THRU);
	logCodeAndQuitIfError(code);

	if(isCalibrationComplete(task))
		printf("Done with calibration\n");

	printf("\nMeasuring some calibrated data\n");
	for (int x = 0; x < 10; x += 1)
	{
		ComplexData S11, S21;
		ComplexData dontcare = { NULL, NULL };
		S11.I = new double[getNumberOfFrequencies(task)];
		S11.Q = new double[getNumberOfFrequencies(task)];
		S21.I = new double[getNumberOfFrequencies(task)];
		S21.Q = new double[getNumberOfFrequencies(task)];
		code = measure2PortCalibrated(task, S11, S21, dontcare, dontcare);
		logCodeAndQuitIfError(code);
		printf("S11.I\tS11.Q\t\tS21.I\tS21.Q\t\tS11_phase\n");
		for (unsigned int i = 0; i < 5; ++i)
			printf("%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\n", S11.I[i], S11.Q[i], S21.I[i], S21.Q[i], atan2(S11.Q[i], S11.I[i]));



		printf("Re-measuring calibrated data\n");
		code = measure2PortCalibrated(task, S11, S21, dontcare, dontcare);
		logCodeAndQuitIfError(code);
		printf("S11.I\tS11.Q\t\tS21.I\tS21.Q\t\tS11_phase\n");
		for (unsigned int i = 0; i < 5; ++i)
			printf("%.2f\t%.2f\t\t%.2f\t%.2f\t%.2f\n", S11.I[i], S11.Q[i], S21.I[i], S21.Q[i], atan2(S11.Q[i], S11.I[i]));

		printf("\nStopping the task\n");
		code = stop(task);
		logCodeAndQuitIfError(code);


		printf("utilGenerateLinearSweep()\n");
		code = utilGenerateLinearSweep(task, details.minimum_frequency, details.maximum_frequency, 1024);
		logCodeAndQuitIfError(code);


		printf("\nStarting task\n");
		code = start(task);
		logCodeAndQuitIfError(code);
		delete[] S11.I;
		delete[] S11.Q;
		delete[] S21.I;
		delete[] S21.Q;
	}
	{

		ComplexData t1r1 = {
			new double[getNumberOfFrequencies(task)],
			new double[getNumberOfFrequencies(task)] };
		ComplexData dontcare = { NULL, NULL };
		int n, m;
		m = 0;
		while (m < 50)
		{
			n = getNumberOfFrequencies(task);
			printf("Looping! %i freq points\n", n);
			code = measureUncalibrated(task, t1r1, dontcare, dontcare, dontcare, dontcare);
			logCodeAndQuitIfError(code);
			m++;
		}

		delete [] t1r1.I;
		delete [] t1r1.Q;
	}


	printf("\nStopping the task\n");
	code = stop(task);
	logCodeAndQuitIfError(code);

	printf("Deleting task\n");

	deleteTask(task);

	printf("Goodbye\n");
	#ifdef WIN32
		// no "pause" on linux
		system("pause");
	#endif


	return 0;
}

