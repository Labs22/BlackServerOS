#!/usr/bin/python

import sys
import httplib2
from threading import Thread
from Queue import Queue
import time

num_worker_threads = 8
q = Queue()
url = ""
	
def Scan():	

	
	# Start threads
	for _ in range(num_worker_threads):
		t = Thread(target=Work)
		t.daemon = True
		t.start()
	
	while not q.empty():
		time.sleep(4)
		print "Queue left: ", q.qsize()
	#q.join()
	
def Work():	
	
	h = httplib2.Http(disable_ssl_certificate_validation=True)
	h.follow_all_redirects = False
	
	while not q.empty():
		mod = q.get().rstrip()
		# Check for license.txt
		licensurl = url+"/sites/all/modules/{}/LICENSE.txt".format(mod)
		resp, content = h.request(licensurl)
		#print resp["content-location"]
		#print licensurl
		if(resp.status == 200 and resp["content-location"] == licensurl):
			print mod
			q.task_done()
			
		# Check for 403 or other response than redirect or 404 for module folder
		folderurl = url+"/sites/all/modules/{}/".format(mod)
		resp, content = h.request(folderurl)
		if(resp.status == 200 and resp["content-location"] == folderurl):
			print mod
			q.task_done()
	
		
	
	


if __name__ == '__main__':
	if(len(sys.argv) < 2):
		print """
		USAGE: scanner.py <url>
		"""
		exit(0)
		
	# Load files into queue.
	modules = file("drupal-modules.lst")
	for mod in modules:
		q.put(mod)
		
	url = sys.argv[1]
	Scan()
		
	
