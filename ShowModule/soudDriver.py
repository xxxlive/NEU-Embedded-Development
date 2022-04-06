import threading
import os

class SoundDriver(threading.Thread):
    def __init__(self, string):
        super().__init__()
        self.string = string

    def run(self):
        os.system('espeak \"'+self.string.replace('+', ' ').replace('\'', '')+'\" -v zh')
