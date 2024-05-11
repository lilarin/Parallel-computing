import socket
import os
from concurrent.futures import ThreadPoolExecutor


def handle_request(client_socket):
    with client_socket:
        request = client_socket.recv(1024).decode('utf-8')
        if not request:
            return

        path = request.split()[1]
        file_path = path[1:]

        if file_path == '':
            filename = 'index.html'
            status = "200 OK"
        elif os.path.exists(file_path) and os.path.isfile(file_path):
            filename = file_path
            status = "200 OK"
        else:
            filename = '404.html'
            status = "404 Not Found"

        with open(filename, 'rb') as file:
            html_content = file.read()

        css_filename = 'css/style.css'
        try:
            with open(css_filename, 'rb') as css_file:
                css_content = css_file.read()
        except FileNotFoundError:
            css_content = b''

        html_response = html_content.replace(
            b'</head>', f'<style>{css_content.decode("utf-8")}</style></head>'.encode("utf-8")
        )

        response_headers = f"HTTP/1.1 {status}\r\nContent-Type: text/html\r\n"
        response_headers += f"Content-Length: {len(html_response)}\r\n"
        response_headers += "\r\n"

        client_socket.sendall(response_headers.encode('utf-8'))
        client_socket.sendall(html_response)


def main():
    host = 'localhost'
    port = 8080

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((host, port))
        server_socket.listen(5)
        print(f"Server listening on {host}:{port}")

        with ThreadPoolExecutor(max_workers=50) as executor:
            while True:
                client_socket, client_address = server_socket.accept()

                executor.submit(handle_request, client_socket)


if __name__ == "__main__":
    main()
