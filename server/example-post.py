from sys import argv
from requests import post

if len(argv) == 2:
    print(
        post("http://localhost:9880/upload", files={"file_data": open(argv[1], "rb")})
    )
else:
    print("Usage: python3 example-post.py <path_to_mp3_file>")
