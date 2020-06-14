#!/usr/bin/python3


# Copyright (C) 2019  Vidas Simkus (vic.simkus@simkus.com)

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


#
# Tho whom it may concern, I realize that this script is probably not "the python way" of doing things and frankly I don't care.  It gets the job done and that's all that matters.  So keep you criticisms to yourself
# 

PID_FILE="/tmp/bbb_hvac.pid"
LD_LIB_PATH="../HVAC_LIB/bin"

import subprocess
import os.path
import os
import signal
import sys
from time import sleep

_my_file = None
_me = None
_my_dir = None

def get_previous_pid():
	
	if not os.path.exists(PID_FILE):
		return None

	if not os.path.isfile(PID_FILE):
		print(PID_FILE + ": exists but is not a file.  This is weird.")
		return None

	# PID_FILE exists and is a file.

	pid_file = open(PID_FILE,"rt")
	pid = None

	try:
		pid = int(pid_file.read())
	except:
		print("Failed to parse contents of " + PID_FILE + ".  This is weird.")
		return None

	return pid

def kill_previous_process():
	
	pid = get_previous_pid()

	if pid is None:
		# print("Previous process does not exist?")
		return

	i = 0
	while True:
		try:
			os.kill(pid,signal.SIGINT)
		except BaseException as e:
			# If process fails to be killed means process is either not alive or there's a duplicate pid
			break

		# Signal was sent without exception
		sleep(0.5)
		i += 1

		if i > 10:
			raise RuntimeException("Failed to kil previous process.")

	# If we're here that means that we're satisfied with the old process's death

	if os.path.isfile(PID_FILE):
		try:
			os.unlink(PID_FILE)
		except BaseException as e:
			raise RuntimeException("Failed to delete stale pid file " + PID_FILE + ": " + str(e))

	# Process and pid file dead

	return

def start_process():
	if "LD_LIBRARY_PATH" in os.environ:
		os.environ["LD_LIBRARY_PATH"] = os.environ["LD_LIBRARY_PATH"] + ":" + LD_LIB_PATH;
	else:
		os.environ["LD_LIBRARY_PATH"] = LD_LIB_PATH;

	parms = []

	parms.append(os.path.join(_my_dir,"bin","LOGIC_CORE"))
	parms += sys.argv[1:]

	os.chdir(_my_dir)

	res = subprocess.run(parms)

	if res.returncode != 0:
		# Only command line parameter errors return with a code
		return False
	else:
		return True


def start_server():

	pid = get_previous_pid()

	if pid is not None:
		print("Found previous process: " + str(pid))
		kill_previous_process()

	if not start_process():
		return

	sleep(1)
	pid = get_previous_pid()

	if pid is not None:
		print ("Server started on the first try.")
		return True

	print ("Server failed to start on first try.")

	if start_process():
		print("Server started on second try.")
		return True

	print ("Server failed to start.")

	return False


def main():

	global _my_file
	global _my_dir
	global _me

	_my_file = os.path.abspath(__file__)
	_my_dir,_me = os.path.split(_my_file)

	me = _me.upper()
	mode = None

	if "STOP" in me:
		print("Stopping server")
		kill_previous_process()
		return

	start_server()
	return

	raise RuntimeException("Unexpected code path")

if __name__ == "__main__":
	main()
