import os
import threading
import time
import urllib.parse
from functools import partial
from http.server import BaseHTTPRequestHandler, HTTPServer
from MenuLoader import MenuLoader
import requests


class HTTPServerThread(threading.Thread):
    def __init__(self, httpd):
        super(HTTPServerThread, self).__init__()
        self.httpd = httpd

    def run(self):
        print('MenuServerOn')
        self.httpd.serve_forever()


class RecipeDisplay:
    def __init__(self, content):
        self.cursor = -1
        self.content = content
        self.max_len = len(content)

    def switch_right(self):
        if self.cursor != self.max_len - 1:
            self.cursor += 1
        if len(self.content) != 0:
            self.send_message(self.content[self.cursor])

    def switch_left(self):
        if self.cursor != 0:
            self.cursor -= 1
        if len(self.content) != 0:
            self.send_message(self.content[self.cursor])

    def send_message(self, stream):
        url = "http://127.0.0.1:9999/"
        params = {"recipe": stream}
        try:
            requests.get(url=url, params=params)
        except:
            return

    def shut_down(self):
        if len(self.content) == 0:
            return
        self.clear_up()
        url = "http://127.0.0.1:9999/"
        params = {"clear": '菜谱结束'}
        try:
            requests.get(url=url, params=params)
        except:
            return

    def clear_up(self):
        self.content = []
        self.cursor = 0
        self.max_len = 0


class MenuSys:
    def __init__(self):
        self.menu_loader = MenuLoader()
        self.menu_display = RecipeDisplay([])
        server_address = ('', 9997)
        handler = partial(MenuHandler, self)
        self.httpd = HTTPServer(server_address, handler)
        self.http_thread = HTTPServerThread(httpd=self.httpd)
        self.time_stamp = {'switch_left': time.time(),
                           'switch_right': time.time(),
                           'shut_down': time.time()}

    def search_recipe(self, goal):
        content = self.menu_loader.search_recipe(goal)
        if content is not None:
            self.menu_display = RecipeDisplay(content)
            self.menu_display.switch_right()
            return True
        else:
            self.warning_message('Recipe not found')
            return False

    def warning_message(self, content):
        url = "http://127.0.0.1:9999/"
        params = {"clear": content.encode('UTF-8')}
        try:
            requests.get(url=url, params=params)
        except:
            return

    def check_time(self, fun):
        fun_time = self.time_stamp.get(fun)
        cur_time = time.time()
        if cur_time - fun_time <= 4:
            return False
        else:
            return True

    def switch_right(self):
        if self.check_time('switch_right'):
            self.menu_display.switch_right()
            self.time_stamp.update({'switch_right': time.time()})
            self.time_stamp.update({'switch_left': time.time()})

    def switch_left(self):
        if self.check_time('switch_left'):
            self.menu_display.switch_left()
            self.time_stamp.update({'switch_right': time.time()})
            self.time_stamp.update({'switch_left': time.time()})

    def shut_down(self):
        if self.check_time('shut_down'):
            self.menu_display.shut_down()
            self.time_stamp.update({'shut_down': time.time()})

    def run(self):
        self.http_thread.start()


class MenuHandler(BaseHTTPRequestHandler):
    def __init__(self, menu_sys, *args, **kargs):
        self.menu_sys = menu_sys
        self.fun_map = {'switch_left': self.menu_sys.switch_left,
                        'switch_right': self.menu_sys.switch_right,
                        'shut_down': self.menu_sys.shut_down,
                        'search': self.menu_sys.search_recipe}
        super().__init__(*args, **kargs)

    def do_fun(self, fun):
        c_fun = self.fun_map.get(fun)
        if c_fun is None:
            return False
        c_fun()
        return True

    def handle_params(self, params):
        if params.get('fun'):
            return self.do_fun(params.get('fun'))
        elif params.get('search'):
            name = urllib.parse.unquote(params.get('search'))
            return self.menu_sys.search_recipe(name)
        else:
            return False

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content_type', 'text/plain;charset=utf-8')
        self.end_headers()
        parsed_path = urllib.parse.urlparse(self.path)
        flag = True
        try:
            params = dict([p.split('=') for p in parsed_path[4].split('&')])
            self.handle_params(params)
        except:
            return
        if flag:
            self.wfile.write("Hello Cook\n".encode())
        else:
            self.wfile.write("Error Happen\n".encode())


def main():
    sys = MenuSys()
    sys.run()


if __name__ == '__main__':
    main()
