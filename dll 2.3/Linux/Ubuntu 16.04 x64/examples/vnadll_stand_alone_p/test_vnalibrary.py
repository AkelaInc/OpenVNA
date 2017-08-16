

import VNA

import numpy as np
import time
import sys
import unittest

from vna_address import TEST_ADDRESS

def resilient_connect():
	'''
	So there is a race condition here, as apparently
	closing a socket doesn't /immediately/ release it back
	to the host OS.
	As such, if two tests are run in /very/ quick succession,
	the second test will fail to open the socket, because
	the OS thinks it's still busy.
	'''
	for port in range(1025, 5000):
		time.sleep(0.1)
		try:
			vna_instance = VNA.RAW_VNA()
			vna_instance.setIPAddress(TEST_ADDRESS)
			assert vna_instance.getIPAddress() == TEST_ADDRESS
			vna_instance.setIPPort(port)
			assert vna_instance.getIPPort() == port

			vna_instance.setTimeout(150)
			vna_instance.initialize()
			return vna_instance
		except VNA.VNA_Exception_Socket as e:
			if port > 1200:
				raise e
			time.sleep(0.1)

def resilient_highlevel():
	'''
	So there is a race condition here, as apparently
	closing a socket doesn't /immediately/ release it back
	to the host OS.
	As such, if two tests are run in /very/ quick succession,
	the second test will fail to open the socket, because
	the OS thinks it's still busy.
	'''
	for port in range(1025, 5000):
		time.sleep(0.1)
		try:
			vna_instance = VNA.VNA(TEST_ADDRESS, port)
			assert vna_instance.getIPAddress() == TEST_ADDRESS
			assert vna_instance.getIPPort() == port

			vna_instance.setTimeout(150)
			return vna_instance
		except VNA.VNA_Exception_Socket as e:
			if port > 1200:
				raise e
			time.sleep(0.1)

class TestVnaNoHardwarePresent(unittest.TestCase):

	def setUp(self):
		self.vna = VNA.RAW_VNA()

	def test_version_string(self):
		tmp = VNA.versionString()
		self.assertTrue(tmp)


	def test_ip_address(self):
		tmp = self.vna.getIPAddress()
		self.assertEqual(tmp, None)
		self.vna.setIPAddress("192.168.123.123")
		tmp = self.vna.getIPAddress()
		self.assertEqual(tmp, "192.168.123.123")

	def test_ip_port(self):
		tmp = self.vna.getIPPort()
		self.assertEqual(tmp, 0)
		self.vna.setIPPort(1234)
		tmp = self.vna.getIPPort()
		self.assertEqual(tmp, 1234)

	def test_freq_num(self):
		tmp = self.vna.getNumberOfFrequencies()
		self.assertEqual(tmp, 0)

	def test_get_hardware_details(self):
		tmp = self.vna.getHardwareDetails()

		# These values are 0 when the hardware is still
		# in TASK_UNINITIALIZED
		self.assertEqual(0, tmp['minimum_frequency'])
		self.assertEqual(0, tmp['maximum_frequency'])
		self.assertEqual(0, tmp['maximum_points'])
		self.assertEqual(0, tmp['serial_number'])
		self.assertEqual([0, 0, 0, 0, 0, 0, 0, 0], tmp['band_boundaries'])
		self.assertEqual(0, tmp['number_of_band_boundaries'])


	def test_calibration_number_of_frequencies(self):
		tmp = self.vna.getCalibrationNumberOfFrequencies()
		tmp = self.assertEqual(tmp, 0)


	def test_calibration_frequencies(self):
		tmp = self.vna.getCalibrationFrequencies()
		tmp = self.assertEqual(tmp.shape, (0, ))



	def test_get_frequencies(self):
		tmp = self.vna.getFrequencies()
		tmp = self.assertEqual(tmp.shape, (0, ))



	def test_state(self):
		state = self.vna.getState()
		self.assertEqual(state, VNA.TASK_UNINITIALIZED)

	def test_timeout(self):
		val = self.vna.getTimeout()
		self.assertEqual(val, 1000)
		self.vna.setTimeout(0)
		val = self.vna.getTimeout()
		self.assertEqual(val, 0)
		self.vna.setTimeout(50000000)
		val = self.vna.getTimeout()
		self.assertEqual(val, 50000000)
		self.vna.setTimeout(155)
		val = self.vna.getTimeout()
		self.assertEqual(val, 155)


