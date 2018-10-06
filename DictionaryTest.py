from random import randint

from HashTable_Open import HashTable

#This test script will test the dictionary functionality of testee against tester, and will throw an error if it testee disagrees with tester
#Testee and Tester must be some function which returns a new instance of their respective dictionaries
def Tester():
	return {}
def Testee():
	return HashTable()

INTLIMIT = 2^16

def Test(n):
	#n defines the number of tests to occur, and each successive test becomes larger
	k = randint(0, INTLIMIT)
	for i in range(0, n):
		#test if setting values causes problems
		tester = Tester()
		testee = Testee()
		for j in range(0, 2*i):
			if randint(0, n//4) > 0:
				k = randint(0, INTLIMIT)
			v = randint(0, INTLIMIT)
			tester[k] = v
			testee[k] = v

		#test if accurate length
		if len(tester) != len(testee):
			print("tester:", repr(tester))
			print("testee:", repr(testee))
			print("Incorrect length", len(testee), "was supposed to be", len(tester))
			raise

		#test if all key value pairs are present
		for k in tester:
			if k not in testee:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print(testee.data)
				print("Key not found", k)
				raise
			if testee[k] != tester[k]:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Key had incorrect value", k, testee[k], "was supposed to be", tester[k])
				raise

		#test if deletion causes problems for any of the previous tests
		tester = Tester()
		testee = Testee()
		for j in range(0, randint(8, 2*n)):
			k = randint(0, i)
			if k in tester:
				del tester[k]
				del testee[k]
			else:
				v = randint(0, INTLIMIT)
				tester[k] = v
				testee[k] = v
		if len(tester) != len(testee):
			print("tester:", repr(tester))
			print("testee:", repr(testee))
			print("Incorrect length", len(testee), "was supposed to be", len(tester))
			raise
		for k in tester:
			if k not in testee:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Key not found", k)
				raise
			if testee[k] != tester[k]:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Key had incorrect value", k, testee[k],"was supposed to be", tester[k])
				raise

		#test if insertion/deletion generated an erroneous key
		for k in range(0, i):
			if k in testee and not k in tester:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Extra key found", k, testee[k])
				raise

		#test if iteration is accurate
		for k, v in testee.items():
			if k not in tester:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Extra key returned in iteration", k, v)
				raise
			if v != tester[k]:
				print("tester:", repr(tester))
				print("testee:", repr(testee))
				print("Iteration returned incorrect value", k, testee[k],"was supposed to be", tester[k])
				raise

		#test equality
		if testee != tester:
			print("tester:", repr(tester))
			print("testee:", repr(testee))
			print("Equality was incorrect, were equal")
			raise

		#test inequality
		k = randint(0, i)
		if k in tester:
			r = randint(0,1)
			if r == 0:
				del tester[k]
			elif r == 1:
				del testee[k]
			elif r == 2:
				tester[k] = tester[k] + 1
			else:
				testee[k] = testee[k] + 1
		else:
			v = randint(0, INTLIMIT)
			if randint(0,1) == 0:
				tester[k] = v
			else:
				testee[k] = v
		if testee == tester:
			print("tester:", repr(tester))
			print("testee:", repr(testee))
			print("Equality was incorrect, were unequal; change at", k)
			raise

	print("Test completed, no errors occurred; Testee agrees with Tester")


Test(512)
