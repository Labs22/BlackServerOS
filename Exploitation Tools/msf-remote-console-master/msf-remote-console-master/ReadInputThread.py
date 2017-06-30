import threading
import time


class ReadInputThread (threading.Thread):

    command = ""
    path = ""
    lock = None
    stop_thread = False

    def __init__(self, path):
        self.lock = threading.Lock()
        self.path = path
        threading.Thread.__init__(self)

    def run(self):
        # Wait for user input
        temp_command = raw_input(self.get_path())

        # If command empty set to None and raise exception in get_command
        if not temp_command:
            self.set_command(None)

        else: # Else save user typed command
            self.set_command(temp_command)

    def get_command(self):
        self.lock.acquire()
        try:
            if self.command == None:
                raise ValueError()

            return self.command
        finally:
            self.lock.release()

    def set_command(self, command):
        self.lock.acquire()
        try:
            self.command = command
        finally:
            self.lock.release()

    def set_path(self, path):
        self.lock.acquire()
        try:
            self.path = path
        finally:
            self.lock.release()


    def get_path(self):
        self.lock.acquire()
        try:
            return self.path
        finally:
            self.lock.release()