class TestVnaCommsHardwarePresent(unittest.TestCase):

	def setUp(self):
		self.vna = resilient_connect()

	def test_basic_comms(self):
		'''
		This just validates the basic communication setup is working.
		'''
		pass


class TestVnaHardwarePresent(unittest.TestCase):

	def setUp(self):
		self.vna = resilient_connect()

	def config(self):
		self.vna.setAttenuation(VNA.ATTEN_0)
		self.vna.setHopRate(VNA.HOP_45K)
		self.vna.utilGenerateLinearSweep(400, 1500, 1024)


	def test_hardware_details(self):
		'''
		This just validates the basic communication setup is working.
		'''
		details = self.vna.getHardwareDetails()
		expect = {
						'band_boundaries':  [3000, 1500, 750, 0, 0, 0, 0, 0],
						'maximum_frequency': 6050,
						'maximum_points': 4001,
						'serial_number': 401,
						'minimum_frequency': 375,
						'number_of_band_boundaries': 3
				}
		self.assertEqual(details, expect, "This may fail depending on hardware configuration")

	def test_get_number_of_frequencies(self):
		self.config()
		val = self.vna.getNumberOfFrequencies()
		self.assertEqual(val, 1024)

	def test_utilGenerateLinearSweep(self):
		self.config()

		self.vna.utilGenerateLinearSweep(400, 1500, 1024)
		val = self.vna.getNumberOfFrequencies()
		self.assertEqual(val, 1024)

		self.vna.utilGenerateLinearSweep(400, 1500, 3)
		val = self.vna.getNumberOfFrequencies()
		self.assertEqual(val, 3)

		freqs = self.vna.getFrequencies()
		expect = [  400.00010515,   949.99971334,  1499.99909569]

		# We have to use isclose() because the constants
		# in this file aren't /quite/ the /EXACT/ value due
		# to issues with printing precision.
		self.assertTrue(all(np.isclose(expect, freqs)))

		self.vna.utilGenerateLinearSweep(400, 1500, 8)
		val = self.vna.getNumberOfFrequencies()
		self.assertEqual(val, 8)


		freqs = self.vna.getFrequencies()
		expect = [
					400.00010515,   557.14175338,   714.28340161,   871.42527569,
					1028.56692392,  1185.70857216,  1342.85022039,  1499.99186862
				]

		# We have to use isclose() because the constants
		# in this file aren't /quite/ the /EXACT/ value due
		# to issues with printing precision.
		self.assertTrue(all(np.isclose(expect, freqs)))

	def test_hop_rate(self):
		self.assertEqual(self.vna.getHopRate(), VNA.HOP_UNDEFINED)
		self.vna.setHopRate(VNA.HOP_15K)
		self.assertEqual(self.vna.getHopRate(), VNA.HOP_15K)


	def test_attenuation(self):
		self.vna.setAttenuation(VNA.ATTEN_0)
		self.assertEqual(self.vna.getAttenuation(), VNA.ATTEN_0)
		self.vna.setAttenuation(VNA.ATTEN_6)
		self.assertEqual(self.vna.getAttenuation(), VNA.ATTEN_6)

	def test_state(self):
		state = self.vna.getState()
		self.assertEqual(state, VNA.TASK_STOPPED)

	def test_start_stop(self):
		self.config()

		state = self.vna.getState()
		self.assertEqual(state, VNA.TASK_STOPPED)

		self.vna.start()

		state = self.vna.getState()
		self.assertEqual(state, VNA.TASK_STARTED)

		self.vna.stop()

		state = self.vna.getState()
		self.assertEqual(state, VNA.TASK_STOPPED)


	def test_ping_unit(self):
		self.vna.utilPingUnit()

	def test_set_frequencies(self):

		freqs = [500, 501, 600, 601, 700, 701, 5001, 5002]
		ret = self.vna.setFrequencies(freqs)

		actual = self.vna.getFrequencies()

		# Use isclose() to allow the slight deviation as a result of snapping the
		# desired frequencies to the hardware precision limits.
		self.assertTrue(all(np.isclose(freqs, actual)))


	def test_get_num_frequencies(self):

		freqs = [500, 501, 600, 601, 700, 701, 5001, 5002]
		ret = self.vna.setFrequencies(freqs)

		self.assertEqual(self.vna.getNumberOfFrequencies(), len(freqs))



	def test_unset_frequencies(self):
		unset_freqs = self.vna.getFrequencies()

		# The equality operator on empty numpy arrays results in
		# a empty boolean numpy array. As such, use all()
		self.assertTrue(all(unset_freqs == np.empty([0])))

	def test_fix_sweep_limits(self):
		sweeps = [
			(500,  600,   100),
			(500,  5000,  100),
			(500,  5000, 1000),
		]

		for start, stop, pts in sweeps:
			rstart, rstop = self.vna.utilFixLinearSweepLimits(start, stop, pts)

			self.assertTrue(abs(start - rstart) < 0.25, "delta too large: {} - {} : {}".format(start, rstart, abs(start - rstart)))
			self.assertTrue(abs(stop - rstop)   < 0.25, "delta too large: {} - {} : {}".format(stop,  rstop,  abs(stop  - rstop)))

		with self.assertRaises(VNA.VNA_Exception_Too_Many_Points):
			self.vna.utilFixLinearSweepLimits(1000, 1000, 50000)
		with self.assertRaises(VNA.VNA_Exception_Freq_Out_Of_Bounds):
			self.vna.utilFixLinearSweepLimits(1000, 10000, 500)

	def test_nearest_freq(self):
		freqs = [500, 501, 600, 601, 700, 701, 5001, 5002]
		for freq in freqs:
			res = self.vna.utilNearestLegalFreq(freq)

			# The hardware has quite fine resolution, we should be within 10 KHz in all
			# operations.
			self.assertTrue(abs(freq - res) < 0.01)

		# Test the out-of-bounds behaviour
		with self.assertRaises(VNA.VNA_Exception_Freq_Out_Of_Bounds):
			self.vna.utilNearestLegalFreq(1)
		# Test the out-of-bounds behaviour
		with self.assertRaises(VNA.VNA_Exception_Freq_Out_Of_Bounds):
			self.vna.utilNearestLegalFreq(-1)
		with self.assertRaises(VNA.VNA_Exception_Freq_Out_Of_Bounds):
			self.vna.utilNearestLegalFreq(9000)





