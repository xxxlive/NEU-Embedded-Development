from concurrent.futures import ThreadPoolExecutor
from doctest import FAIL_FAST
from functools import partial
from http.server import BaseHTTPRequestHandler, HTTPServer
from mimetypes import init
from pickle import NONE, TRUE
import threading
from time import sleep
# import cv2
# import Jetson.GPIO as GPIO
import urllib
import _thread

import sound


class Sound(threading.Thread):
    def __init__(self):
        self.videoState = False

    def run_video(self, is_request_in):
        self.open = is_request_in


class VideoMonitor(threading.Thread):
    def __init__(self) -> None:
        super(VideoMonitor,self).__init__()
        self.open = False

    # def run(self):
    #     print('Monitor On')
    #     self.monitor()



class HTTPServerThread(threading.Thread):
    def __init__(self, httpd):
        super(HTTPServerThread, self).__init__()
        self.httpd = httpd

    def run(self):
        print('videoServer_on')
        self.httpd.serve_forever()


# class GasMonitor(threading.Thread):
#     def __init__(self) -> None:
#         super(GasMonitor,self).__init__()
#         self.people = False
#         self.al = 1
#         # GPIO.setmode(GPIO.BOARD)
#         # GPIO.setup(40, GPIO.IN)
#         # GPIO.setup(7, GPIO.OUT, initial=False)
#         # GPIO.output(7, GPIO.LOW)

    # def monitor(self):
    #     while TRUE:
    #         print('sfdsaf')
    #         # self.al = GPIO.input(40)
    #         print(self.al)
    #         print(self.people)
    #         sleep(1)
    #         if self.al == 0 and self.people == False:
    #             # GPIO.output(7, GPIO.HIGH)
    #             sleep(2)
    #             # GPIO.output(7, GPIO.LOW)
    #         sleep(1)
    #
    # def run(self):
    #     print('Monitor On')
    #     self.monitor()
    #
    # def set_people(self, is_people):
    #     self.people = is_people


# class HTTPServerThread(threading.Thread):
#     def __init__(self, httpd):
#         super(HTTPServerThread, self).__init__()
#         self.httpd = httpd
#
#     def run(self):
#         print('GASServer_on')
#         self.httpd.serve_forever()


class VideoHandler(BaseHTTPRequestHandler):

    def __init__(self, video_monitor, *args, **kargs):
        self.video_monitor = video_monitor
        super().__init__(*args, **kargs)

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content_type', 'text/plain;charset=utf-8')
        self.end_headers()
        self.wfile.write("video is on\n".encode())
        parsed_path = urllib.parse.urlparse(self.path)
        try:
            params = dict([p.split('=') for p in parsed_path[4].split('&')])
        except:
            return
        if params.get('video') != NONE:
            if params['video'] == 'ON':
                sound.run_video() #此处加入启动程序

# class GasHandler(BaseHTTPRequestHandler):
#
#     def __init__(self, gas_monitor, *args, **kargs):
#         self.gas_monitor = gas_monitor
#         super().__init__(*args, **kargs)
#
#     def do_GET(self):
#         self.send_response(200)
#         self.send_header('Content_type', 'text/plain;charset=utf-8')
#         self.end_headers()
#         self.wfile.write("Hello Cook\n".encode())
#         parsed_path = urllib.parse.urlparse(self.path)
#         try:
#             params = dict([p.split('=') for p in parsed_path[4].split('&')])
#         except:
#             return
#         if params.get('face') != NONE:
#             if params['face'] == 'YES':
#                 self.gas_monitor.set_people(True)
#             else:
#                 self.gas_monitor.set_people(False)
#         print(self.gas_monitor.people)
#         print(self.gas_monitor.al)

class VideoMonitorSys:
    def __init__(self, monitor):
        self.video_monitor = monitor
        server_address = ('', 10001)
        handler = partial(VideoHandler, self.video_monitor)
        self.httpd = HTTPServer(server_address, handler)
        self.http_thread = HTTPServerThread(httpd=self.httpd)

    def run(self):
        self.http_thread.start()
        self.video_monitor.start()


# class GasMonitorSys:
#     def __init__(self, monitor):
#         self.gas_monitor = monitor
#         server_address = ('', 10000)
#         handler = partial(GasHandler, self.gas_monitor)
#         self.httpd = HTTPServer(server_address, handler)
#         self.http_thread = HTTPServerThread(httpd=self.httpd)
#
#     def run(self):
#         self.http_thread.start()
#         self.gas_monitor.start()


def main():
    monitor = VideoMonitor()
    factory = VideoMonitorSys(monitor)
    factory.run()


if __name__ == '__main__':
    main()
