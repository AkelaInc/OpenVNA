
import logging
logging.basicConfig(level=logging.DEBUG)

import unittest
import test_vnalibrary


def test():
	print("Running DLL API Tests")


	suite = unittest.TestLoader().loadTestsFromModule(test_vnalibrary)
	unittest.TextTestRunner().run(suite)


if __name__ == '__main__':
	test()