class TestVnaAcquisition(unittest.TestCase):

	def setUp(self):
		self.vna = resilient_connect()


		self.vna.setAttenuation(VNA.ATTEN_0)
		self.vna.setHopRate(VNA.HOP_45K)
		self.vna.utilGenerateLinearSweep(400, 1500, 1024)
		self.vna.start()

	def tearDown(self):
		self.vna.stop()

	def test_measure_uncal(self):
		T1R1, T1R2, T2R1, T2R2, Ref = self.vna.measureUncalibrated()
		self.assertTrue(any(T1R1))
		self.assertTrue(any(T1R2))
		self.assertTrue(any(T2R1))
		self.assertTrue(any(T2R2))
		self.assertTrue(any(Ref))

		T1R1, T1R2, T2R1, T2R2, Ref = self.vna.measureUncalibrated()
		self.assertTrue(any(T1R1))
		self.assertTrue(any(T1R2))
		self.assertTrue(any(T2R1))
		self.assertTrue(any(T2R2))
		self.assertTrue(any(Ref))

		T1R1, T1R2, T2R1, T2R2, Ref = self.vna.measureUncalibrated()
		self.assertTrue(any(T1R1))
		self.assertTrue(any(T1R2))
		self.assertTrue(any(T2R1))
		self.assertTrue(any(T2R2))
		self.assertTrue(any(Ref))


	def test_import_cal(self):

		freqs     = [500, 1000, 1500, 2000]
		e00       = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		e11       = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		e10e01    = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		e30       = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		e22       = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		e10e32    = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep33      = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep22      = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep12ep32  = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep03      = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep11      = np.array([1+5j, 2+6j, 3+7j, 4+8j])
		ep23ep01  = np.array([1+5j, 2+6j, 3+7j, 4+8j])

		self.vna.importCalibration(freqs, e00, e11, e10e01, e30, e22, e10e32, ep33, ep22, ep12ep32, ep03, ep11, ep23ep01)


		r_e00, r_e11, r_e10e01, r_e30, r_e22, r_e10e32, r_ep33, r_ep22, r_ep12ep32, r_ep03, r_ep11, r_ep23ep01 = self.vna.exportCalibration()

		self.assertTrue(all(e00      == r_e00))
		self.assertTrue(all(e11      == r_e11))
		self.assertTrue(all(e10e01   == r_e10e01))
		self.assertTrue(all(e30      == r_e30))
		self.assertTrue(all(e22      == r_e22))
		self.assertTrue(all(e10e32   == r_e10e32))
		self.assertTrue(all(ep33     == r_ep33))
		self.assertTrue(all(ep22     == r_ep22))
		self.assertTrue(all(ep12ep32 == r_ep12ep32))
		self.assertTrue(all(ep03     == r_ep03))
		self.assertTrue(all(ep11     == r_ep11))
		self.assertTrue(all(ep23ep01 == r_ep23ep01))

		self.assertEqual(len(freqs), self.vna.getCalibrationNumberOfFrequencies())


	def test_measure_cal(self):
		# use our fake cal for forcing a calibrated measurement
		self.test_import_cal()
		S11, S21, S12, S22 = self.vna.measure2PortCalibrated()
		self.assertTrue(any(S11))
		self.assertTrue(any(S21))
		self.assertTrue(any(S12))
		self.assertTrue(any(S22))


	def test_measure_uncal_a_bunch(self):
		for x in range(20):
			T1R1, T1R2, T2R1, T2R2, Ref = self.vna.measureUncalibrated()
			self.assertTrue(any(T1R1))
			self.assertTrue(any(T1R2))
			self.assertTrue(any(T2R1))
			self.assertTrue(any(T2R2))
			self.assertTrue(any(Ref))


	def test_measure_cal_a_bunch(self):
		# use our fake cal for forcing a calibrated measurement
		self.test_import_cal()
		for x in range(20):
			S11, S21, S12, S22 = self.vna.measure2PortCalibrated()
			self.assertTrue(any(S11))
			self.assertTrue(any(S21))
			self.assertTrue(any(S12))
			self.assertTrue(any(S22))

	def test_hop_rates(self):
		# The test harness init puts the task in the TASK_STARTED state.
		# To change the acq params, we need to push it back into the
		# TASK_STOPPED state.
		self.vna.stop()

		# use our fake cal for forcing a calibrated measurement
		self.test_import_cal()

		# Don't try measuring 1024 points at hop rate of 20,
		# because that'll take forever
		hops = [
				(VNA.HOP_45K, 1024),
				(VNA.HOP_30K, 1024),
				(VNA.HOP_15K, 1024),
				(VNA.HOP_7K,   512),
				(VNA.HOP_3K,   512),
				(VNA.HOP_2K,   512),
				(VNA.HOP_1K,   512),
				(VNA.HOP_550,  256),
				(VNA.HOP_312,  256),
				(VNA.HOP_156,   64),
				(VNA.HOP_78,    32),
				(VNA.HOP_39,    16),
				(VNA.HOP_20,     8)
			]

		print("Testing all possible hop-rates.")
		print("This /will/ take a while.")
		for hop_rate, npts in hops:
			self.vna.setHopRate(hop_rate)
			self.vna.utilGenerateLinearSweep(375, 6000, npts)

			self.vna.start()
			for x in range(3):
				S11, S21, S12, S22 = self.vna.measure2PortCalibrated()
				self.assertTrue(any(S11))
				self.assertTrue(any(S21))
				self.assertTrue(any(S12))
				self.assertTrue(any(S22))
				sys.stdout.write(".")
				sys.stdout.flush()
			self.vna.stop()

		# The teardown handler expects the VNA to be in the TASK_STARTED
		# state. If not, it'll error. Therefore, start the vna before we exit,
		# so it tears down cleanly.
		self.vna.start()


