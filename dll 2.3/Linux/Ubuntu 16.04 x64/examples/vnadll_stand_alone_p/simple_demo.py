
import logging
logging.basicConfig(level=logging.DEBUG)

import VNA
import numpy as np

from vna_address import TEST_ADDRESS

def demo():

	vna = VNA.VNA(TEST_ADDRESS, 1026)
	vna.setTimeout(150)
	vna.set_config(hoprate=VNA.HOP_45K, attenuation=VNA.ATTEN_0, freq=(400, 1500, 1024))
	vna.start()

	# Uncalibrated measurement
	unc = vna.measure_uncal()
	print("T1R1: ", unc.T1R1)
	print("T1R2: ", unc.T1R2)
	print("T2R1: ", unc.T2R1)
	print("T2R2: ", unc.T2R2)
	print("Ref: ", unc.Ref)

	if vna.hasFactoryCalibration() and not vna.isCalibrationComplete():
		vna.importFactoryCalibration()

	if vna.isCalibrationComplete():
		cal = vna.measure_cal()
		print("S11: ", cal.S11)
		print("S21: ", cal.S21)
		print("S12: ", cal.S12)
		print("S22: ", cal.S22)

	vna.stop()


if __name__ == '__main__':
	demo()