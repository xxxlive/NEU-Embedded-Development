import threading
import urllib
from http.server import BaseHTTPRequestHandler, HTTPServer


class ShowHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content_type', 'text/plain;charset=utf-8')
        self.end_headers()
        parsed_path = urllib.parse.urlparse(self.path)
        flag = True
        try:
            params = dict([p.split('=') for p in parsed_path[4].split('&')])
            if params.get('content'):
                s = params.get('content')
                s = urllib.parse.unquote(s)
                print(s)
            else:
                flag = False
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
        print('ShowServer On')
        self.httpd.serve_forever()


# class ShowSys:
#     def __init__(self):
#         # 0 for time mode 1 for recipe mode
#         # 自动机设计
#         self.mode = 0
#         self.mode_map = {'time': 0, 'recipe': 1, 'clear': 2}
#         self.update_map = [{'time': 0, 'recipe': 1}, {'clear': 2, 'time'}]
#
#     def show(self, string):
#         print(string)


def main():
    server_address = ('', 9999)
    httpd = HTTPServer(server_address, ShowHandler)
    http_thread = HTTPServerThread(httpd)
    http_thread.start()


if __name__ == '__main__':
    main()
