import threading
import time
import urllib
from functools import partial
from http.server import BaseHTTPRequestHandler, HTTPServer
from time import sleep

from ShowModule.showDriver import ShowDriver


class ShowHandler(BaseHTTPRequestHandler):

    def __init__(self, show_sys, *args, **kargs):
        self.show_sys = show_sys
        super().__init__(*args, **kargs)

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content_type', 'text/plain;charset=utf-8')
        self.end_headers()
        parsed_path = urllib.parse.urlparse(self.path)
        flag = True
        try:
            params = dict([p.split('=') for p in parsed_path[4].split('&')])
            if len(params) != 1:
                flag = False
            else:
                action = ''
                mes = ''
                for item in params.items():
                    action = item[0]
                    mes = item[1]
                mes = urllib.parse.unquote(mes)
                self.show_sys.deal_message(action, mes)
        except:
            return
        if flag:
            self.wfile.write("Hello Cook\n".encode())
        else:
            self.wfile.write("Error Happen\n".encode())


class HTTPServerThread(threading.Thread):
    def __init__(self, httpd):
        super(HTTPServerThread, self).__init__()
        self.httpd = httpd

    def run(self):
        self.httpd.serve_forever()


class Timer(threading.Thread):

    def __init__(self, show_sys):
        self.show_sys = show_sys
        super(Timer, self).__init__()

    def send_clock(self):
        while True:
            time_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
            self.show_sys.deal_message('time', time_str)
            sleep(1)

    def run(self):
        self.send_clock()


class ShowSys:
    def __init__(self):
        # 0 for time mode 1 for recipe mode
        # 自动机设计
        self.mode = 0
        self.mode2index = {'time_s': 0, 'recipe_s': 1, 'clear_s': 2}
        self.index2mode = {0: 'time_s', 1: 'recipe_s', 2: 'clear_s'}
        self.update_map = [{'recipe': 'recipe_s'}, {'clear': 'clear_s'}, {'recipe': 'recipe_s', 'time': 'time_s'}]
        self.string_map = {}
        for item in self.mode2index:
            self.string_map.update({item: ''})
        self.show_driver = ShowDriver()
        self.timer = Timer(self)
        self.handler = partial(ShowHandler, self)
        server_address = ('', 9999)
        self.httpd = HTTPServer(server_address, self.handler)
        self.httpd_thread = HTTPServerThread(self.httpd)

    def run(self):
        self.httpd_thread.start()
        self.timer.start()

    def show(self):
        cur_state = self.index2mode.get(self.mode)
        show_string = self.string_map.get(cur_state)
        self.show_driver.show(show_string)

    def deal_message(self, me_type, string):
        self.update_string_map(me_type, string)
        self.change_mode(me_type)
        self.show()

    def action_to_situation(self, action):
        return action + '_s'

    def update_string_map(self, me_type, string):
        self.string_map.update({self.action_to_situation(me_type): string})

    def change_mode(self, action):
        future_state = self.update_map[self.mode].get(action)
        if future_state:
            self.mode = self.mode2index.get(future_state)


def main():
    sys = ShowSys()
    sys.run()


if __name__ == '__main__':
    main()