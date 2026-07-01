from __future__ import print_function
import subprocess
import sys

def main():
	iterations = 12
	maxworkers = 16
	maxcounter = 10000000
	
	try:
		if (len(sys.argv) == 3):
			maxworkers = int(sys.argv[1])
			maxwcounter = int(sys.argv[2])
		elif (len(sys.argv) == 2):
			maxworkers = int(sys.argv[1])
		elif (len(sys.argv) == 1):
			pass
		else:
			raise
	except:
		print("Usage: tests.py [maxworkers] [maxcounter]")
		return

	test_time(maxcounter = maxcounter, maxworkers = maxworkers, iterations = iterations)

def test_time(maxcounter, maxworkers, iterations):
	print("count: {}".format(maxcounter))
	print("workers\ttime(s)")

	i = 1
	while i <= maxworkers:
		t = run_test_time(maxcounter = maxcounter, workers = i, iterations = iterations)
		print("{}\t{:.2f}".format(i, t))
		i *= 2

def run_test_time(maxcounter, workers, iterations):
	tot = 0
	for i in range(0, iterations):
		out = subprocess.check_output(
			"(time -f %e -- ./task4 -m {} -w {}) 2>&1".format(maxcounter, workers), 
			shell = True)
		tot += float(out)
	return tot / iterations

main()