class TestVnaHighLevelAcquisition(unittest.TestCase):

	def setUp(self):
		self.vna = resilient_highlevel()

		self.vna.set_config(hoprate=VNA.HOP_45K, attenuation=VNA.ATTEN_0, freq=(400, 1500, 1024))

	def test_aggregate_conf(self):
		hops = [
				VNA.HOP_45K,
				VNA.HOP_30K,
				VNA.HOP_15K,
				VNA.HOP_7K,
				VNA.HOP_3K,
				VNA.HOP_2K,
				VNA.HOP_1K,
				VNA.HOP_550,
				VNA.HOP_312,
				VNA.HOP_156,
				VNA.HOP_78,
				VNA.HOP_39,
				VNA.HOP_20,
			]
		attens = [
				VNA.ATTEN_0,
				VNA.ATTEN_1,
				VNA.ATTEN_2,
				VNA.ATTEN_3,
				VNA.ATTEN_4,
				VNA.ATTEN_5,
				VNA.ATTEN_6,
				VNA.ATTEN_7,
				VNA.ATTEN_8,
				VNA.ATTEN_9,
				VNA.ATTEN_10,
				VNA.ATTEN_11,
				VNA.ATTEN_12,
				VNA.ATTEN_13,
				VNA.ATTEN_14,
				VNA.ATTEN_15,
				VNA.ATTEN_16,
				VNA.ATTEN_17,
				VNA.ATTEN_18,
				VNA.ATTEN_19,
				VNA.ATTEN_20,
				VNA.ATTEN_21,
				VNA.ATTEN_22,
				VNA.ATTEN_23,
				VNA.ATTEN_24,
				VNA.ATTEN_25,
				VNA.ATTEN_26,
				VNA.ATTEN_27,
				VNA.ATTEN_28,
				VNA.ATTEN_29,
				VNA.ATTEN_30,
				VNA.ATTEN_31,
		]

		for hop in hops:
			for atten in attens:
				self.vna.set_config(hoprate=hop, attenuation=atten, freq=(400, 1500, 1024))
				self.assertEqual(hop, self.vna.getHopRate())
				self.assertEqual(atten, self.vna.getAttenuation())

	def test_nt_measure_uncal(self):
		self.vna.start()

		# The higher-level measure_uncal returns a named tuple
		res = self.vna.measure_uncal()
		self.assertTrue(any(res.T1R1))
		self.assertTrue(any(res.T1R2))
		self.assertTrue(any(res.T2R1))
		self.assertTrue(any(res.T2R2))
		self.assertTrue(any(res.Ref))
		self.vna.stop()

	def test_nt_measure_cal_nocal(self):

		# Clear the cal so we should throw
		self.vna.clearCalibration()

		self.vna.start()
		with self.assertRaises(VNA.VNA_Exception_Bad_Cal):
			self.vna.measure_cal()

		self.vna.stop()

	def test_do_cal(self):
		# Clear the cal
		self.vna.clearCalibration()
		self.assertFalse(self.vna.isCalibrationComplete())

		self.vna.start()

		cal_steps = [
			VNA.STEP_P1_OPEN,
			VNA.STEP_P1_SHORT,
			VNA.STEP_P1_LOAD,
			VNA.STEP_P2_OPEN,
			VNA.STEP_P2_SHORT,
			VNA.STEP_P2_LOAD,
			VNA.STEP_THRU,
		]

		# Do a crappy (but mathematically valid) calibration
		for cal_step in cal_steps:
			self.vna.measureCalibrationStep(cal_step)

		self.assertTrue(self.vna.isCalibrationComplete())

		for x in range(20):
			# measure_cal returns a named tuple.
			res = self.vna.measure_cal()
			self.assertTrue(any(res.S11))
			self.assertTrue(any(res.S21))
			self.assertTrue(any(res.S12))
			self.assertTrue(any(res.S22))

		self.vna.stop()


