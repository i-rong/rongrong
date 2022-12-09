import json
from http.server import HTTPServer, SimpleHTTPRequestHandler
import webbrowser

ip = "localhost"  # 监听IP，配置项
port = 8800  # 监听端口，配置项
index_url = "http://%s:%d/index.html" % (ip, port)  # 监听主页url，配置项


# 创建http server
class GetHttpServer(SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.0"
    server_version = "PSHS/0.1"
    sys_version = "Python/3.9.x"
    target = "./"  # 监听目录，配置项

    def do_get(self):
        if self.path.find("/json/") > 0:
            print(self.path)
            self.send_response(200)
            self.send_header("Content-type", "json")
            self.end_headers()
            req = {"success": "ok"}
            self.wfile.write(req.encode("utf-8"))
        else:
            SimpleHTTPRequestHandler.do_GET(self)

    def do_post(self):
        if self.path == "/signin":
            print("postmsg recv, path right")
        else:
            print("postmsg recv, path error")
            data = ""
            data = json.loads(data)
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            rspstr = "recv ok, data = "
            rspstr += json.dumps(data, ensure_ascii=False)
            self.wfile.write(rspstr.encode("utf-8"))


def http_server():
    server = HTTPServer((ip, port), GetHttpServer)
    try:
        # 弹出窗口
        webbrowser.open(index_url)
        # 输出信息
        print("服务器监听地址： ", index_url)
        server.serve_forever()
    except KeyboardInterrupt:
        server.socket.close()


# 执行服务器脚本
http_server()

