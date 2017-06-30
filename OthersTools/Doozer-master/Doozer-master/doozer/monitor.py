#!/usr/bin/python

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from core.log import ERROR
from threading import Thread
from datetime import date, datetime
from copy import copy
from time import sleep
import core.sethor as sethor
import sys
import os

""" monitor.py  
"""

class Settings:
    """ Monitor settings
    """

    def __init__(self):
        self.LOG_FILE = "./monitor_log"         # where we log to
        self.MONITOR_DIR = sethor.MONITOR_DIR   # folder to watch
        self.MONITOR_CHECK = 10                 # how often to check, in seconds
        self.DOOZER = sethor.DOOZER             # doozer location

        # state variables
        self.MAX_RUN = 1                        # how many jobs can we run in parallel?
        self.DATA_QUEUE = []                    # queue of jobs, not running
        self.RUNNING = 0                        # number of jobs running
        self.RUNNING_QUEUE = []                 # queue of running jobs

    def log(self, string):
        """ write output to a local log file
        """

        with open(self.LOG_FILE, "a+") as f:
            f.write("[%s %s] %s\n" % (date.today().isoformat(),
                                      datetime.now().strftime("%I:%M%p"),
                                      string))


class Process:
    """ Manage an instance of a process
    """

    def __init__(self, path):
        self.running = False
        self.start_time = datetime.now()
        self.end_time = None
        self.path = path

        self.session = path.split("/")[-1]
        self.session_home = "%s/%s" % (sethor.WORKING_DIR, self.session)
        self.hash_file = None

    def run(self):
        """ Spin up the doozer job
        """

        self.running = True
        os.system("python {0} -p {1} -s {2} 2>/dev/null 1>/dev/null"
                                        .format(settings.DOOZER,
                                                self.hash_file,
                                                self.session))
        self.running = False
        self.end_time = datetime.now()

        
class Handler(FileSystemEventHandler):
    """ Class manages events triggered by our watchdog.  We
    currently just watch for created and moved files.
    """

    def on_created(self, event):
        """ File created event
        """

        self.feast(event.src_path)

    def on_moved(self, event):
        """ File moved event
        """

        self.feast(event.dest_path)

    def feast(self, path):
        """ Generate the process and add it to the data queue 
        """

        settings.log("Creating job for file %s" % path)

        process = Process(path)

        # we're going to move the file into our sessions folder and
        # resume execution there
        try:
            os.makedirs(process.session_home)

            # move our processing files into the newely generated
            # session location
            os.system("mv %s %s/" % (path, process.session_home))

            # reset hash file location
            process.hash_file = "%s/%s" % (process.session_home, path.split('/')[-1])

        except Exception, e:
            settings.log("FATAL: %s" % e)
            return

        settings.DATA_QUEUE.append(process)
        


def run():
    """ Run the job scheduler to manage jobs in parallel.  This function
    ensures that we have no more than MAX_RUN jobs running at a time
    to ensure adequate processing power for cracking.

    Jobs are queued up in DATA_QUEUE if we have hit MAX_RUN, and when a
    spot becomes available, a new job is started.  Pretty typical of process
    scheduling, except we let processes run to completion instead of allocating
    quantums.

    Python lists are thread-safe due to Python GIL.
    """

    observer = Observer()
    observer.schedule(Handler(), settings.MONITOR_DIR)
    observer.start()

    try:

        while True:
            sleep(settings.MONITOR_CHECK)
            
            if len(settings.DATA_QUEUE) > 0 or settings.RUNNING > 0:
                settings.log("%d jobs in queue and %d currently running" % 
                                        (len(settings.DATA_QUEUE), settings.RUNNING))
            if len(settings.DATA_QUEUE) > 0 and settings.RUNNING < settings.MAX_RUN:
                
                process = settings.DATA_QUEUE.pop()
                if not process:
                    continue

                settings.log("Starting up process %s" % process.path)
                thread = Thread(target=process.run)
                thread.start()

                settings.RUNNING += 1
                settings.RUNNING_QUEUE.append(process)
                continue

            tmp_q = copy(settings.RUNNING_QUEUE)
            for proc in tmp_q:
                if not proc.running:
                    settings.log("Job %s is done." % proc.path)
                    settings.RUNNING -= 1
                    settings.RUNNING_QUEUE.remove(proc)
                
    except KeyboardInterrupt:
        observer.stop()
    except Exception, e:
        settings.log('Error: %s' % e)
    observer.join()


if __name__ == "__main__":
    if os.fork():
       sys.exit()

    settings = Settings()
    run()
